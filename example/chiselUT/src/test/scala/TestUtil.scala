package examples

import com.xspcomm.{chiselUT, BaseDUTTrait};
import chisel3._
import chisel3.stage.ChiselGeneratorAnnotation
import circt.stage._
import scala.reflect.runtime.universe._


object TestUtil {

  def CreateDUT[A: TypeTag](dutInstanceCreater:() => RawModule): BaseDUTTrait = {
    // define how to create the DUT verilog
    def issueVerilog(workDir: String, dutClassName: String): Unit = {
        (new ChiselStage).execute(Array("--target-dir", workDir, "--target", "verilog"),
          Seq(
              ChiselGeneratorAnnotation(dutInstanceCreater)
            )
        )
    }
    // picker args explain:
    //  --rw mem_direct       : export internal signals with mem_direct read/write
    //  --w out/alu_wave.fst  : write waveform to out/alu_wave.fst
    val waveFile = typeOf[A].typeSymbol.fullName.replace(".", "_")
    chiselUT.generateDUT[A](issueVerilog, pickerExArgs=s"-w out/${waveFile}.fst --rw mem_direct")
  }

}
