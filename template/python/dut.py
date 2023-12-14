
import xspcomm as xsp
from libUT_{{__TOP_MODULE_NAME__}} import *

class DUT{{__TOP_MODULE_NAME__}}(DutUnifiedBase):

	# 初始化
	def __init__(self, *a, **kw):
		super().__init__(*a, **kw)
		self.xclock = xsp.XClock(self.step)
		self.port  = xsp.XPort()
		# self.event = self.xclock.getEvent()
	
		# all Pins
{{__XDATA_INIT__}}

		# BindDPI
{{__XDATA_BIND__}}
		# Add2Port
{{__XPORT_ADD__}}
	
	def init_clock(self,name:str):
		self.xclock.Add(self.port[name])
	 
	def Step(self,i: int):
		return self.xclock.Step(i)
	
	def __getitem__(self, key):
		return xsp.XPin(self.port[key], self.event)
	
	# dut["clock"].value = 1
	# def __setitem__(self, key, value):
	#    self.port[key].value = value
	
	async def astep(self,i: int):
		return self.xclock.AStep(i)
		
	async def acondition(self,fc_cheker):
		return self.xclock.ACondition(fc_cheker)
	
	async def runstep(self,i: int):
		return self.xclock.RunSetp(i)

if __name__=="__main__":
	dut=DUT{{__TOP_MODULE_NAME__}}()
	dut.init_clock("clock")
	dut.Step(1)