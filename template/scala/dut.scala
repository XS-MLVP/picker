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
    val internalSignalsList: scala.collection.mutable.Map[String, XArray] = scala.collection.mutable.Map()
    {% if __HAS_AGGREGATE_VIEWS__ %}
    var vpiBackend: XVPI = null
    {% endif %}
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
    def ResumeWaveformDump(): Boolean = {
        this.dut.ResumeWaveformDump()
    }
    def PauseWaveformDump(): Boolean = {
        this.dut.PauseWaveformDump()
    }
    def WaveformPaused(): Int = {
        this.dut.WaveformPaused()
    }
    def SetWaveform(wave_name: String) = {
        this.dut.SetWaveform(wave_name)
    }
    def GetWaveFormat(): String = {
        com.ut.{{__TOP_MODULE_NAME__}}.DutUnifiedBase.GetWaveFormat()
    }
    def FlushWaveform() = {
        this.dut.FlushWaveform()
    }
    def SetCoverage(coverage_name: String) = {
        this.dut.SetCoverage(coverage_name)
    }
    def ResetCoverage() = {
        this.dut.ResetCoverage()
    }
    def GetCovMetrics(): Int = {
        com.ut.{{__TOP_MODULE_NAME__}}.DutUnifiedBase.GetCovMetrics()
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
        {% if __HAS_AGGREGATE_VIEWS__ %}
        if (this.vpiBackend != null) {
            this.vpiBackend.Close()
            this.vpiBackend.delete()
            this.vpiBackend = null
        }
        {% endif %}
        this.dut.Finish()
    }
    def InitClock(clock_name: String) = {
        this.xclock.Add(this.xport.Get(clock_name))
    }
    def RefreshComb() = {
        this.xclock.RefreshComb()
    }
    def AtClone(): Unit = {
        this.dut.atClone()
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
    {% if __HAS_AGGREGATE_VIEWS__ %}
    def GetInternalSignalView(name: String): XView = {
        if (this.vpiBackend == null) {
            this.vpiBackend = new XVPI()
            this.vpiBackend.Configure(
                this.dut.GetVPIFuncPtr("vpi_get"),
                this.dut.GetVPIFuncPtr("vpi_get_value"),
                this.dut.GetVPIFuncPtr("vpi_put_value"),
                this.dut.GetVPIFuncPtr("vpi_handle_by_name"),
                this.dut.GetVPIFuncPtr("vpi_handle_by_index"),
                this.dut.GetVPIFuncPtr("vpi_handle"),
                this.dut.GetVPIFuncPtr("vpi_iterate"),
                this.dut.GetVPIFuncPtr("vpi_scan"),
                this.dut.GetVPIFuncPtr("vpi_get_str"),
                this.dut.GetVPIFuncPtr("vpi_release_handle"),
                this.dut.GetVPIScope(), {{__AGGREGATE_LAYOUTS_JSON__}})
        }
        this.vpiBackend.Get(name)
    }
    {% endif %}
    def GetInternalSignal(name: String, is_array:Boolean): XArray = {
        if (this.internalSignalsList.contains(name)) {
            return this.internalSignalsList(name)
        }
        if (this.dut.GetXSignalCFGBasePtr() == 0) {
            return null
        }
        if(!is_array){
            return null
        }
        val result = this.xcfg.NewXArray(name)
        this.internalSignalsList(name) = result
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
