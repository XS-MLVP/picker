#include <cassert>
#include <string>
#include <vector>

#include "codegen/sv.hpp"

bool picker::is_debug = false;
char *picker::lib_random_hash = const_cast<char *>("H");

static bool contains_line(const std::string &haystack, const std::string &needle)
{
    return haystack.find(needle) != std::string::npos;
}

int main()
{
    nlohmann::json data;
    data["__TOP_MODULE_NAME__"] = "Top";

    picker::sv_module_define mod;
    mod.module_name = "dut";
    mod.module_nums = 2;
    mod.pins.push_back({"clk", "input", -1, 0});
    mod.pins.push_back({"rst", "input", -1, 0});

    std::vector<picker::sv_module_define> modules{mod};
    std::vector<picker::sv_signal_define> internal{};

    nlohmann::json signal_tree;
    picker::codegen::gen_sv_param(
        data,
        modules,
        internal,
        signal_tree,
        "dump.fsdb",
        "vcs",
        picker::SignalAccessType::DPI
    );

    const auto inner = data["__INNER_MODULES__"].get<std::string>();
    assert(contains_line(inner, "dut dut_0("));
    assert(contains_line(inner, "dut dut_1("));

    const auto logic_decl = data["__LOGIC_PIN_DECLARATION__"].get<std::string>();
    assert(contains_line(logic_decl, "logic  dut_0_clk;"));
    assert(contains_line(logic_decl, "logic  dut_1_rst;"));

    const auto sv_wave = data["__SV_DUMP_WAVE__"].get<std::string>();
    assert(contains_line(sv_wave, "$fsdbDumpfile(\"dump.fsdb\")"));
    assert(data["__TRACE__"].get<std::string>() == "fsdb");

    nlohmann::json data2;
    data2["__TOP_MODULE_NAME__"] = "Top";
    nlohmann::json signal_tree2;
    picker::codegen::gen_sv_param(
        data2,
        modules,
        internal,
        signal_tree2,
        "",
        "verilator",
        picker::SignalAccessType::DPI
    );
    assert(data2["__TRACE__"].get<std::string>() == "OFF");
    assert(data2["__SV_DUMP_WAVE__"].get<std::string>().empty());

    return 0;
}
