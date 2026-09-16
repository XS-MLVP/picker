#coding=utf8

import json as _json
import keyword as _keyword
from types import MappingProxyType as _MappingProxyType

try:
    from . import xspcomm as xsp
except Exception as e:
    import xspcomm as xsp

if __package__ or "." in __name__:
    from .libUT_{{__TOP_MODULE_NAME__}} import *
else:
    from libUT_{{__TOP_MODULE_NAME__}} import *

{% if __SIMULATOR__ == "vcs" and __VERDI_MODE__ == "modern" %}
# After import, promote libvcsnew.so to RTLD_GLOBAL for VCS 2024.09+.
# At this point the library is already loaded through the dependency chain.
# RTLD_NOLOAD|RTLD_GLOBAL only changes symbol visibility and does not reload it.
import ctypes as _ctypes, os as _os
_vcs_home = _os.environ.get("VCS_HOME", "")
if _vcs_home:
    _libvcsnew = _os.path.join(_vcs_home, "linux64/lib/libvcsnew.so")
    if _os.path.exists(_libvcsnew):
        _RTLD_NOLOAD = 0x4  # Do not reload; only update the symbol scope.
        try:
            _ctypes.CDLL(_libvcsnew, mode=_RTLD_NOLOAD | _ctypes.RTLD_GLOBAL)
        except OSError:
            pass
del _ctypes, _os
{% endif %}


class _SignalView(object):
    """Read-only hierarchy whose leaves are generated XData objects."""

    __slots__ = ("_fields",)

    def __init__(self, fields):
        object.__setattr__(self, "_fields", _MappingProxyType(dict(fields)))

    def __getitem__(self, key):
        return self._fields[key]

    def __iter__(self):
        return iter(self._fields)

    def __len__(self):
        return len(self._fields)

    def keys(self):
        return self._fields.keys()

    def items(self):
        return self._fields.items()

    def __getattr__(self, name):
        key = name[:-1] if _keyword.iskeyword(name[:-1]) else name
        try:
            return self._fields[key]
        except KeyError as error:
            raise AttributeError(name) from error

    def __setattr__(self, name, value):
        raise AttributeError("generated signal hierarchy is read-only")


def _build_signal_view(dut, node, prefix):
    children = {
        key: value for key, value in node.items()
        if key not in ("_", "Pin", "High", "Low")
    }
    if node.get("_") is True:
        return getattr(dut, prefix)
    if children and all(key.isdigit() for key in children):
        indexes = sorted(int(key) for key in children)
        if indexes != list(range(len(indexes))):
            raise ValueError(
                f"signal-tree sequence {prefix!r} must use contiguous indexes"
            )
        return tuple(
            _build_signal_view(dut, children[str(index)], f"{prefix}_{index}")
            for index in indexes
        )
    return _SignalView({
        key: _build_signal_view(
            dut, child, f"{prefix}_{key}" if prefix else key
        )
        for key, child in children.items()
    })


def _attach_signal_hierarchy(dut):
    for key, node in dut.signal_tree.items():
        if node.get("_") is True:
            continue
        top = getattr(dut, key)
        view = _build_signal_view(dut, node, key)
        setattr(top, "_hierarchy", view)
        for field, value in view.items():
            attr = field + "_" if _keyword.iskeyword(field) else field
            if hasattr(type(top), attr):
                # Keep the backend XPort API intact.  A conflicting RTL field
                # remains available through top._hierarchy[field].
                continue
            setattr(top, attr, value)


class DUT{{__TOP_MODULE_NAME__}}(object):

    # initialize
    def __init__(self, *args, **kwargs):
        self.dut = DutUnifiedBase(*args)
        self.xclock = xsp.XClock(self.dut.pxcStep, self.dut.pSelf)
        self.xport  = xsp.XPort()
        self.xclock.Add(self.xport)
        self.event = self.xclock.getEvent()
        self.internal_signals = {}
        self.signal_tree = _json.loads(r'''{{__SIGNAL_TREE_JSON__}}''')
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

        # Structured access over the same XData leaves.  Top-level aggregates
        # remain XPort instances for compatibility with batch backend APIs.
        _attach_signal_hierarchy(self)

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
                self.internal_signals[name] = list(signal)
            else:
                self.internal_signals[name] = signal
        return self.internal_signals[name]

    def GetInternalSignalList(self, prefix="", deep=99, use_vpi=False):
        if self.dut.GetXSignalCFGBasePtr() != 0 and not use_vpi:
            return self.xcfg.GetSignalNames(prefix)
        else:
            return self.dut.VPIInternalSignalList(prefix, deep)

    def VPIInternalSignalList(self, prefix="", deep=99):
        return self.dut.VPIInternalSignalList(prefix, deep)

    def Finish(self):
        self.dut.Finish()

    def RefreshComb(self):
        self.dut.RefreshComb()

    def AtClone(self):
        """Re-init simulator state in child after fork."""
        return self.dut.atClone()

    ################################
    #      End of User APIs        #
    ################################

    def __getitem__(self, key):
        return self.port[key]

    # Async APIs wrapped from XClock
    async def AStep(self,i: int):
        return await self.xclock.AStep(i)

    async def ACondition(self,fc_cheker):
        return await self.xclock.ACondition(fc_cheker)

    def RunStep(self,i: int):
        return self.xclock.RunStep(i)

    def __setattr__(self, name, value):
        assert not isinstance(getattr(self, name, None),
                              xsp.XData), \
        f"XData attributes of DUT are read-only; set the signal with `{name}.value = ...` instead."
        return super().__setattr__(name, value)


if __name__=="__main__":
    dut=DUT{{__TOP_MODULE_NAME__}}()
    dut.Step(100)
