package com.ut;

import com.ut.UT_dual_port_stack;
import com.xspcomm.XData;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Random;
import java.util.Map;


class StackModel {
    private List<Integer> stack;

    public StackModel() {
        stack = new ArrayList<>();
    }

    public void commitPush(int data) {
        stack.add(data);
        System.out.println("push " + data);
    }

    public void commitPop(int dutData) {
        System.out.println("Pop " + dutData);
        int modelData = stack.remove(stack.size() - 1);
        if (modelData != dutData) {
            throw new AssertionError("The model data " + modelData + " is not equal to the dut data " + dutData);
        }
        System.out.println("Pass: " + modelData + " == " + dutData);
    }
}

class SinglePortDriver {
    enum Status {
        IDLE, WAIT_REQ_READY, WAIT_RESP_VALID
    }

    enum BusCMD {
        PUSH, POP, PUSH_OKAY, POP_OKAY
    }

    private UT_dual_port_stack dut;
    private StackModel model;
    private Status status;
    private int operationNum;
    private int remainingDelay;
    private Random random;
    Map<String, XData> pinMap;

    public SinglePortDriver(UT_dual_port_stack dut, StackModel model, Map<String, XData> pinMap) {
        this.dut = dut;
        this.model = model;
        this.status = Status.IDLE;
        this.operationNum = 0;
        this.remainingDelay = 0;
        this.random = new Random();
        this.pinMap = pinMap;
    }

    public void push() {
        this.pinMap.get("in_valid").Set(1);
        this.pinMap.get("in_cmd").Set(BusCMD.PUSH.ordinal());
        this.pinMap.get("in_data").Set(random.nextInt() & 0xff);
    }

    public void pop() {
        this.pinMap.get("in_valid").Set(1);
        this.pinMap.get("in_cmd").Set(BusCMD.POP.ordinal());
    }

    public void stepCallback(long cycle) {
        switch (status) {
            case WAIT_REQ_READY:
                if (this.pinMap.get("in_ready").Get().intValue() == 1) {
                    this.pinMap.get("in_valid").Set(0);
                    this.pinMap.get("out_ready").Set(1);
                    status = Status.WAIT_RESP_VALID;
                    if (this.pinMap.get("in_cmd").Get().intValue() == BusCMD.PUSH.ordinal()) {
                        model.commitPush(this.pinMap.get("in_data").Get().intValue());
                    }
                }
                break;

            case WAIT_RESP_VALID:
                if (this.pinMap.get("out_valid").Get().intValue() == 1) {
                    this.pinMap.get("out_ready").Set(0);
                    status = Status.IDLE;
                    remainingDelay = random.nextInt(6);
                    if (this.pinMap.get("out_cmd").Get().intValue() == BusCMD.POP_OKAY.ordinal()) {
                        model.commitPop(this.pinMap.get("out_data").Get().intValue());
                    }
                }
                break;

            case IDLE:
                if (remainingDelay == 0) {
                    if (operationNum < 10) {
                        push();
                    } else if (operationNum < 20) {
                        pop();
                    } else {
                        return;
                    }

                    operationNum++;
                    status = Status.WAIT_REQ_READY;
                } else {
                    remainingDelay--;
                }
                break;
        }
    }
}


public class example {
    public static void testStack(UT_dual_port_stack stack) {
        StackModel model = new StackModel();
        Map<String, XData> pinMap0 = new HashMap<>();
        Map<String, XData> pinMap1 = new HashMap<>();
        
        pinMap0.put("in_valid", stack.in0_valid);
        pinMap0.put("in_ready", stack.in0_ready);
        pinMap0.put("in_data", stack.in0_data);
        pinMap0.put("in_cmd", stack.in0_cmd);
        pinMap0.put("out_valid", stack.out0_valid);
        pinMap0.put("out_ready", stack.out0_ready);
        pinMap0.put("out_data", stack.out0_data);
        pinMap0.put("out_cmd", stack.out0_cmd);

        pinMap1.put("in_valid", stack.in1_valid);
        pinMap1.put("in_ready", stack.in1_ready);
        pinMap1.put("in_data", stack.in1_data);
        pinMap1.put("in_cmd", stack.in1_cmd);
        pinMap1.put("out_valid", stack.out1_valid);
        pinMap1.put("out_ready", stack.out1_ready);
        pinMap1.put("out_data", stack.out1_data);
        pinMap1.put("out_cmd", stack.out1_cmd);

        SinglePortDriver driver0 = new SinglePortDriver(stack, model, pinMap0);
        SinglePortDriver driver1 = new SinglePortDriver(stack, model, pinMap1);

        stack.StepRis((c)->{driver0.stepCallback(c);});
        stack.StepRis((c)->{driver1.stepCallback(c);});
        stack.Step(200);
    }

    public static void main(String[] args) {
        UT_dual_port_stack dut = new UT_dual_port_stack();
        dut.InitClock("clk");
        testStack(dut);
        dut.Finish();
    }
}
