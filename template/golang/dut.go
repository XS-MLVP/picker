package UT_{{__TOP_MODULE_NAME__}}


import (
    "xspcomm"
    "container/list"
)

type UT_{{__TOP_MODULE_NAME__}} struct {
    Dut DutUnifiedBase
    Xclock xspcomm.XClock
    Xport xspcomm.XPort
    internalSignals map[string]xspcomm.XData
    internalSignalsList map[string]*list.List
    Xcfg xspcomm.XSignalCFG
    // Pins
{{__XDATA_DECL__}}
    // SubPorts
{{__XPORT_CASCADED_DEC__}}
}


func NewUT_{{__TOP_MODULE_NAME__}}(a ...interface{}) *UT_{{__TOP_MODULE_NAME__}}{
    self := &UT_{{__TOP_MODULE_NAME__}}{}
    self.internalSignals = make(map[string]xspcomm.XData)
	if len(a) == 1 {
        strings, ok := a[0].([]string)
        if ok {
            args := NewStringVector()
            for _, v := range strings {
                args.Add(v)
            }
            self.Dut = NewDutUnifiedBase(args)
        }else{
            self.Dut = NewDutUnifiedBase(a ...)
        }
    }else{
        self.Dut = NewDutUnifiedBase(a ...)
    }
    self.Xclock = xspcomm.NewXClock(self.Dut.GetPxcStep(), self.Dut.GetPSelf())
    self.Xport = xspcomm.NewXPort()
    self.Xcfg = xspcomm.NewXSignalCFG(self.Dut.GetXSignalCFGPath(), self.Dut.GetXSignalCFGBasePtr())
    // Create
{{__XDATA_INIT__}}
    // Bind
{{__XDATA_BIND__}}
    // Add to port
{{__XPORT_ADD__}}
    self.Xclock.Add(self.Xport)
    // New SubPorts
{{__XPORT_CASCADED_SGN__}}
    return self
}

/*******************************/
/**      User APIs             */
/*******************************/
func (self *UT_{{__TOP_MODULE_NAME__}}) Step(a ...interface{}) {
    argc := len(a)
	if argc == 1 {
        self.Xclock.Step(a[0].(int))
    } else {
        self.Xclock.Step(1)
    }
}

func (self *UT_{{__TOP_MODULE_NAME__}}) Finish() {
    self.Dut.Finish()
}

func (self *UT_{{__TOP_MODULE_NAME__}}) InitClock(name string){
    self.Xclock.Add(self.Xport.Get(name))
}

func (self *UT_{{__TOP_MODULE_NAME__}}) StepRis(fc func(uint64)){
    self.Xclock.StepRis(fc)
}

func (self *UT_{{__TOP_MODULE_NAME__}}) StepFal(fc func(uint64)){
    self.Xclock.StepFal(fc)
}

func (self *UT_{{__TOP_MODULE_NAME__}}) OpenWaveform() bool {
    return self.Dut.OpenWaveform()
}

func (self *UT_{{__TOP_MODULE_NAME__}}) CloseWaveform() bool {
    return self.Dut.CloseWaveform()
}

func (self *UT_{{__TOP_MODULE_NAME__}}) GetXClock() xspcomm.XClock {
    return self.Xclock
}

func (self *UT_{{__TOP_MODULE_NAME__}}) GetXPort() xspcomm.XPort {
    return self.Xport
}

func (self *UT_{{__TOP_MODULE_NAME__}}) SetWaveform(filename string){
    self.Dut.SetWaveform(filename)
}

func (self *UT_{{__TOP_MODULE_NAME__}}) FlushWaveform(){
    self.Dut.FlushWaveform()
}

func (self *UT_{{__TOP_MODULE_NAME__}}) CheckPoint(name string) int {
    return self.Dut.CheckPoint(name)
}

func (self *UT_{{__TOP_MODULE_NAME__}}) Restore(name string) int {
    return self.Dut.Restore(name)
}

func (self *UT_{{__TOP_MODULE_NAME__}}) GetInternalSignal(name string, exargs ...interface{}) xspcomm.XData {
    // args: string name, int index = -1, bool use_vpi = false
    if v, ok := self.internalSignals[name]; ok {
        return v
    }
    var index int = -1
    var use_vpi bool = false
    var signal xspcomm.XData = nil
    var xname string = "CFG:" + name
    if len(exargs) > 0 {
        if len(exargs) == 1 {
            index = exargs[0].(int)
        } else if len(exargs) == 2 {
            index = exargs[0].(int)
            use_vpi = exargs[1].(bool)
        }
    }
    if !use_vpi {
        var signalGo xspcomm.XDataGo
        if index >= 0 {
            signalGo = self.Xcfg.NewXData(name, index, xname);
        } else {
            signalGo = self.Xcfg.NewXData(name, xname);
        }
        if signalGo == nil {
            return nil
        }
        signal = xspcomm.NewXDataFrom(signalGo)
    }else{
        signal = xspcomm.XDataFromVPI(self.Dut.GetVPIHandleObj(name),
                                      self.Dut.GetVPIFuncPtr("vpi_get"),
                                      self.Dut.GetVPIFuncPtr("vpi_get_value"),
                                      self.Dut.GetVPIFuncPtr("vpi_put_value"), "VPI:" + name)
    }
    if signal == nil {
        return nil
    }
    self.internalSignals[name] = signal
    return self.internalSignals[name]
}

func (self *UT_{{__TOP_MODULE_NAME__}}) GetInternalSignalArray(name string) *list.List {
    if v, ok := self.internalSignalsList[name]; ok {
        return v
    }
    return_list := list.New()
    signal_array := self.Xcfg.NewXDataArray(name, "CFG:" + name)
    if signal_array.Size() == 0 {
        return return_list
    }
    for i := 0; i < int(signal_array.Size()); i++ {
        return_list.PushBack(signal_array.Get(i))
    }
    self.internalSignalsList[name] = return_list
    return return_list
}

func (self *UT_{{__TOP_MODULE_NAME__}}) GetInternalSignalList(args ...interface{}) *list.List {
    var prefix string = ""
    var deep int = 99
    var use_vpi bool = false
    if len(args) > 0 {
        prefix = args[0].(string)
        if len(args) > 1 {
            deep = args[1].(int)
        }
        if len(args) > 2 {
            use_vpi = args[2].(bool)
        }
    }
    rlist := list.New()
    if !use_vpi {
        slist := self.Xcfg.GetSignalNames(prefix)
        if slist.Size() == 0 {
            return rlist
        }
        for i := 0; i < int(slist.Size()); i++ {
            signal := slist.Get(i)
            rlist.PushBack(signal)
        }
        return rlist
    }
    return self.VPIInternalSignalList(prefix, deep)
}

func (self *UT_{{__TOP_MODULE_NAME__}}) VPIInternalSignalList(args ...interface{}) *list.List {
    var prefix string = ""
    var deep int = 99
    if len(args) > 0 {
        prefix = args[0].(string)
        if len(args) > 1 {
            deep = args[1].(int)
        }
    }
    rlist := list.New()
    slist := self.Dut.VPIInternalSignalList(prefix, deep)
    if slist.Size() == 0 {
        return rlist
    }
    for i := 0; i < int(slist.Size()); i++ {
        rlist.PushBack(slist.Get(i))
    }
    return rlist
}

func (self *UT_{{__TOP_MODULE_NAME__}}) SetCoverage(filename string){
    self.Dut.SetCoverage(filename)
}

func (self *UT_{{__TOP_MODULE_NAME__}}) RefreshComb(){
    self.Dut.RefreshComb()
}

/*******************************/
/**    End of User APIs        */
/*******************************/
