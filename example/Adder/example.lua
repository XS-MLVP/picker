
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


local DUTAdder = require_ut_adder("UT_Adder", "UT_Adder")

-- Tests
dut = DUTAdder:new()

local function random()
    return math.random(0, 0xFFF)
end

local function Assert(condition, message)
    if not condition then
        error(message)
    end
end

local input_t = {a = 0, b = 0, cin = 0}
local output_t = {sum = 0, cout = 0}

for c = 1, 30 do
    local i = {a = random(), b = random(), cin = random() & 1}
    local o_dut = {sum = 0, cout = 0}
    local o_ref = {sum = 0, cout = 0}

    local function dut_cal()
        dut.a:Set(i.a)
        dut.b:Set(i.b)
        dut.cin:Set(i.cin)
        dut:Step(1)
        o_dut.sum = dut.sum:U()
        o_dut.cout = dut.cout:U()
    end

    local function ref_cal()
        local sum = i.a + i.b
        local carry = sum < i.a
        sum = sum + i.cin
        carry = carry or sum < i.cin
        o_ref.sum = sum
        o_ref.cout = carry and 1 or 0
    end

    dut_cal()
    ref_cal()
    print(string.format("[cycle %d] a=0x%X, b=0x%X, cin=0x%X", c, i.a, i.b, i.cin))
    print(string.format("DUT: sum=0x%X, cout=0x%X", o_dut.sum, o_dut.cout))
    print(string.format("REF: sum=0x%X, cout=0x%X", o_ref.sum, o_ref.cout))
    Assert(o_dut.sum == o_ref.sum, "sum mismatch")
    Assert(o_dut.cout == o_ref.cout, "cout mismatch")
end

dut:Finish()
print("Test Passed, destroy DUTAdder")
