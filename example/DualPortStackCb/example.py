try:
    from DUTdual_port_stack import *
except:
    try:
        from UT_dual_port_stack import *
    except:
        from __init__ import *


import random
from enum import Enum

class StackModel:
    def __init__(self):
        self.stack = []

    def commit_push(self, data):
        self.stack.append(data)
        print("push", data)

    def commit_pop(self, dut_data):
        print("Pop", dut_data)
        model_data = self.stack.pop()
        assert model_data == dut_data, f"The model data {model_data} is not equal to the dut data {dut_data}"
        print(f"Pass: {model_data} == {dut_data}")

class SinglePortDriver:
    class Status(Enum):
        IDLE = 0
        WAIT_REQ_READY = 1
        WAIT_RESP_VALID = 2
    class BusCMD(Enum):
        PUSH = 0
        POP = 1
        PUSH_OKAY = 2
        POP_OKAY = 3

    def __init__(self, dut, model: StackModel, port_dict):
        self.dut = dut
        self.model = model
        self.port_dict = port_dict

        self.status = self.Status.IDLE
        self.operation_num = 0
        self.remaining_delay = 0

    def push(self):
        self.port_dict["in_valid"].value = 1
        self.port_dict["in_cmd"].value = self.BusCMD.PUSH.value
        self.port_dict["in_data"].value = random.randint(0, 2**32-1)

    def pop(self):
        self.port_dict["in_valid"].value = 1
        self.port_dict["in_cmd"].value = self.BusCMD.POP.value

    def step_callback(self, cycle):
        if self.status == self.Status.WAIT_REQ_READY:
            if self.port_dict["in_ready"].value == 1:
                self.port_dict["in_valid"].value = 0
                self.port_dict["out_ready"].value = 1
                self.status = self.Status.WAIT_RESP_VALID

                if self.port_dict["in_cmd"].value == self.BusCMD.PUSH.value:
                    self.model.commit_push(self.port_dict["in_data"].value)

        elif self.status == self.Status.WAIT_RESP_VALID:
            if self.port_dict["out_valid"].value == 1:
                self.port_dict["out_ready"].value = 0
                self.status = self.Status.IDLE
                self.remaining_delay = random.randint(0, 5)

                if self.port_dict["out_cmd"].value == self.BusCMD.POP_OKAY.value:
                    self.model.commit_pop(self.port_dict["out_data"].value)

        if self.status == self.Status.IDLE:
            if self.remaining_delay == 0:
                if self.operation_num < 10:
                    self.push()
                elif self.operation_num < 20:
                    self.pop()
                else:
                    return

                self.operation_num += 1
                self.status = self.Status.WAIT_REQ_READY
            else:
                self.remaining_delay -= 1

def test_stack(stack):
    model = StackModel()

    port0 = SinglePortDriver(stack, model, {
        "in_valid": stack.in0_valid,
        "in_ready": stack.in0_ready,
        "in_data": stack.in0_data,
        "in_cmd": stack.in0_cmd,
        "out_valid": stack.out0_valid,
        "out_ready": stack.out0_ready,
        "out_data": stack.out0_data,
        "out_cmd": stack.out0_cmd,
    })

    port1 = SinglePortDriver(stack, model, {
        "in_valid": stack.in1_valid,
        "in_ready": stack.in1_ready,
        "in_data": stack.in1_data,
        "in_cmd": stack.in1_cmd,
        "out_valid": stack.out1_valid,
        "out_ready": stack.out1_ready,
        "out_data": stack.out1_data,
        "out_cmd": stack.out1_cmd,
    })

    dut.StepRis(port0.step_callback)
    dut.StepRis(port1.step_callback)

    dut.Step(200)


if __name__ == "__main__":
    dut = DUTdual_port_stack()
    dut.InitClock("clk")
    test_stack(dut)
    dut.Finish()
