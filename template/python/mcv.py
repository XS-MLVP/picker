#coding=utf-8
#python wrapper of mcv


{{python_dut_types}}

class VerilatedContext:
    def __init__(self):
        pass
    def commandArgs(self, cmd: str):
        pass
    def gotFinish(self) -> bool:
        pass


class {{dut_name}}:
    {{python_dut_members}}
    def __init__(self, ctx:VerilatedContext, top:str="top"):
        pass
    {{python_dut_funcs}}


# overwrite the abow class
from _{{dut_name}} import *


def DUT(*args, top:str="top") -> (VerilatedContext, {{dut_name}}):
    ctx = VerilatedContext()
    ctx.commandArgs(" ".join(args))
    dut = {{dut_name}}(ctx, top)
    return ctx, dut


def default_test():
    ctx, dut = DUT()
    step = 0
    while not ctx.gotFinish():
        print("step: ", step, end="\r")
        dut.eval_step()
        step += 1
    print("test complete")


if __name__ == "__main__":
    default_test()
