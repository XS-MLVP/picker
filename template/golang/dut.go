package UT_{{__TOP_MODULE_NAME__}}


import (
    "xspcomm"
)

type UT_{{__TOP_MODULE_NAME__}} struct {
    Dut DutUnifiedBase
    Xclock xspcomm.XClock
    Xport xspcomm.XPort
    // Pins
{{__XDATA_DECL__}}
    // SubPorts
{{__XPORT_CASCADED_DEC__}}
}


func NewUT_{{__TOP_MODULE_NAME__}}(a ...interface{}) *UT_{{__TOP_MODULE_NAME__}}{
    self := &UT_{{__TOP_MODULE_NAME__}}{}
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
    self.Xclock = xspcomm.NewXClock(func(dump bool) int {
        self.Dut.SimStep(dump)
        return 0
    })
    self.Xport = xspcomm.NewXPort()
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

func (self *UT_{{__TOP_MODULE_NAME__}}) SetWaveform(filename string){
    self.Dut.SetWaveform(filename)
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
