
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
	def __init__(self, waveform_filename=None, coverage_filename=None, *a, **kw):
		super().__init__(*a, **kw)
		self.xclock = xsp.XClock(self.step)
		self.port  = xsp.XPort()
		self.xclock.Add(self.port)
		self.event = self.xclock.getEvent()

		# set output files
		if waveform_filename:
			super().set_waveform(waveform_filename)
		if coverage_filename:
			super().set_coverage(coverage_filename)

		# all Pins
{{__XDATA_INIT__}}

		# BindDPI
{{__XDATA_BIND__}}
		# Add2Port
{{__XPORT_ADD__}}

	def __del__(self):
		super().__del__()
		self.finalize()

	def init_clock(self,name:str):
		self.xclock.Add(self.port[name])

	def Step(self,i: int):
		return self.xclock.Step(i)

	def StepRis(self, call_back, args=(), kwargs={}):
		return self.xclock.StepRis(call_back, args, kwargs)

	def StepFal(self, call_back, args=(), kwargs={}):
		return self.xclock.StepFal(call_back, args, kwargs)

	def __getitem__(self, key):
		return xsp.XPin(self.port[key], self.event)

	async def astep(self,i: int):
		return await self.xclock.AStep(i)

	async def acondition(self,fc_cheker):
		return await self.xclock.ACondition(fc_cheker)

	def runstep(self,i: int):
		return self.xclock.RunStep(i)

if __name__=="__main__":
	dut=DUT{{__TOP_MODULE_NAME__}}("libDPI{{__TOP_MODULE_NAME__}}.so")
	dut.Step(1)
