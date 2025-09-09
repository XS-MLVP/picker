#coding=utf8

try:
    from . import xspcomm as xsp
except Exception as e:
    import xspcomm as xsp

if __package__ or "." in __name__:
    from .libUT_{{__TOP_MODULE_NAME__}} import *
else:
    from libUT_{{__TOP_MODULE_NAME__}} import *


class DUT{{__TOP_MODULE_NAME__}}(object):

    # initialize
    def __init__(self, *args, **kwargs):
        self.dut = DutUnifiedBase(*args)
        self.xclock = xsp.XClock(self.dut.pxcStep, self.dut.pSelf)
        self.xport  = xsp.XPort()
        self.xclock.Add(self.xport)
        self.event = self.xclock.getEvent()
        self.internal_signals = {}
        self.xcfg = xsp.XSignalCFG(self.dut.GetXSignalCFGPath(), self.dut.GetXSignalCFGBasePtr())
        {% if __SIMULATOR__ == "gsim" %}
        # Set fast mode for GSim
        self.xclock.SetFastMode(xsp.FastMode_ONLY_STEP_RIS)
        {% endif %}

        # set output files
        if kwargs.get("waveform_filename"):
            self.dut.SetWaveform(kwargs.get("waveform_filename"))
        if kwargs.get("coverage_filename"):
            self.dut.SetCoverage(kwargs.get("coverage_filename"))

        # All pins
{{__XDATA_INIT__}}

        # BindDPI or Native pin address
{{__XDATA_BIND__}}

        # Add2Port
{{__XPORT_ADD__}}

        # Cascaded ports
{{__XPORT_CASCADED__}}

    def __del__(self):
        self.Finish()

    ################################
    #         User APIs            #
    ################################
    def InitClock(self, name: str):
        """
        Initialize the clock, bind the 'XClock' in the DUT to the corresponding pin

        Args:
            name(str): the name of 'XClock'
        """
        self.xclock.Add(self.xport[name])

    def Step(self, i:int = 1):
        """
        Push the timing circuit by i clock cycles.

        Args:
            i(int): Push by i clock cycles(Default is 1)
        """
        self.xclock.Step(i)

    def StepRis(self, callback, args=(), kwargs={}):
        """
        Set a callback function triggered on the rising edge

        The callback function will be called every time the clock rises

        Args:
            callback: the callback function triggered on the rising edge
            args: the args of the callback function
            kwargs: the keyword args of the callback function
        """
        self.xclock.StepRis(callback, args, kwargs)

    def StepFal(self, callback, args=(), kwargs={}):
        """
        Set a callback function triggered on the falling edge

        The callback function will be called every time the clock falls

        Args:
            callback: the callback function triggered on the falling edge
            args: the args of the callback function
            kwargs: the keyword args of the callback function
        """
        self.xclock.StepFal(callback, args, kwargs)

    def ResumeWaveformDump(self):
        """
        Restart waveform export and return 1

        Returns:
            bool: return 1 when restart successfully
        """
        return self.dut.ResumeWaveformDump()

    def PauseWaveformDump(self):
        """
        Pause waveform export and return 1

        Returns:
            bool: return 1 when pause successfully
        """
        return self.dut.PauseWaveformDump()

    def WaveformPaused(self) -> int:
        """
        Check if the waveform export has been paused

        Returns:
            bool: return 1 if waveform export is paused
        """
        return self.dut.WaveformPaused()

    def GetXPort(self):
        """
        Get the information of XPort

        Returns:
            XPort: the XPort of dut      
        """
        return self.xport

    def GetXClock(self):
        """
        Get the information of XClock

        Returns:
            XClock: the XClock of dut 
        """
        return self.xclock

    def SetWaveform(self, filename: str):
        """
        Specify the output file for waveform

        Args:
            filename(str):the name of file   
        """
        self.dut.SetWaveform(filename)

    def GetWaveFormat(self) -> str:
        """
        Get the waveform extension, or an empty string if disabled.

        Returns:
            str: The extension of waveform file.
        """
        return self.dut.GetWaveFormat()

    def FlushWaveform(self):
        """
        Flush waveform to the file
        """
        self.dut.FlushWaveform()

    def SetCoverage(self, filename: str):
        """
        Specify the output file for coverage

        Args:
            filename(str):the name of file  
        """
        self.dut.SetCoverage(filename)

    def GetCovMetrics(self) -> int:
        """
        Get the bitmask for collected coverage metrics. 0 means coverage is disabled

        Returns:
            int: Collected coverage metrics bitmask:
                - Bit 0: line   (Line coverage)
                - Bit 1: cond   (Condition coverage)
                - Bit 2: fsm    (Finite-State Machine coverage)
                - Bit 3: toggle (Toggle coverage)
                - Bit 4: branch (Branch coverage)
                - Bit 5: assert (Assertion coverage)
        """
        return self.dut.GetCovMetrics()
    
    def CheckPoint(self, name: str) -> int:
        """
        Save current simulation state to a file

        Args:
            name(str): the name of file    
        """
        self.dut.CheckPoint(name)

    def Restore(self, name: str) -> int:
        """
        Restore simulation state from a file

        Args:
            name(str): the name of file  
        """
        self.dut.Restore(name)

    def GetInternalSignal(self, name: str, index=-1, is_array=False, use_vpi=False):
        """
        Get internal signal objects by variable name

        Args:
            name(str): the name of the internal signal
            index(int): 
            is_array(bool): check if array signal
            use_vpi(bool): True if the picker compilation uses VPI

        Returns:
            list: the internal signal list
        """
        if name not in self.internal_signals:
            signal = None
            if self.dut.GetXSignalCFGBasePtr() != 0 and not use_vpi:
                xname = "CFG:" + name
                if is_array:
                    assert index < 0, "Index is not supported for array signal"
                    signal = self.xcfg.NewXDataArray(name, xname)
                elif index >= 0:
                    signal = self.xcfg.NewXData(name, index, xname)
                else:
                    signal = self.xcfg.NewXData(name, xname)
            else:
                assert index < 0, "Index is not supported for VPI signal"
                assert not is_array, "Array is not supported for VPI signal"
                signal = xsp.XData.FromVPI(self.dut.GetVPIHandleObj(name),
                                           self.dut.GetVPIFuncPtr("vpi_get"),
                                           self.dut.GetVPIFuncPtr("vpi_get_value"),
                                           self.dut.GetVPIFuncPtr("vpi_put_value"), "VPI:" + name)
                if use_vpi:
                    assert signal is not None, f"Internal signal {name} not found (Check VPI is enabled)"
            if signal is None:
                return None
            if not isinstance(signal, xsp.XData):
                self.internal_signals[name] = [xsp.XPin(s, self.event) for s in signal]
            else:
                self.internal_signals[name] = xsp.XPin(signal, self.event)
        return self.internal_signals[name]

    def GetInternalSignalList(self, prefix="", deep=99, use_vpi=False):
        """
        Get the internal signal list

        Args:
            prefix(str): the prefix of the internal signals
            deep(int): specify the number of hierarchy levels below each top module instance
            use_vpi(bool): True if the picker compilation uses VPI

        Returns:
            list: the internal signal list
        """
        if self.dut.GetXSignalCFGBasePtr() != 0 and not use_vpi:
            return self.xcfg.GetSignalNames(prefix)
        else:
            return self.dut.VPIInternalSignalList(prefix, deep)

    def VPIInternalSignalList(self, prefix="", deep=99):
        """
        Get the internal signal list (requires enabling '--vpi' flag when picker compilation)

        Args:
            prefix(str): the prefix of the internal signals
            deep(int): specify the number of hierarchy levels below each top module instance
        
        Returns:
            list: the internal signal list
        """
        return self.dut.VPIInternalSignalList(prefix, deep)

    def Finish(self):
        """
        End the simulation, and keep the result files (waveforms, coverage, etc.).
        """
        self.dut.Finish()

    def RefreshComb(self):
        """
        Refresh the combinational logic state of the circuit
        """
        self.dut.RefreshComb()

    ################################
    #      End of User APIs        #
    ################################

    def __getitem__(self, key):
        return xsp.XPin(self.port[key], self.event)

    # Async APIs wrapped from XClock
    async def AStep(self,i: int):
        """
        Asynchronous wait for clock to push i cycle

        Args:
            i(int): wait for i clock cycles
        """
        return await self.xclock.AStep(i)

    async def ACondition(self,fc_cheker):
        """
        Asynchronous waiting condition is true

        Args:
            fc_cheker(bool): Asynchronous waiting condition
        """
        return await self.xclock.ACondition(fc_cheker)

    def RunStep(self,i: int):
        """
        Drive clock signal, continuously push clock by i clock cycles

        Args:
            i(int): push i clock cycles
        """
        return self.xclock.RunStep(i)

    def __setattr__(self, name, value):
        assert not isinstance(getattr(self, name, None),
                              (xsp.XPin, xsp.XData)), \
        f"XPin and XData of DUT are read-only, do you mean to set the value of the signal? please use `{name}.value = ` instead."
        return super().__setattr__(name, value)


if __name__=="__main__":
    dut=DUT{{__TOP_MODULE_NAME__}}()
    dut.Step(100)
