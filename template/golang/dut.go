package UT_{{__TOP_MODULE_NAME__}}


import (
    "xspcomm"
)

type UT_{{__TOP_MODULE_NAME__}} struct {
    dut DutUnifiedBase
    Clock xspcomm.XClock
    Port xspcomm.XPort
    // Pins
{{__XDATA_DECL__}}
}


func NewUT_{{__TOP_MODULE_NAME__}}(a ...interface{}) *UT_{{__TOP_MODULE_NAME__}}{
    self := &UT_{{__TOP_MODULE_NAME__}}{}
    self.dut = NewDutUnifiedBase(a ...)
    self.Clock = xspcomm.NewXClock(func(dump bool) int {
        self.dut.SimStep(dump)
        return 0
    })
    self.Port = xspcomm.NewXPort()
    // Create
{{__XDATA_INIT__}}
    // Bind
{{__XDATA_BIND__}}
    // Add to port
{{__XPORT_ADD__}}
    self.Clock.Add(self.Port)
    return self
}

/*******************************/
/**      User APIs             */
/*******************************/
func (self *UT_{{__TOP_MODULE_NAME__}}) Step(a ...interface{}) {
    argc := len(a)
	if argc == 1 {
        self.Clock.Step(a[0].(int))
    } else {
        self.Clock.Step(1)
    }
}

func (self *UT_{{__TOP_MODULE_NAME__}}) Finish() {
    self.dut.Finish()
}

func (self *UT_{{__TOP_MODULE_NAME__}}) InitClock(name string){
    self.Clock.Add(self.Port.Get(name))
}

func (self *UT_{{__TOP_MODULE_NAME__}}) StepRis(fc func(uint64)){
    self.Clock.StepRis(fc)
}

func (self *UT_{{__TOP_MODULE_NAME__}}) StepFal(fc func(uint64)){
    self.Clock.StepFal(fc)
}

func (self *UT_{{__TOP_MODULE_NAME__}}) SetWaveform(filename string){
    self.dut.SetWaveform(filename)
}

func (self *UT_{{__TOP_MODULE_NAME__}}) SetCoverage(filename string){
    self.dut.SetCoverage(filename)
}

func (self *UT_{{__TOP_MODULE_NAME__}}) RefreshComb(){
    self.dut.RefreshComb()
}

/*******************************/
/**    End of User APIs        */
/*******************************/
