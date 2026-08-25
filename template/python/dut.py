#coding=utf8

try:
    from . import xspcomm as xsp
except Exception as e:
    import xspcomm as xsp
import os as _os
import warnings as _warnings

if __package__ or "." in __name__:
    from .libUT_{{__TOP_MODULE_NAME__}} import *
else:
    from libUT_{{__TOP_MODULE_NAME__}} import *

{% if __SIMULATOR__ == "vcs" and __VERDI_MODE__ == "modern" %}
# After import, promote libvcsnew.so to RTLD_GLOBAL for VCS 2024.09+.
# At this point the library is already loaded through the dependency chain.
# RTLD_NOLOAD|RTLD_GLOBAL only changes symbol visibility and does not reload it.
import ctypes as _ctypes
_vcs_home = _os.environ.get("VCS_HOME", "")
if _vcs_home:
    _libvcsnew = _os.path.join(_vcs_home, "linux64/lib/libvcsnew.so")
    if _os.path.exists(_libvcsnew):
        _RTLD_NOLOAD = 0x4  # Do not reload; only update the symbol scope.
        try:
            _ctypes.CDLL(_libvcsnew, mode=_RTLD_NOLOAD | _ctypes.RTLD_GLOBAL)
        except OSError:
            pass
del _ctypes
{% endif %}



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
        self.xclock.Add(self.xport[name])

    def Step(self, i:int = 1):
        self.xclock.Step(i)

    def StepRis(self, callback, args=(), kwargs={}):
        self.xclock.StepRis(callback, args, kwargs)

    def StepFal(self, callback, args=(), kwargs={}):
        self.xclock.StepFal(callback, args, kwargs)

    def ResumeWaveformDump(self):
        return self.dut.ResumeWaveformDump()

    def PauseWaveformDump(self):
        return self.dut.PauseWaveformDump()

    def WaveformPaused(self) -> int:
        """ Returns 1 if waveform export is paused """
        return self.dut.WaveformPaused()

    def GetXPort(self):
        return self.xport

    def GetXClock(self):
        return self.xclock

    def SetWaveform(self, filename: str):
        self.dut.SetWaveform(filename)

    def GetWaveFormat(self) -> str:
        """
        Get the waveform extension, or an empty string if disabled.

        Returns:
            str: The extension of waveform file.
        """
        return self.dut.GetWaveFormat()

    def FlushWaveform(self):
        self.dut.FlushWaveform()

    def SetCoverage(self, filename: str):
        self.dut.SetCoverage(filename)

    def ResetCoverage(self):
        self.dut.ResetCoverage()

    def FlushCoverage(self) -> int:
        """Finalize a queryable coverage snapshot without destroying the DUT."""
        return self.dut.FlushCoverage()

    def GetCoveragePath(self) -> str:
        return self.dut.GetCoveragePath()

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

    def GetSimulator(self) -> str:
        """Return the simulator selected when this DUT was exported."""
        return "{{__SIMULATOR__}}"

    def GetCoverage(self, kind=None, module=None, instance=None, test=None):
        """
        Query the existing simulator coverage database.

        Call FlushCoverage() before querying a running DUT, or query after
        Finish(). The default query requests line coverage only.

        Args:
            kind: A coverage kind or iterable of kinds.
            module: Optional module-name filter.
            instance: Optional hierarchical instance-prefix filter.
            test: Optional VCS testdata name.

        Returns:
            xspcomm.CoverageReport: Summary metrics and uncovered items.
        """
        path = self.GetCoveragePath()
{% if __SIMULATOR__ == "vcs" or __SIMULATOR__ == "verilator" %}
        workspace = _os.path.dirname(_os.path.abspath(__file__))
        executable = _os.path.join(workspace, "coverage", "coverage")
        if not _os.path.isfile(executable):
            raise RuntimeError(
                "Coverage query executable was not generated. Re-export with --coverage."
            )

        kinds = ["line"] if kind is None else (
            [kind] if isinstance(kind, str) else list(kind)
        )
{% if __SIMULATOR__ == "verilator" %}
        if kinds != ["line"]:
            raise ValueError("Verilator coverage queries support only kind='line'")
        if test is not None:
            raise ValueError("Verilator coverage.dat has no named testdata")
{% endif %}
        query = xsp.CoverageQuery()
        for name in kinds:
            parsed = xsp.CoverageKindFromString(str(name))
            if xsp.CoverageKindName(parsed) == "unknown":
                raise ValueError(f"unknown coverage kind: {name}")
            query.kinds.push_back(parsed)
        query.module = "" if module is None else str(module)
        query.instance = "" if instance is None else str(instance)
        if test is not None:
            query.tests.push_back(str(test))

        provider = xsp.CoverageProviderInfo()
        provider.simulator = "{{__SIMULATOR__}}"
        provider.database = path
        provider.executable = executable
        client = xsp.CoverageClient(provider)
        report_obj = client.Query(query)
        if not report_obj.success:
            error = client.LastError()
            if not error and report_obj.errors:
                error = report_obj.errors[0]
            raise RuntimeError(error or "coverage query failed")
        for warning in report_obj.errors:
            _warnings.warn(str(warning), RuntimeWarning, stacklevel=2)
        return report_obj
{% else %}
        raise RuntimeError("GetCoverage is not implemented for simulator '{{__SIMULATOR__}}'")
{% endif %}

    def PrintCoverage(self, kind=None, module=None, instance=None, test=None):
        """Print and return the report produced by GetCoverage()."""
        return xsp.print_coverage_report(
            self.GetCoverage(
                kind=kind,
                module=module,
                instance=instance,
                test=test,
            )
        )
    
    def CheckPoint(self, name: str) -> int:
        self.dut.CheckPoint(name)

    def Restore(self, name: str) -> int:
        self.dut.Restore(name)

    def GetInternalSignal(self, name: str, index=-1, is_array=False, use_vpi=False):
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
        if self.dut.GetXSignalCFGBasePtr() != 0 and not use_vpi:
            return self.xcfg.GetSignalNames(prefix)
        else:
            return self.dut.VPIInternalSignalList(prefix, deep)

    def VPIInternalSignalList(self, prefix="", deep=99):
        return self.dut.VPIInternalSignalList(prefix, deep)

    def Finish(self):
        return self.dut.Finish()

    def RefreshComb(self):
        self.dut.RefreshComb()

    def AtClone(self):
        """Re-init simulator state in child after fork."""
        return self.dut.atClone()

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
