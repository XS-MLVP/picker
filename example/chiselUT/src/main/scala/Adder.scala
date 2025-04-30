package examples

import chisel3._
import chisel3.util._

class AdderWithControl(width: Int = 32) extends Module {
  val io = IO(new Bundle {
    val a = Input(UInt(width.W))
    val b = Input(UInt(width.W))
    val start = Input(Bool())
    val sum = Output(UInt(width.W))
    val done = Output(Bool())
  })
  val resultReg = RegInit(0.U(width.W))
  val doneReg = RegInit(false.B)
  when (io.start) {
    resultReg := io.a + io.b
    doneReg := true.B
  } .elsewhen (doneReg) {
    doneReg := false.B
  }
  io.sum := resultReg
  io.done := doneReg
}
