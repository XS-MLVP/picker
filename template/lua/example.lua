
local function directory_exists(path)
    local f = io.popen("cd " .. path .. " && pwd")
    local result = f:read("*a")
    f:close()
    return result ~= ""
end

local function get_current_directory()
    local info = debug.getinfo(1, "S")
    local path = info.source:match("^@(.*/)")
    if path == nil then
        path = "./"
    end
    return path
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


local DUT = require_ut_adder("UT_{{__TOP_MODULE_NAME__}}", "UT_{{__TOP_MODULE_NAME__}}")

dut = DUT:new()
dut:Step(10)
print("run 10 steps done")
dut:Finish()
