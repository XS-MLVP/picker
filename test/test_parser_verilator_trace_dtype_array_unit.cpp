#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <map>
#include "parser/verilator_trace.hpp"

int main(int argc, char** argv) {
    assert(argc == 2 || argc == 3);
    const int mem_slot = argc == 3 ? std::stoi(argv[2]) : 3;
    namespace fs = std::filesystem;
    fs::path fixture(argv[1]);
    std::vector<std::string> files = {(fixture / "VArrTop__Trace__0.cpp").string(),
        (fixture / "VArrTop__Trace__0__Slow.cpp").string()};
    // Mirrors the layout produced from a Verilator 5.052 root header:
    // ports live at the root, unpacked arrays are VlUnpacked<..., N> members.
    std::vector<picker::cpp_variableInfo> vars = {
        {"clk", "CData", 1, 0, 0},
        {"din", "CData", 8, 0, 1},
        {"dout", "CData", 8, 0, 2},
        {"ArrTop__DOT__mem", "SData", 16, 4, 3},
        {"ArrTop__DOT__tiny", "CData", 8, 2, 11}};
    for (int order = 0; order < 2; ++order) {
        auto result = picker::parser::verilator::processTraceFiles(
            (fixture / "VArrTop___024root.h").string(), files, vars);
        std::map<std::string, picker::parser::trace_signal_info> signals;
        for (const auto& signal : result.signals) {
            assert(signal.name.find('[') == std::string::npos);
            signals[signal.name] = signal;
        }

        auto& mem = signals.at("ArrTop.mem");
        assert(mem.kind == "direct");
        assert(mem.root_name == "ArrTop.mem");
        assert(mem.type == "SData");
        assert(mem.width == 16);
        assert(mem.slot == mem_slot);
        assert(mem.deps.size() == 1);
        assert(mem.deps[0].found);
        assert(mem.deps[0].array_size == 4);

        auto& tiny = signals.at("ArrTop.tiny");
        assert(tiny.kind == "direct");
        assert(tiny.type == "CData");
        assert(tiny.width == 8);
        assert(tiny.slot == mem_slot + 4);
        assert(tiny.deps.size() == 1);
        assert(tiny.deps[0].found);
        assert(tiny.deps[0].array_size == 2);

        assert(signals.at("ArrTop.din").kind == "direct");
        assert(result.decl_count == 8);
        std::reverse(files.begin(), files.end());
    }
    std::cout << "dtype unpacked array declarations PASS\n";
}
