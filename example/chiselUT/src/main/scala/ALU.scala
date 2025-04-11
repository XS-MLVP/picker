package examples

import chisel3._
import chisel3.util._
import chisel3.experimental.BundleLiterals._
import com.xspcomm.{nameOf, attrOf}
import javax.xml.crypto.Data


class MultiCycleALU(width: Int) extends Module {
  val io = IO(new Bundle {
    val start = Input(Bool())
    val op = Input(UInt(2.W))
    val operandA = Input(UInt(width.W))
    val operandB = Input(UInt(width.W))
    val result = Output(UInt(width.W))
    val done = Output(Bool())
    val debug_count = Output(UInt(log2Ceil(width).W))
  })

  val idle :: computeAddSub :: computeMul :: computeDiv :: done :: Nil = Enum(5)
  val state = RegInit(idle)
  val operandAReg = RegInit(0.U(width.W))
  val operandBReg = RegInit(0.U(width.W))
  val resultReg   = RegInit(0.U(width.W))
  val count       = RegInit(0.U((log2Ceil(width)+1).W))
  val mulTemp     = RegInit(0.U((2 * width).W))
  val divRemainder= RegInit(0.U(width.W))
  val divQuotient = RegInit(0.U(width.W))
  val unusedReg   = RegInit(0.U(width.W))
  val unusedWire1 = Wire(UInt(width.W))
  var unusedWire2 = unusedReg + unusedWire1

  unusedWire1 := unusedReg

  io.debug_count := count
  io.result := resultReg
  io.done := (state === done)

  when(state === idle) {
      count := 0.U
      when(io.start) {
        operandAReg := io.operandA
        operandBReg := io.operandB
        mulTemp := 0.U
        divRemainder := 0.U
        divQuotient := 0.U
        when(io.op === 0.U || io.op === 1.U) {
          state := computeAddSub
        }.elsewhen(io.op === 2.U) {
          state := computeMul
        }.elsewhen(io.op === 3.U) {
          state := computeDiv
        }.otherwise {
          state := idle
        }
      }
    }.elsewhen(state === computeAddSub) {
      val sum = operandAReg + operandBReg
      val diff = operandAReg - operandBReg
      resultReg := Mux(io.op === 0.U, sum, diff)
      state := done
    }.elsewhen(state === computeMul) {
      when(count < width.U) {
        when(operandBReg(0)) {
          mulTemp := mulTemp + (operandAReg << count)
        }
        count := count + 1.U
        operandBReg := operandBReg >> 1
      }.otherwise {
        resultReg := mulTemp(width - 1, 0)
        state := done
      }
    }.elsewhen(state === computeDiv) {
      when(count === 0.U) {
        resultReg := operandAReg / operandBReg
        state := done
      }.otherwise {
        count := count + 1.U
      }
    }.elsewhen(state === done) {
      when(!io.start) {
        state := idle
      }
    }

  MarkAsDebug(nameOf(unusedReg, unusedWire1, unusedWire2))
  // Or use `attrOf` to preserve all signals, excluding literals and Bundles.
  //MarkAsDebug(attrOf[chisel3.Data](this, filter = (k, v) => !v.isLit && !v.isInstanceOf[chisel3.Bundle]))
}
