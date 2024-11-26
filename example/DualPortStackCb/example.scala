package com.ut

import com.ut.UT_dual_port_stack
import com.xspcomm.XData

import scala.collection.mutable
import scala.util.Random

class StackModel {
  private val stack = new mutable.Stack[Int]()

  def commitPush(data: Int): Unit = {
    stack.push(data)
    println(s"push $data")
  }

  def commitPop(dutData: Int): Unit = {
    println(s"Pop $dutData")
    val modelData = stack.pop()
    if (modelData != dutData) {
      throw new AssertionError(s"The model data $modelData is not equal to the dut data $dutData")
    }
    println(s"Pass: $modelData == $dutData")
  }
}

class SinglePortDriver(dut: UT_dual_port_stack, model: StackModel, pinMap: Map[String, XData]) {
  object Status extends Enumeration {
    type Status = Value
    val IDLE, WAIT_REQ_READY, WAIT_RESP_VALID = Value
  }

  object BusCMD extends Enumeration {
    type BusCMD = Value
    val PUSH, POP, PUSH_OKAY, POP_OKAY = Value
  }

  import Status._
  import BusCMD._

  private var status: Status = IDLE
  private var operationNum: Int = 0
  private var remainingDelay: Int = 0
  private val random = new Random()

  def push(): Unit = {
    pinMap("in_valid").Set(1)
    pinMap("in_cmd").Set(PUSH.id)
    pinMap("in_data").Set(random.nextInt() & 0xff)
  }

  def pop(): Unit = {
    pinMap("in_valid").Set(1)
    pinMap("in_cmd").Set(POP.id)
  }

  def stepCallback(cycle: Long): Unit = {
    status match {
      case WAIT_REQ_READY =>
        if (pinMap("in_ready").Get().intValue() == 1) {
          pinMap("in_valid").Set(0)
          pinMap("out_ready").Set(1)
          status = WAIT_RESP_VALID
          if (pinMap("in_cmd").Get().intValue() == PUSH.id) {
            model.commitPush(pinMap("in_data").Get().intValue())
          }
        }

      case WAIT_RESP_VALID =>
        if (pinMap("out_valid").Get().intValue() == 1) {
          pinMap("out_ready").Set(0)
          status = IDLE
          remainingDelay = random.nextInt(6)
          if (pinMap("out_cmd").Get().intValue() == POP_OKAY.id) {
            model.commitPop(pinMap("out_data").Get().intValue())
          }
        }

      case IDLE =>
        if (remainingDelay == 0) {
          if (operationNum < 10) {
            push()
          } else if (operationNum < 20) {
            pop()
          } else {
            return
          }
          operationNum += 1
          status = WAIT_REQ_READY
        } else {
          remainingDelay -= 1
        }
    }
  }
}

object example {
  def testStack(stack: UT_dual_port_stack): Unit = {
    val model = new StackModel()

    val pinMap0 = Map(
      "in_valid" -> stack.in0_valid,
      "in_ready" -> stack.in0_ready,
      "in_data" -> stack.in0_data,
      "in_cmd" -> stack.in0_cmd,
      "out_valid" -> stack.out0_valid,
      "out_ready" -> stack.out0_ready,
      "out_data" -> stack.out0_data,
      "out_cmd" -> stack.out0_cmd
    )

    val pinMap1 = Map(
      "in_valid" -> stack.in1_valid,
      "in_ready" -> stack.in1_ready,
      "in_data" -> stack.in1_data,
      "in_cmd" -> stack.in1_cmd,
      "out_valid" -> stack.out1_valid,
      "out_ready" -> stack.out1_ready,
      "out_data" -> stack.out1_data,
      "out_cmd" -> stack.out1_cmd
    )

    val driver0 = new SinglePortDriver(stack, model, pinMap0)
    val driver1 = new SinglePortDriver(stack, model, pinMap1)

    stack.StepRis((c: Long) => driver0.stepCallback(c))
    stack.StepRis((c: Long) => driver1.stepCallback(c))
    stack.Step(200)
    println("Scala tests passed")
  }

  def main(args: Array[String]): Unit = {
    val dut = new UT_dual_port_stack()
    dut.InitClock("clk")
    testStack(dut)
    dut.Finish()
  }
}