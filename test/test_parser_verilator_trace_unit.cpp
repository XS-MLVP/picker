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
    const bool is_v5048 = fixture.filename() == "v5_048";
    const fs::path trace =
        fixture / (is_v5048 ? "VTop__Trace__0__Slow.cpp" : "VTop__Trace__0.cpp");
    std::vector<picker::cpp_variableInfo> vars = {
        {"Top__DOT__native", "CData", 8, 0, 0},
        {"Top__DOT__other", "CData", 8, 0, 1},
    };
    const auto result = picker::parser::verilator::processTraceFiles({trace.string()}, vars);

    if (!is_v5048) {
        assert(result.decl_count == 5 && result.value_count == 5 && result.matched_slot_count == 5);
        const auto *direct_arrow = find_signal(result.signals, "Top.direct_arrow");
        const auto *direct_ref = find_signal(result.signals, "Top.direct_ref");
        const auto *projection = find_signal(result.signals, "Top.projection");
        const auto *expression = find_signal(result.signals, "Top.expression");
        const auto *constant = find_signal(result.signals, "Top.constant");
        assert(direct_arrow && direct_ref && projection && expression && constant);
        assert(direct_arrow->kind == "direct" && direct_arrow->root_name == "Top.native" &&
               direct_arrow->source_expr == "Top.native" && direct_arrow->deps.size() == 1 &&
               direct_arrow->deps[0].found);
        assert(direct_ref->kind == "direct" && direct_ref->root_name == "Top.other" &&
               direct_ref->source_expr == "Top.other" && direct_ref->deps.size() == 1 &&
               direct_ref->deps[0].found);
        assert(projection->kind == "projection" && projection->root_name == "Top.native" &&
               projection->source_expr.find("vlSelf") == std::string::npos);
        assert(expression->kind == "expr" && expression->deps.size() == 2 &&
               expression->deps[0].found && expression->deps[1].found &&
               expression->source_expr.find("vlSelf") == std::string::npos);
        assert(constant->kind == "const" && constant->deps.empty() && constant->source_expr == "3U");
    } else {
        assert(result.decl_count == 11 && result.value_count == 6 && result.matched_slot_count == 6);
        assert(find_signal(result.signals, "direct_arrow"));
        assert(find_signal(result.signals, "Top.bit"));
        assert(find_signal(result.signals, "Top.expression"));
        assert(find_signal(result.signals, "Top.constant"));
    }
    return 0;
}
