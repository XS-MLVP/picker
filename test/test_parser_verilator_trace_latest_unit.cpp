#include <cassert>
#include <filesystem>
#include <string>
#include <vector>

#include "parser/verilator_trace.hpp"

static const picker::parser::trace_signal_info *find_signal(
    const std::vector<picker::parser::trace_signal_info> &signals, const std::string &name)
{
    for (const auto &signal : signals) {
        if (signal.name == name) return &signal;
    }
    return nullptr;
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    namespace fs = std::filesystem;
    const fs::path fixture = argv[1];
    std::vector<std::string> files = {
        (fixture / "VAdder__Trace__0__Slow.cpp").string(),
        (fixture / "VAdder__Trace__0.cpp").string(),
    };
    std::vector<picker::cpp_variableInfo> vars = {
        {"Adder_top__DOT__a", "VlWide<4>", 128, 0, 0},
        {"Adder_top__DOT__b", "VlWide<4>", 128, 0, 4},
        {"Adder_top__DOT__cin", "CData", 1, 0, 8},
        {"Adder_top__DOT__sum", "VlWide<4>", 128, 0, 9},
        {"Adder_top__DOT__cout", "CData", 1, 0, 13},
    };
    const auto result = picker::parser::verilator::processTraceFiles(files, vars);

    const auto *a = find_signal(result.signals, "Adder_top.a");
    const auto *cin = find_signal(result.signals, "Adder_top.cin");
    const auto *nested_a = find_signal(result.signals, "Adder_top.Adder.a");
    const auto *nested_sum = find_signal(result.signals, "Adder_top.Adder.sum");
    assert(a && cin && nested_a && nested_sum);
    assert(a->kind == "direct" && a->root_name == "Adder_top.a" && a->width == 128);
    assert(cin->kind == "direct" && cin->root_name == "Adder_top.cin" && cin->width == 1);
    assert(nested_a->kind == "direct" && nested_a->root_name == "Adder_top.a");
    // The child instance sum is sampled from a computed temporary, so it is
    // kept as an expression rather than being mistaken for direct storage.
    assert(nested_sum->kind == "expr" && nested_sum->source_expr == "__Vtemp_3");
    assert(result.matched_slot_count >= 5);
    return 0;
}
