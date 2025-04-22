package com.ut.{{__TOP_MODULE_NAME__}}


import com.xspcomm._

class UT_{{__TOP_MODULE_NAME__}}(args: Array[String]) extends JavaUT_{{__TOP_MODULE_NAME__}} with BaseDUTTrait {

    val vec = new StringScalaVector()
    for (i <- args.indices) {
      vec.add(args(i))
    }
    var dut = new DutUnifiedBase(vec)
    var xclock = new XClock(this.dut.getPxcStep(), this.dut.getPSelf())
    val internalSignals: scala.collection.mutable.Map[String, XData] = scala.collection.mutable.Map()
    val internalSignalsList: scala.collection.mutable.Map[String, Array[XData]] = scala.collection.mutable.Map()
    var xport = new XPort()
    var xcfg = this.newXCFG()

    def newXCFG(): XSignalCFG = {
        var ret: XSignalCFG = null
        try {
            JavaUT_{{__TOP_MODULE_NAME__}}.loadFileInJar("/{{__TOP_MODULE_NAME__}}_offset.yaml", (p) => {
                ret = new XSignalCFG(p, dut.GetXSignalCFGBasePtr())
            })
        } catch {
            case e: Exception => {
                if(dut.GetXSignalCFGBasePtr() == 0){
                    println("Error: " + e.getMessage)
                    return null
                }
                ret = new XSignalCFG("/{{__TOP_MODULE_NAME__}}_offset.yaml", dut.GetXSignalCFGBasePtr())
            }
        }
        ret
    }

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
    def VPIInternalSignalList(prefix: String="", deep: Int=99): Array[String] = {
        var ret: Array[String] = Array.empty[String]
        this.dut.VPIInternalSignalList(prefix, deep).forEach { i =>
            if (i != null) {
                ret :+= i
            }
        }
        ret
    }
    def GetInternalSignal(name: String, index: Int = -1, use_vpi: Boolean = false): XData = {
        if (this.internalSignals.contains(name)) {
            return this.internalSignals(name)
        }
        var signal: XData = null
        if(!use_vpi){
            val xname = "CFG:" + name
            if (dut.GetXSignalCFGBasePtr() == 0) {
                return signal
            }
            if (index >= 0) {
                signal = this.xcfg.NewXData(name, index, xname)
            }else{
                signal = this.xcfg.NewXData(name, xname)
            }
            if (signal != null) {
                this.internalSignals += (name -> signal)
            }
            return signal
        }
        signal = XData.FromVPI(this.dut.GetVPIHandleObj(name),
                                   this.dut.GetVPIFuncPtr("vpi_get"),
                                   this.dut.GetVPIFuncPtr("vpi_get_value"),
                                   this.dut.GetVPIFuncPtr("vpi_put_value"), "VPI:" + name)
        if (signal != null) {
            this.internalSignals += (name -> signal)
        }
        signal
    }
    def GetInternalSignal(name: String, is_array:Boolean): Array[XData] = {
        if (this.internalSignalsList.contains(name)) {
            return this.internalSignalsList(name)
        }
        var result = Array.empty[XData]
        if (this.dut.GetXSignalCFGBasePtr() == 0) {
            return result
        }
        if(!is_array){
            return result
        }
        val xname = "CFG:" + name
        val signalList = this.xcfg.NewXDataArray(name, xname)
        if (signalList != null) {
            signalList.forEach { i =>
                if (i != null) {
                    result :+= i
                }
            }
        }
        result
    }
    def GetInternalSignalList(prefix:String="", deep:Int=99, use_vpi:Boolean=false): Array[String] = {
        var ret: Array[String] = Array.empty[String]
        if (this.dut.GetXSignalCFGBasePtr() != 0 && !use_vpi) {
            this.xcfg.GetSignalNames(prefix).forEach { i =>
                if (i != null) {
                    if (prefix == "" || i.startsWith(prefix)) {
                        ret :+= i
                    }
                }
            }
            return ret
        }
        this.VPIInternalSignalList(prefix, deep)
    }
    /*************************************************/
    /*               End of User APIs                */
    /*************************************************/
}
