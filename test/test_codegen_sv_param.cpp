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
    mod.module_nums = 1;
    mod.pins.push_back({"a", "input", 3, 0});
    mod.pins.push_back({"b", "output", -1, 0});

    std::vector<picker::sv_module_define> modules{mod};
    std::vector<picker::sv_signal_define> internal{
        {"sig0", "output", 7, 0},
    };

    nlohmann::json signal_tree;
    auto external = picker::codegen::gen_sv_param(
        data,
        modules,
        internal,
        signal_tree,
        "wave.vcd",
        "verilator",
        picker::SignalAccessType::DPI
    );

    assert(external.size() == 2);

    const auto logic_decl = data["__LOGIC_PIN_DECLARATION__"].get<std::string>();
    assert(contains_line(logic_decl, "logic [3:0] a;"));
    assert(contains_line(logic_decl, "b;"));

    const auto dpi_export = data["__DPI_FUNCTION_EXPORT__"].get<std::string>();
    assert(contains_line(dpi_export, "get_a"));
    assert(contains_line(dpi_export, "set_a"));
    assert(contains_line(dpi_export, "get_sig0"));

    const auto sv_wave = data["__SV_DUMP_WAVE__"].get<std::string>();
    assert(contains_line(sv_wave, "wave.vcd"));

    const auto tree = data["__SIGNAL_TREE__"].get<std::string>();
    assert(contains_line(tree, "a"));
    assert(contains_line(tree, "b"));

    // MEM_DIRECT adds public_flat markers
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
        picker::SignalAccessType::MEM_DIRECT
    );
    const auto logic_decl2 = data2["__LOGIC_PIN_DECLARATION__"].get<std::string>();
    assert(contains_line(logic_decl2, "public_flat_rw_on"));
    assert(contains_line(logic_decl2, "public_off"));

    return 0;
}
