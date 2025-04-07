package com.ut


import com.xspcomm._

class UT_{{__TOP_MODULE_NAME__}}(args: Array[String]) extends JavaUT_{{__TOP_MODULE_NAME__}} with BaseDUTTrait {

    val vec = new StringScalaVector()
    for (i <- args.indices) {
      vec.add(args(i))
    }
    var dut = new DutUnifiedBase(vec)
    var xclock = new XClock(this.dut.getPxcStep(), this.dut.getPSelf())
    val internalSignals: scala.collection.mutable.Map[String, XData] = scala.collection.mutable.Map()
    var xport = new XPort()

    def this() = {
        this(Array.empty[String])
    }

    def this(arg: String) = {
        this(Array(arg))
    }

    // Create Pins
{{__XDATA_INIT__}}

    // Bind DPIs
{{__XDATA_BIND__}}

    // Add to port
{{__XPORT_ADD__}}

    // New Subports
{{__XPORT_CASCADED_SGN__}}

    this.xclock.Add(this.xport)
    
    /*************************************************/
    /*                  User APIs                    */
    /*************************************************/
    def GetXClock(): XClock = {
        this.xclock
    }
    def GetXPort(): XPort = {
        this.xport
    }
    def OpenWaveform(): Boolean = {
        this.dut.OpenWaveform()
    }
    def CloseWaveform(): Boolean = {
        this.dut.CloseWaveform()
    }
    def SetWaveform(wave_name: String) = {
        this.dut.SetWaveform(wave_name)
    }
    def FlushWaveform() = {
        this.dut.FlushWaveform()
    }
    def SetCoverage(coverage_name: String) = {
        this.dut.SetCoverage(coverage_name)
    }
    def Step(i: Int = 1): Unit = {
        this.xclock.Step(i)
    }
    def StepRis(callback: (Long) => Unit) = {
        this.xclock.StepRis(callback)
    }
    def StepFal(callback: (Long) => Unit) = {
        this.xclock.StepFal(callback)
    }
    def Finish(): Unit  = {
        this.dut.Finish()
    }
    def InitClock(clock_name: String) = {
        this.xclock.Add(this.xport.Get(clock_name))
    }
    def RefreshComb() = {
        this.dut.RefreshComb()
    }
    def CheckPoint(check_point: String) = {
        this.dut.CheckPoint(check_point)
    }
    def Restore(check_point: String) = {
        this.dut.Restore(check_point)
    }
    def VPIInternalSignalList(prefix: String, deep: Int): StringVector = {
        val ret: StringVector = new StringVector()
        this.dut.VPIInternalSignalList(prefix, deep).forEach { i =>
            if (i != null) {
                ret.add(i)
            }
        }
        ret
    }
    def GetInternalSignal(name: String): XData = {
        if (this.internalSignals.contains(name)) {
            return this.internalSignals(name)
        }
        val signal = XData.FromVPI(this.dut.GetVPIHandleObj(name),
                                   this.dut.GetVPIFuncPtr("vpi_get"),
                                   this.dut.GetVPIFuncPtr("vpi_get_value"),
                                   this.dut.GetVPIFuncPtr("vpi_put_value"), name)
        if (signal != null) {
            this.internalSignals += (name -> signal)
        }
        signal
    }
    /*************************************************/
    /*               End of User APIs                */
    /*************************************************/
}
