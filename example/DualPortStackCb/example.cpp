#include "UT_dual_port_stack.hpp"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <functional>
#include <stdexcept>


class StackModel {
public:
    void commitPush(int data) {
        stack.push_back(data);
        std::cout << "push " << data << std::endl;
    }

    void commitPop(int dutData) {
        std::cout << "Pop " << dutData << std::endl;
        if (stack.empty()) {
            throw std::runtime_error("Stack is empty");
        }
        int modelData = stack.back();
        stack.pop_back();
        if (modelData != dutData) {
            throw std::runtime_error("The model data " + std::to_string(modelData) + " is not equal to the dut data " + std::to_string(dutData));
        }
        std::cout << "Pass: " << modelData << " == " << dutData << std::endl;
    }

private:
    std::vector<int> stack;
};

class SinglePortDriver {
public:
    enum class Status {
        IDLE, WAIT_REQ_READY, WAIT_RESP_VALID
    };

    enum class BusCMD {
        PUSH, POP, PUSH_OKAY, POP_OKAY
    };

    SinglePortDriver(UTdual_port_stack* dut, StackModel* model, std::unordered_map<std::string, XData*> pinMap)
        : dut(dut), model(model), status(Status::IDLE), operationNum(0), remainingDelay(0), random(std::random_device{}()), pinMap(pinMap) {}

    void push() {
        pinMap["in_valid"]->Set(1);
        pinMap["in_cmd"]->Set(static_cast<int>(BusCMD::PUSH));
        pinMap["in_data"]->Set(random() & 0xff);
    }

    void pop() {
        pinMap["in_valid"]->Set(1);
        pinMap["in_cmd"]->Set(static_cast<int>(BusCMD::POP));
    }

    void stepCallback(long cycle) {
        switch (status) {
            case Status::WAIT_REQ_READY:
                if (pinMap["in_ready"]->U() == 1) {
                    pinMap["in_valid"]->Set(0);
                    pinMap["out_ready"]->Set(1);
                    status = Status::WAIT_RESP_VALID;
                    if (pinMap["in_cmd"]->U() == static_cast<int>(BusCMD::PUSH)) {
                        model->commitPush(pinMap["in_data"]->AsInt32());
                    }
                }
                break;

            case Status::WAIT_RESP_VALID:
                if (pinMap["out_valid"]->U() == 1) {
                    pinMap["out_ready"]->Set(0);
                    status = Status::IDLE;
                    remainingDelay = random() % 6;
                    if (pinMap["out_cmd"]->U() == static_cast<int>(BusCMD::POP_OKAY)) {
                        model->commitPop(pinMap["out_data"]->AsInt32());
                    }
                }
                break;

            case Status::IDLE:
                if (remainingDelay == 0) {
                    if (operationNum < 10) {
                        push();
                    } else if (operationNum < 20) {
                        pop();
                    } else {
                        return;
                    }
                    operationNum++;
                    status = Status::WAIT_REQ_READY;
                } else {
                    remainingDelay--;
                }
                break;
        }
    }

private:
    UTdual_port_stack* dut;
    StackModel* model;
    Status status;
    int operationNum;
    int remainingDelay;
    std::mt19937 random;
    std::unordered_map<std::string, XData*> pinMap;
};

void testStack(UTdual_port_stack* stack) {
    StackModel model;
    std::unordered_map<std::string, XData*> pinMap0 = {
        {"in_valid", &stack->in0_valid}, {"in_ready", &stack->in0_ready}, {"in_data", &stack->in0_data}, {"in_cmd", &stack->in0_cmd},
        {"out_valid", &stack->out0_valid}, {"out_ready", &stack->out0_ready}, {"out_data", &stack->out0_data}, {"out_cmd", &stack->out0_cmd}
    };

    std::unordered_map<std::string, XData*> pinMap1 = {
        {"in_valid", &stack->in1_valid}, {"in_ready", &stack->in1_ready}, {"in_data", &stack->in1_data}, {"in_cmd", &stack->in1_cmd},
        {"out_valid", &stack->out1_valid}, {"out_ready", &stack->out1_ready}, {"out_data", &stack->out1_data}, {"out_cmd", &stack->out1_cmd}
    };

    SinglePortDriver driver0(stack, &model, pinMap0);
    SinglePortDriver driver1(stack, &model, pinMap1);

    stack->StepRis([&driver0](long cycle, void*) { driver0.stepCallback(cycle); });
    stack->StepRis([&driver1](long cycle, void*) { driver1.stepCallback(cycle); });
    stack->Step(200);
    printf("C++ Test Passed\n");
}

int main() {
    UTdual_port_stack dut;
    dut.InitClock("clk");
    testStack(&dut);
    dut.Finish();
    return 0;
}
