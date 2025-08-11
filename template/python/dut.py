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
    """Python wrapper for the DUT (Device Under Test) circuit, based on Verilog DPI-C and similar interfaces.

    This class provides a unified interface for simulation, including clock control, pin access, waveform/coverage management, and internal signal inspection.

    Typical usage:
        1. Instantiate: `dut = DUT{{__TOP_MODULE_NAME__}}()`
        2. Initialize clock: `dut.InitClock("clk")`
        3. Set input pin: `dut.pin_name.value = 1`
        4. Drive circuit: `dut.Step()` (sequential) or `dut.RefreshComb()` (combinational)
        5. Read output pin: `val = dut.pin_name.value`
        6. Finish: `del dut` or `dut.Finish()`

    Pin semantics:
        - Pins are XData objects. Assign with `x.value = ...`, read with `y = x.value`.
        - By default, assignments take effect on the next clock edge (use `Step()`).
        - For immediate assignment (combinational), call `x.AsImmWrite()` and use `RefreshComb()`.
        - After driving, all pin values reflect the latest DUT state.

    Additional features:
        - Internal signal access via `GetInternalSignal()` and `GetInternalSignalList()`.
        - Waveform and coverage file management.
        - Async and callback support for advanced testbench scenarios.
    """
    # Initialize
    def __init__(self, *args, **kwargs):
        self.dut = DutUnifiedBase(*args)
        self.xclock = xsp.XClock(self.dut.pxcStep, self.dut.pSelf)
        self.xport  = xsp.XPort()
        self.xclock.Add(self.xport)
        self.event = self.xclock.getEvent()
        self.internal_signals = {}
        self.xcfg = xsp.XSignalCFG(self.dut.GetXSignalCFGPath(), self.dut.GetXSignalCFGBasePtr())

        # Set output files
        if kwargs.get("waveform_filename"):
            self.dut.SetWaveform(kwargs.get("waveform_filename"))
        if kwargs.get("coverage_filename"):
            self.dut.SetCoverage(kwargs.get("coverage_filename"))

        # All pins:
{{__XDATA_INIT__}}

        # BindDPI
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
        """Initialize the clock signal
        Args:
            name (str): The name of the clock signal. this name should be the same as the one used in the DUT.
        """
        self.xclock.Add(self.xport[name])

    def Step(self, i:int = 1):
        """Step the clock signal
        Args:
            i (int, optional): The number of steps to take. Defaults to 1.
        """
        self.xclock.Step(i)

    def StepRis(self, callback, args=(), kwargs={}):
        """Add a clock signal rising edge callback function
        Args:
            callback (function): The callback function to be called on the rising edge of the clock signal.
            args (tuple, optional): The arguments to be passed to the callback function. Defaults to ().
            kwargs (dict, optional): The keyword arguments to be passed to the callback function. Defaults to {}.
        """
        self.xclock.StepRis(callback, args, kwargs)

    def StepFal(self, callback, args=(), kwargs={}):
        """Add a clock signal falling edge callback function
        Args:
            callback (function): The callback function to be called on the falling edge of the clock signal.
            args (tuple, optional): The arguments to be passed to the callback function. Defaults to ().
            kwargs (dict, optional): The keyword arguments to be passed to the callback function. Defaults to {}.
        """
        self.xclock.StepFal(callback, args, kwargs)

    def OpenWaveform(self):
        """Open the waveform file for writing."""
        return self.dut.OpenWaveform()

    def CloseWaveform(self):
        """Close the waveform file."""
        return self.dut.CloseWaveform()

    def GetXPort(self):
        """Get the XPort object
        Returns:
            XPort: The XPort object.
        """
        return self.xport

    def GetXClock(self):
        """Get the XClock object
        Returns:
            XClock: The XClock object.
        """
        return self.xclock

    def SetWaveform(self, filename: str):
        """Set the waveform file name
        Args:
            filename (str): The name of the waveform file.
        """
        self.dut.SetWaveform(filename)
    
    def FlushWaveform(self):
        """Flush the waveform file"""
        self.dut.FlushWaveform()

    def SetCoverage(self, filename: str):
        """Set the coverage file name
        Args:
            filename (str): The name of the coverage file.
        """
        self.dut.SetCoverage(filename)
    
    def CheckPoint(self, name: str) -> int:
        """Set a checkpoint
        Args:
            name (str): The name of the checkpoint.
        """
        self.dut.CheckPoint(name)

    def Restore(self, name: str) -> int:
        """Restore to a checkpoint
        Args:
            name (str): The name of the checkpoint.
        """
        self.dut.Restore(name)

    def GetInternalSignal(self, name: str, index=-1, is_array=False, use_vpi=False):
        """Get an internal signal
        Args:
            name (str): The name of the internal signal.
            index (int, optional): The index of the internal signal. Defaults to -1.
            is_array (bool, optional): Whether the internal signal is an array. Defaults to False.
            use_vpi (bool, optional): Whether to use VPI. Defaults to False.
        Returns:
            XData wrapper (XPin): The internal signal object or None if not found.
        """
        key_name = name
        if index >= 0:
            key_name = f"{name}[{index}]"
        if key_name not in self.internal_signals:
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
                self.internal_signals[key_name] = [xsp.XPin(s, self.event) for s in signal]
            else:
                self.internal_signals[key_name] = xsp.XPin(signal, self.event)
        return self.internal_signals[key_name]

    def GetInternalSignalList(self, prefix="", deep=99, use_vpi=False):
        """Get a list of internal signals
        Args:
            prefix (str, optional): The prefix of the internal signals. Defaults to "".
            deep (int, optional): The depth of the internal signals. Defaults to 99.
            use_vpi (bool, optional): Whether to use VPI. Defaults to False.
        Returns:
            list(list(string)): A list of internal signal names.
        """
        if self.dut.GetXSignalCFGBasePtr() != 0 and not use_vpi:
            return self.xcfg.GetSignalNames(prefix)
        else:
            return self.dut.VPIInternalSignalList(prefix, deep)

    def VPIInternalSignalList(self, prefix="", deep=99):
        """Get a list of internal signals using VPI
        Args:
            prefix (str, optional): The prefix of the internal signals. Defaults to "".
            deep (int, optional): The depth of the internal signals. Defaults to 99.
        Returns:
            list(list(string)): A list of internal signal names.
        """
        return self.dut.VPIInternalSignalList(prefix, deep)

    def Finish(self):
        """Finish the DUT and release resources"""
        self.dut.Finish()

    def RefreshComb(self):
        """Refresh the combinational logic"""
        self.dut.RefreshComb()

    ################################
    #      End of User APIs        #
    ################################

    def __getitem__(self, key):
        return xsp.XPin(self.port[key], self.event)

    # Async APIs wrapped from XClock
    async def AStep(self,i: int):
        return await self.xclock.AStep(i)

    async def ACondition(self,fc_cheker):
        return await self.xclock.ACondition(fc_cheker)

    def RunStep(self,i: int):
        return self.xclock.RunStep(i)

    def __setattr__(self, name, value):
        assert not isinstance(getattr(self, name, None),
                              (xsp.XPin, xsp.XData)), \
        f"XPin and XData of DUT are read-only, do you mean to set the value of the signal? please use `{name}.value = ` instead."
        return super().__setattr__(name, value)


if __name__=="__main__":
    dut=DUT{{__TOP_MODULE_NAME__}}()
    dut.Step(100)
