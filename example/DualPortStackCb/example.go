package main

import (
    "fmt"
    "math/rand"
	"xspcomm"
    "time"
	ut "UT_dual_port_stack"
)

type StackModel struct {
    stack []int
}

func NewStackModel() *StackModel {
    return &StackModel{stack: []int{}}
}

func (sm *StackModel) CommitPush(data int) {
    sm.stack = append(sm.stack, data)
    fmt.Println("push", data)
}

func (sm *StackModel) CommitPop(dutData int) {
    fmt.Println("Pop", dutData)
    if len(sm.stack) == 0 {
        panic("stack is empty")
    }
    modelData := sm.stack[len(sm.stack)-1]
    sm.stack = sm.stack[:len(sm.stack)-1]
    if modelData != dutData {
        panic(fmt.Sprintf("The model data %d is not equal to the dut data %d", modelData, dutData))
    }
    fmt.Printf("Pass: %d == %d\n", modelData, dutData)
}

type Status int

const (
    IDLE Status = iota
    WAIT_REQ_READY
    WAIT_RESP_VALID
)

type BusCMD int

const (
    PUSH BusCMD = iota
    POP
    PUSH_OKAY
    POP_OKAY
)

type SinglePortDriver struct {
    dut           *ut.UT_dual_port_stack
    model         *StackModel
    status        Status
    operationNum  int
    remainingDelay int
    random        *rand.Rand
    pinMap        map[string] xspcomm.XData
}

func NewSinglePortDriver(dut *ut.UT_dual_port_stack, model *StackModel, pinMap map[string] xspcomm.XData) *SinglePortDriver {
    return &SinglePortDriver{
        dut:           dut,
        model:         model,
        status:        IDLE,
        operationNum:  0,
        remainingDelay: 0,
        random:        rand.New(rand.NewSource(time.Now().UnixNano())),
        pinMap:        pinMap,
    }
}

func (spd *SinglePortDriver) Push() {
    spd.pinMap["in_valid"].Set(1)
    spd.pinMap["in_cmd"].Set(int(PUSH))
    spd.pinMap["in_data"].Set(spd.random.Intn(256))
}

func (spd *SinglePortDriver) Pop() {
    spd.pinMap["in_valid"].Set(1)
    spd.pinMap["in_cmd"].Set(int(POP))
}

func (spd *SinglePortDriver) StepCallback(cycle uint64) {
    switch spd.status {
    case WAIT_REQ_READY:
        if int(spd.pinMap["in_ready"].Get().Int64()) == 1 {
            spd.pinMap["in_valid"].Set(0)
            spd.pinMap["out_ready"].Set(1)
            spd.status = WAIT_RESP_VALID
            if int(spd.pinMap["in_cmd"].Get().Int64()) == int(PUSH) {
                spd.model.CommitPush(int(spd.pinMap["in_data"].Get().Int64()))
            }
        }
    case WAIT_RESP_VALID:
        if spd.pinMap["out_valid"].Get().Int64() == 1 {
            spd.pinMap["out_ready"].Set(0)
            spd.status = IDLE
            spd.remainingDelay = spd.random.Intn(6)
            if int(spd.pinMap["out_cmd"].Get().Int64()) == int(POP_OKAY) {
                spd.model.CommitPop(int(spd.pinMap["out_data"].Get().Int64()))
            }
        }
    case IDLE:
        if spd.remainingDelay == 0 {
            if spd.operationNum < 10 {
                spd.Push()
            } else if spd.operationNum < 20 {
                spd.Pop()
            } else {
                return
            }
            spd.operationNum++
            spd.status = WAIT_REQ_READY
        } else {
            spd.remainingDelay--
        }
    }
}


func testStack(stack *ut.UT_dual_port_stack) {
    model := NewStackModel()

    pinMap0 := map[string] xspcomm.XData{
        "in_valid": stack.In0_valid, "in_ready": stack.In0_ready, "in_data": stack.In0_data, "in_cmd": stack.In0_cmd,
        "out_valid": stack.Out0_valid, "out_ready": stack.Out0_ready, "out_data": stack.Out0_data, "out_cmd": stack.Out0_cmd,
    }

    pinMap1 := map[string] xspcomm.XData{
        "in_valid": stack.In1_valid, "in_ready": stack.In1_ready, "in_data": stack.In1_data, "in_cmd": stack.In1_cmd,
        "out_valid": stack.Out1_valid, "out_ready": stack.Out1_ready, "out_data": stack.Out1_data, "out_cmd": stack.Out1_cmd,
    }

    driver0 := NewSinglePortDriver(stack, model, pinMap0)
    driver1 := NewSinglePortDriver(stack, model, pinMap1)

    stack.StepRis(func(cycle uint64) { driver0.StepCallback(cycle) })
    stack.StepRis(func(cycle uint64) { driver1.StepCallback(cycle) })
    stack.Step(200)
	fmt.Println("Golang tests passed")
}

func main() {
    dut := ut.NewUT_dual_port_stack()
    dut.InitClock("clk")
    testStack(dut)
    dut.Finish()
}
