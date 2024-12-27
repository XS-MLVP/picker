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


local function directory_exists(path)
    local f = io.popen("cd " .. path .. " && pwd")
    local result = f:read("*a")
    f:close()
    return result ~= ""
end

local function require_ut_adder(path, module_name)
    local current_dir = get_current_directory()
    local ut_adder_dir = current_dir .. path
    if directory_exists(ut_adder_dir) then
        package.path = package.path .. ";" .. ut_adder_dir .. "/?.lua"
        return require(module_name)
    else
        return require(module_name)
    end
end


local UT_vpi = require_ut_adder("UT_vpi", "UT_vpi")


local function main(args)
    local dut = UT_vpi.new()
    print("internal signals: ")
    local signals = dut:VPIInternalSignalList("", 99)
    for i = 0, signals:size() - 1 do
        print(signals[i])
    end
    local v1 = dut:GetInternalSignal("vpi._v1_base")
    local v2 = dut:GetInternalSignal("vpi._v2_base")
    local v3 = dut:GetInternalSignal("vpi._v3_base")
    local v4 = dut:GetInternalSignal("vpi._v4_base")
    dut:InitClock("clk")
    print("data size: v1" .. v1:W() .. " v2:" .. v2:W() .. " v3:" .. v3:W() .. " v4:" .. v4:W())
    print("------------------step-------------------")

    for i = 0, 19 do
        dut:Step(1)
        dut:FlushWaveform()
        if i == 10 then
            -- write to internal signals
            v1:Set(1) -- 1 bit, type logic, .W() is 0
            v2:Set(10) -- 8 bits
            v3:Set(20) -- 32 bits
            v4:Set(30) -- 64 bits
        end
        print(string.format("%d v1:%d v2:%d v3:%d v4:%d | _v1:%d, _v2:%d, _v3:%d, _v4:%d",
            i, dut.v1:U(), dut.v2:U(), dut.v3:U(), dut.v4:U(), v1:U(), v2:U(), v3:U(), v4:U()))
    end
    dut:Finish()
end

local args = {...}
main(args)
