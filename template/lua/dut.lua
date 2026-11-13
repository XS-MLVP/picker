
local function file_exists(name)
    local f = io.open(name, "r")
    if f then
        f:close()
        return true
    else
        return false
    end
end

local function get_current_directory()
    local info = debug.getinfo(1, "S")
    local path = info.source:match("^@(.*/)")
    if path == nil then
        path = "./"
    end
    return path
end

local function require_xspcomm()
    local current_dir = get_current_directory()
    local xspcomm_path = current_dir .. "xspcomm/xspcomm.lua"
    if file_exists(xspcomm_path) then
        package.path = package.path .. ";" .. current_dir .. "xspcomm/?.lua"
        return require("xspcomm")
    else
        return require("xspcomm")
    end
end


local current_directory = get_current_directory()
package.cpath = current_directory .. "?.so;" .. package.cpath

local xsp = require_xspcomm()
local lib = require("_UT_{{__TOP_MODULE_NAME__}}")


DUT{{__TOP_MODULE_NAME__}} = {}
DUT{{__TOP_MODULE_NAME__}}.__index = DUT{{__TOP_MODULE_NAME__}}

local unpack = table.unpack or unpack

function DUT{{__TOP_MODULE_NAME__}}:new(...)
    local args = {...}
    local kwargs = args[#args]
    if type(kwargs) == "table" then
        table.remove(args)
    else
        kwargs = {}
    end
    local self = setmetatable({}, DUT{{__TOP_MODULE_NAME__}})
    self.dut = lib.DutUnifiedBase(unpack(args))
    self.xclock = xsp.XClock(self.dut.pxcStep, self.dut.pSelf)
    self.xport = xsp.XPort()
    self.xclock:Add(self.xport)
    self.xcfg = xsp.XSignalCFG(self.dut:GetXSignalCFGPath(), self.dut:GetXSignalCFGBasePtr())
    self.internal_signals = {}
    self._finished = false
    {% if __HAS_AGGREGATE_VIEWS__ %}
    self.vpi_backend = nil
    {% endif %}

    -- set output files
    if kwargs.waveform_filename then
        self.dut.SetWaveform(kwargs.get("waveform_filename"))
    end
    if kwargs.coverage_filename then
        self.dut.SetCoverage(kwargs.get("coverage_filename"))
    end

    -- all Pins
{{__XDATA_INIT__}}

    -- BindDPI
{{__XDATA_BIND__}}

    -- Add2Port
{{__XPORT_ADD__}}

    -- Cascaded ports
{{__XPORT_CASCADED__}}

    return self
end

function DUT{{__TOP_MODULE_NAME__}}:Finish()
    if self._finished then
        return 0
    end
    self._finished = true
    {% if __HAS_AGGREGATE_VIEWS__ %}
    if self.vpi_backend ~= nil then
        local backend = self.vpi_backend
        self.vpi_backend = nil
        backend:Close()
    end
    {% endif %}
    self.dut:Finish()
end

function DUT{{__TOP_MODULE_NAME__}}:__gc()
    self:Finish()
end

-- User APIs
function DUT{{__TOP_MODULE_NAME__}}:InitClock(name)
    self.xclock:Add(self.xport:Get(name))
end

function DUT{{__TOP_MODULE_NAME__}}:Step(i)
    i = i or 1
    self.xclock:Step(i)
end

function DUT{{__TOP_MODULE_NAME__}}:StepRis(callback)
    self.xclock:StepRis(callback)
end

function DUT{{__TOP_MODULE_NAME__}}:StepFal(callback)
    self.xclock:StepFal(callback)
end

function DUT{{__TOP_MODULE_NAME__}}:ResumeWaveformDump()
    return self.dut:ResumeWaveformDump()
end

function DUT{{__TOP_MODULE_NAME__}}:PauseWaveformDump()
    return self.dut:PauseWaveformDump()
end

function DUT{{__TOP_MODULE_NAME__}}:WaveformPaused()
    return self.dut:WaveformPaused()
end

function DUT{{__TOP_MODULE_NAME__}}:GetXClock()
    return self.xclock
end

function DUT{{__TOP_MODULE_NAME__}}:GetXPort()
    return self.xport
end

function DUT{{__TOP_MODULE_NAME__}}:SetWaveform(filename)
    self.dut:SetWaveform(filename)
end

function DUT{{__TOP_MODULE_NAME__}}:GetWaveFormat()
    self.dut:GetWaveFormat()
end

function DUT{{__TOP_MODULE_NAME__}}:FlushWaveform()
    self.dut:FlushWaveform()
end

function DUT{{__TOP_MODULE_NAME__}}:SetCoverage(filename)
    self.dut:SetCoverage(filename)
end

function DUT{{__TOP_MODULE_NAME__}}:ResetCoverage()
    self.dut:ResetCoverage()
end

function DUT{{__TOP_MODULE_NAME__}}:GetCovMetrics()
    return self.dut:GetCovMetrics()
end

function DUT{{__TOP_MODULE_NAME__}}:CheckPoint(name)
    self.dut:CheckPoint(name)
end

function DUT{{__TOP_MODULE_NAME__}}:Restore(name)
    self.dut:Restore(name)
end

function DUT{{__TOP_MODULE_NAME__}}:GetInternalSignal(name, index, is_array, use_vpi)
    -- name: str, index=-1, is_array=False, use_vpi=False
    index = index or -1
    is_array = is_array or false
    use_vpi = use_vpi or false
    xname = "CFG:" .. name
    if not self.internal_signals[name] then
        local signal = nil
        if not use_vpi then
            if self.dut:GetXSignalCFGBasePtr() == 0 then
                return nil
            end
            if is_array then
                signal = self.xcfg:NewXArray(name)
                if signal then
                    self.internal_signals[name] = signal
                end
                return signal
            end
            if index >= 0 then
                signal = self.xcfg:NewXData(name, index, xname)
            else
                signal = self.xcfg:NewXData(name, xname)
            end
        else
            assert(not is_array, "VPI signal cannot be array")
            assert(index == -1, "VPI signal cannot be indexed")
            signal = xsp.XData.FromVPI(self.dut:GetVPIHandleObj(name),
                                       self.dut:GetVPIFuncPtr("vpi_get"),
                                       self.dut:GetVPIFuncPtr("vpi_get_value"),
                                       self.dut:GetVPIFuncPtr("vpi_put_value"), "VPI:" .. name)
        end
        if signal then
            self.internal_signals[name] = signal
        end
    end
    return self.internal_signals[name]
end

{% if __HAS_AGGREGATE_VIEWS__ %}
function DUT{{__TOP_MODULE_NAME__}}:GetInternalSignalView(name)
    if self.vpi_backend == nil then
        self.vpi_backend = xsp.XVPI()
        self.vpi_backend:Configure(
            self.dut:GetVPIFuncPtr("vpi_get"),
            self.dut:GetVPIFuncPtr("vpi_get_value"),
            self.dut:GetVPIFuncPtr("vpi_put_value"),
            self.dut:GetVPIFuncPtr("vpi_handle_by_name"),
            self.dut:GetVPIFuncPtr("vpi_handle_by_index"),
            self.dut:GetVPIFuncPtr("vpi_handle"),
            self.dut:GetVPIFuncPtr("vpi_iterate"),
            self.dut:GetVPIFuncPtr("vpi_scan"),
            self.dut:GetVPIFuncPtr("vpi_get_str"),
            self.dut:GetVPIFuncPtr("vpi_release_handle"),
            self.dut:GetVPIScope(), {{__AGGREGATE_LAYOUTS_JSON__}})
    end
    return self.vpi_backend:Get(name)
end
{% endif %}

function DUT{{__TOP_MODULE_NAME__}}:GetInternalSignalList(prefix, deep, use_vpi)
    -- prefix: str, deep=99, use_vpi=False
    prefix = prefix or ""
    deep = deep or 99
    use_vpi = use_vpi or false
    if not use_vpi then
        if not self.dut:GetXSignalCFGBasePtr() then
            return nil
        end
        return self.xcfg:GetSignalNames(prefix)
    else
        return self:VPIInternalSignalList(prefix, deep)
    end
end

function DUT{{__TOP_MODULE_NAME__}}:VPIInternalSignalList(prefix, deep)
    return self.dut:VPIInternalSignalList(prefix or "", deep or 99)
end

function DUT{{__TOP_MODULE_NAME__}}:RefreshComb()
    self.xclock:RefreshComb()
end

function DUT{{__TOP_MODULE_NAME__}}:AtClone()
    self.dut:atClone()
end

local function contains(a, x)
    return string.find(a, x) ~= nil
end

if contains(arg[0], "UT_{{__TOP_MODULE_NAME__}}.lua") then
    local dut = DUT{{__TOP_MODULE_NAME__}}:new()
    dut:Step(100)
    dut:Finish()
end

return DUT{{__TOP_MODULE_NAME__}}
