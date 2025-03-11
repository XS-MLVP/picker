
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
    self.internal_signals = {}

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

function DUT{{__TOP_MODULE_NAME__}}:SetWaveform(filename)
    self.dut:SetWaveform(filename)
end

function DUT{{__TOP_MODULE_NAME__}}:FlushWaveform()
    self.dut:FlushWaveform()
end

function DUT{{__TOP_MODULE_NAME__}}:SetCoverage(filename)
    self.dut:SetCoverage(filename)
end

function DUT{{__TOP_MODULE_NAME__}}:CheckPoint(name)
    self.dut:CheckPoint(name)
end

function DUT{{__TOP_MODULE_NAME__}}:Restore(name)
    self.dut:Restore(name)
end

function DUT{{__TOP_MODULE_NAME__}}:GetInternalSignal(name)
    if not self.internal_signals[name] then
        local signal = xsp.XData.FromVPI(self.dut:GetVPIHandleObj(name),
                                        self.dut:GetVPIFuncPtr("vpi_get"),
                                        self.dut:GetVPIFuncPtr("vpi_get_value"),
                                        self.dut:GetVPIFuncPtr("vpi_put_value"), name)
        if signal then
            self.internal_signals[name] = signal
        end
    end
    return self.internal_signals[name]
end

function DUT{{__TOP_MODULE_NAME__}}:VPIInternalSignalList(prefix, deep)
    return self.dut:VPIInternalSignalList(prefix, deep)
end

function DUT{{__TOP_MODULE_NAME__}}:RefreshComb()
    self.dut:RefreshComb()
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
