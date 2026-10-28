#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include "parser/verilator_trace.hpp"

int main(int argc, char** argv) {
    assert(argc == 2);
    namespace fs = std::filesystem;
    fs::path fixture(argv[1]);
    std::vector<std::string> files = {(fixture / "VTop__Trace__0__Slow.cpp").string(),
        (fixture / "VTop__Trace__1__Slow.cpp").string()};
    std::vector<picker::cpp_variableInfo> vars = {
        {"Top__DOT__ready", "CData", 1, 0, 0}, {"Top__DOT__valid", "CData", 1, 0, 1},
        {"Top__DOT__first", "SData", 16, 16, 2}, {"Top__DOT__second", "SData", 16, 16, 34}};
    for (int order = 0; order < 2; ++order) {
        auto result = picker::parser::verilator::processTraceFiles(files, vars);
        assert(result.signals.size() == 5);
        std::map<std::string, picker::parser::trace_signal_info> signals;
        for (const auto& signal : result.signals) {
            assert(signal.source_expr.find("__VdtypeVar") == std::string::npos);
            signals[signal.name] = signal;
        }
        assert(signals.at("Top.train_ready").source_expr == "Top.ready");
        assert(signals.at("Top.train_valid").source_expr == "Top.valid");
        assert(signals.at("Top.train_valid").kind == "direct");
        assert(signals.at("Top.first_zero").source_expr == "Top.first[0]");
        assert(signals.at("Top.first_nine").source_expr == "Top.first[9]");
        assert(signals.at("Top.second_zero").source_expr == "Top.second[0]");
        std::reverse(files.begin(), files.end());
    }
    std::cout << "dtype scope, cross-file calls, constant offsets and scalar non-clobber PASS\n";
}
