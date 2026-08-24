#pragma once

#include "parser/verilator_trace.hpp"

namespace picker { namespace parser { namespace verilator { namespace detail {

enum class trace_event_kind {
    declaration,
    source,
};

struct trace_event {
    trace_event_kind kind = trace_event_kind::declaration;
    int slot = -1;
    std::string name;
    std::string type;
    int width = 0;
    std::string raw_expr;
    std::string expr;
    bool is_full = false;
};

std::vector<trace_event> parse_v5_026(const std::vector<std::string> &traceFiles);
std::vector<trace_event> parse_v5_048(const std::vector<std::string> &traceFiles);

trace_parse_result build_result(const std::vector<trace_event> &events,
                                const std::vector<cpp_variableInfo> &vars);

std::string normalize_verilator_name(const std::string &name);
std::string normalize_expr(const std::string &expr);
std::vector<std::string> split_statements(const std::string &content);
std::vector<std::string> split_top_level_args(const std::string &input);
bool unwrap_call_args(const std::string &statement, const std::string &marker,
                      std::vector<std::string> &args);
bool parse_slot_arg(const std::string &arg, int &slot);
std::string strip_quotes(const std::string &input);
std::string strip_wrapping_parentheses(std::string expr);
std::string join_scope(const std::vector<std::string> &scope, const std::string &leaf);
std::string signal_type_from_width(int width);
std::string read_file(const std::string &fileName);

} } } } // namespace picker::parser::verilator::detail
