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

local function require_ut(path, module_name)
    local current_dir = get_current_directory()
    local ut_dir = current_dir .. path
    if directory_exists(ut_dir) then
        package.path = package.path .. ";" .. ut_dir .. "/?.lua"
        return require(module_name)
    else
        return require(module_name)
    end
end


local UT_CacheCFG = require_ut("UT_CacheCFG", "UT_CacheCFG")


local function main(args)
    local dut = UT_CacheCFG.new()
    print("internal signals: ")
    local signals = dut:GetInternalSignalList()
    for i = 0, signals:size() - 1 do
        print(signals[i])
    end
    local valid = dut:GetInternalSignal("CacheCFG_top.io_out_mem_req_valid")
    print("valid: ", valid:AsInt32())
    dut:Finish()
end

local args = {...}
main(args)
