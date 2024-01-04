
if __package__ or "." in __name__:
	from . import xspcomm as xsp
else:
	import xspcomm as xsp

if __package__ or "." in __name__:
	from .libUT_{{__TOP_MODULE_NAME__}} import *
else:
	from libUT_{{__TOP_MODULE_NAME__}} import *


class DUT{{__TOP_MODULE_NAME__}}(DutUnifiedBase):

	# 初始化
	def __init__(self, *a, **kw):
		super().__init__(*a, **kw)
		self.xclock = xsp.XClock(self.step)
		self.port  = xsp.XPort()
		self.xclock.Add(self.port)
		self.event = self.xclock.getEvent()

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

	async def astep(self,i: int):
		return self.xclock.AStep(i)

	async def acondition(self,fc_cheker):
		return self.xclock.ACondition(fc_cheker)

	async def runstep(self,i: int):
		return self.xclock.RunSetp(i)

if __name__=="__main__":
	dut=DUT{{__TOP_MODULE_NAME__}}("libDPI{{__TOP_MODULE_NAME__}}.so")
	dut.Step(1)
