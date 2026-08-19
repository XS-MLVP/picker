#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "parser/verilator_trace.hpp"

static void write_text(const std::filesystem::path &path, const std::string &text)
{
    std::ofstream out(path, std::ios::trunc);
    out << text;
}

static const picker::parser::trace_signal_info *find_signal(
    const std::vector<picker::parser::trace_signal_info> &signals, const std::string &name)
{
    for (const auto &signal : signals) {
        if (signal.name == name) return &signal;
    }
    return nullptr;
}

int main()
{
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() / "picker_test_verilator_trace";
    fs::remove_all(base);
    fs::create_directories(base);

    const fs::path trace = base / "VTop__Trace__0.cpp";
    write_text(trace,
               "tracep->pushPrefix(\"Top\", 0);\n"
               "tracep->declBus(c+1, 0, \"direct_arrow\", -1, 7, 0);\n"
               "tracep->declBus(c+2, 0, \"direct_ref\", -1, 7, 0);\n"
               "tracep->declBus(c+3, 0, \"projection\", -1, 6, 0);\n"
               "tracep->declBus(c+4, 0, \"expression\", -1, 7, 0);\n"
               "tracep->declBus(c+5, 0, \"constant\", -1, 7, 0);\n"
               "tracep->popPrefix();\n"
               "bufp->fullCData(oldp+1, vlSelf->Top__DOT__native, 8);\n"
               "bufp->fullCData(oldp+2, vlSelfRef.Top__DOT__other, 8);\n"
               "bufp->fullCData(oldp+3, (IData)(vlSelf->Top__DOT__native) >> 1U, 7);\n"
               "bufp->fullCData(oldp+4, vlSelf->Top__DOT__native | vlSelfRef.Top__DOT__other, 8);\n"
               "bufp->fullCData(oldp+5, 3U, 8);\n");

    std::vector<picker::cpp_variableInfo> vars = {
        {"Top__DOT__native", "CData", 8, 0, 0},
        {"Top__DOT__other", "CData", 8, 0, 1},
    };
    auto result = picker::parser::verilator::processTraceFiles({trace.string()}, vars);

    assert(result.decl_count == 5);
    assert(result.value_count == 5);
    assert(result.matched_slot_count == 5);

    const auto *direct_arrow = find_signal(result.signals, "Top.direct_arrow");
    const auto *direct_ref = find_signal(result.signals, "Top.direct_ref");
    const auto *projection = find_signal(result.signals, "Top.projection");
    const auto *expression = find_signal(result.signals, "Top.expression");
    const auto *constant = find_signal(result.signals, "Top.constant");
    assert(direct_arrow && direct_ref && projection && expression && constant);

    assert(direct_arrow->kind == "direct");
    assert(direct_arrow->root_name == "Top.native");
    assert(direct_arrow->source_expr == "Top.native");
    assert(direct_arrow->deps.size() == 1 && direct_arrow->deps[0].found);

    assert(direct_ref->kind == "direct");
    assert(direct_ref->root_name == "Top.other");
    assert(direct_ref->source_expr == "Top.other");
    assert(direct_ref->deps.size() == 1 && direct_ref->deps[0].found);

    assert(projection->kind == "projection");
    assert(projection->root_name == "Top.native");
    assert(projection->source_expr.find("vlSelf") == std::string::npos);

    assert(expression->kind == "expr");
    assert(expression->deps.size() == 2);
    assert(expression->deps[0].found && expression->deps[1].found);
    assert(expression->source_expr.find("vlSelf") == std::string::npos);

    assert(constant->kind == "const");
    assert(constant->deps.empty());
    assert(constant->source_expr == "3U");

    fs::remove_all(base);
    return 0;
}
