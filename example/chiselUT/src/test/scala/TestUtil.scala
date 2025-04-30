package examples

import com.xspcomm.{chiselUT, BaseDUTTrait}
import chisel3._
import chisel3.stage.ChiselGeneratorAnnotation
import circt.stage._
import scala.reflect.runtime.universe._
import chisel3.util._
import chisel3.experimental.prefix
import chisel3.internal._


object TestUtil {

  def CreateDUT[A: TypeTag](dutInstanceCreater: () => RawModule): com.xspcomm.BaseDUTTrait = {
    // define how to create the DUT verilog
    def issueVerilog(workDir: String, dutClassName: String): Unit = {
      (new ChiselStage).execute(
        Array("--target-dir", workDir, "--target", "verilog"),
        Seq(
          ChiselGeneratorAnnotation(dutInstanceCreater),
        )
      )
    }
    // picker args explain:
    //  --rw mem_direct       : export internal signals with mem_direct read/write
    //  --w out/alu_wave.fst  : write waveform to out/alu_wave.fst
    val waveFile = typeOf[A].typeSymbol.fullName.replace(".", "_")
    chiselUT.generateDUT[A](issueVerilog, pickerExArgs = s"-w out/${waveFile}.fst --rw mem_direct")
  }
}

object IsDebug{
  def apply(): Boolean ={
    // Check environment XDebug = 1 or not
    if(sys.env.get("XDebug").contains("1")) {
      true
    } else {
      false
    }
  }
}

object MarkAsDebug {
  def apply[T <: Data](signals: Map[String,T]): Unit = {
    // Check environment XDebug = 1 or not
    if(IsDebug()) {
      println("Debugging enabled, generating debug signals")
    } else {
      println("Debugging disabled, skipping debug signal generation")
      return
    }
    // Create a local dontTouch register to hold the debug signals
    signals.foreach { case (name, signal) =>
      prefix(s"${name}") {
        val debug = RegInit(0.U(signal.getWidth.W))
        debug := signal
        dontTouch(debug)
        dontTouch(signal)
      }
    }
  }
}
