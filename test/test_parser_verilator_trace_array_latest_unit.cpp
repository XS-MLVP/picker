#include <cassert>
#include <filesystem>
#include <string>
#include <vector>

#include "parser/verilator_trace.hpp"

// Verilator 5.054 emits array declarations two ways in one file:
//   - non-decomposed arrays keep the dtype helper call
//   - decomposed arrays are inlined as PUSH/loop/DECL_ARRAY and gain
//     `__BRA__N__KET__` component members
// Both must map back to the root storage.
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
        (fixture / "VArrTop__Trace__0__Slow.cpp").string(),
        (fixture / "VArrTop__Trace__0.cpp").string(),
    };
    std::vector<picker::cpp_variableInfo> vars = {
        {"ArrTop_top__DOT__ArrTop__DOT__mem", "CData", 8, 4, 0},
        {"ArrTop_top__DOT__ArrTop__DOT__shadow", "CData", 8, 4, 1},
        {"ArrTop_top__DOT__ArrTop__DOT__shadow__BRA__0__KET__", "CData", 8, 0, 3},
        {"ArrTop_top__DOT__ArrTop__DOT__shadow__BRA__1__KET__", "CData", 8, 0, 4},
        {"ArrTop_top__DOT__ArrTop__DOT__shadow__BRA__2__KET__", "CData", 8, 0, 5},
        {"ArrTop_top__DOT__ArrTop__DOT__shadow__BRA__3__KET__", "CData", 8, 0, 6},
        {"ArrTop_top__DOT__clk", "CData", 1, 0, 9},
        {"ArrTop_top__DOT__din", "CData", 8, 0, 10},
        {"ArrTop_top__DOT__dout", "CData", 8, 0, 11},
    };
    const auto result = picker::parser::verilator::processTraceFiles(files, vars);

    const auto *mem = find_signal(result.signals, "ArrTop_top.ArrTop.mem");
    const auto *shadow = find_signal(result.signals, "ArrTop_top.ArrTop.shadow");
    const auto *module_out = find_signal(result.signals, "ArrTop_top.ArrTop.dout");
    const auto *top_out = find_signal(result.signals, "ArrTop_top.dout");
    assert(mem && shadow && module_out && top_out);

    // dtype helper and inlined loop must both bind the whole array to storage.
    assert(mem->kind == "direct" && mem->root_name == "ArrTop_top.ArrTop.mem");
    assert(shadow->kind == "direct" && shadow->root_name == "ArrTop_top.ArrTop.shadow");
    // 5.054 decomposes the array that feeds a full copy; the module-local
    // output is then sampled from that component slot rather than the array.
    assert(module_out->kind == "direct"
           && module_out->root_name == "ArrTop_top.ArrTop.shadow__BRA__2__KET__");
    assert(top_out->kind == "direct" && top_out->root_name == "ArrTop_top.dout");
    return 0;
}
