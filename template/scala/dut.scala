package com.ut;


import com.xspcomm._;

class UT_{{__TOP_MODULE_NAME__}}(args: Array[String]) extends JavaUT_{{__TOP_MODULE_NAME__}} {

    val vec = new StringVector()
    for (i <- args.indices) {
      vec.add(args(i))
    }
    var dut = new DutUnifiedBase(vec)
    var xclock = new XClock((dump: Boolean) => {
        this.dut.simStep(dump)
        ()
    })
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
    def SetWaveform(wave_name: String) = {
        this.dut.SetWaveform(wave_name)
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
    /*************************************************/
    /*               End of User APIs                */
    /*************************************************/
}
