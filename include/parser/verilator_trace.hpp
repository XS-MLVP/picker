#pragma once

#include "picker.hpp"

namespace picker {
namespace parser {

typedef struct trace_source_binding {
    std::string name;
    std::string type;
    int width      = 0;
    int array_size = 0;
    bool found     = false;
} trace_source_binding;

typedef struct trace_signal_info {
    std::string name;
    int slot = -1;
    std::string kind;
    std::string type;
    int width = 0;
    std::string source_expr;
    std::string root_name;
    std::vector<trace_source_binding> deps;
} trace_signal_info;

typedef struct trace_parse_result {
    std::vector<trace_signal_info> signals;
    size_t decl_count = 0;
    size_t value_count = 0;
    size_t matched_slot_count = 0;
} trace_parse_result;

void outputSignalYAML(const std::vector<trace_signal_info> &signals, const std::string &fileName = "");

namespace verilator {
    std::vector<std::string> findTraceFiles(const std::string &rootHeaderFile);
    trace_parse_result processTraceFiles(const std::vector<std::string> &traceFiles,
                                         const std::vector<cpp_variableInfo> &vars);
} // namespace verilator

} // namespace parser
} // namespace picker
