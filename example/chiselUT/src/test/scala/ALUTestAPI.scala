package examples


import chisel3.stage.ChiselGeneratorAnnotation
import circt.stage._
import com.xspcomm._;


class ALUTestAPI(dut: BaseDUTTrait) {
    dut.InitClock("clock")
    def getDut(): BaseDUTTrait = {
        dut
    }
    def listPins(): StringVector = {
        dut.GetXPort().GetKeys()
    }
    def reset(): Unit = {
        dut("reset") := 1
        dut.Step(1)
        dut("reset") := 0
        dut.Step(1)
    }
    def comOp(a: Int, b: Int, o: Int, c: Int): Int = {
        dut("io_operandA") := a
        dut("io_operandB") := b
        dut("io_op") := o
        dut("io_start") := 1
        var count = 0
        while(!dut("io_done").B() && count < c) {
            dut.Step(1)
            count += 1
            dut("io_start") := 0
        }
        dut("io_result").AsInt32()
    }
    def add(a: Int, b: Int): Int = {
        comOp(a, b, 0, 3)
    }
    def sub(a: Int, b: Int): Int = {
        comOp(a, b, 1, 3)
    }
    def mul(a: Int, b: Int): Int = {
        comOp(a, b, 2, 100)
    }
    def div(a: Int, b: Int): Int = {
        comOp(a, b, 3, 100)
    }
    def finish(): Unit = {
        dut.Finish()
    }
}


object ALUUtils {

  var dut: ALUTestAPI = null

  def issueVerilog(workDir: String, dutClassName: String): Unit = {
    (new ChiselStage).execute(Array("--target-dir", workDir, "--target", "verilog"),
          Seq(
              ChiselGeneratorAnnotation(() => new MultiCycleALU(32)
          )
    ))
  }

  def getDUT(): ALUTestAPI = {
    if(dut == null) {
        dut = new ALUTestAPI(chiselUT.generateDUT[MultiCycleALU](issueVerilog, pickerExArgs="-w out/alu_wave.fst"))
    }
    dut
  }

}