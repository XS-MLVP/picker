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
    bool is_array = false;
    int array_size = 0;
};

// Loaded once per export. Only trace-relevant statements are retained, and
// helper bodies are indexed once regardless of the number of call sites.
struct trace_input {
    std::map<std::string, std::vector<std::string>> functions;
    std::vector<std::string> loose;
    bool modern = false;
    bool legacy = false;
};

trace_input read_trace_input(const std::vector<std::string> &traceFiles);
std::vector<trace_event> parse_trace_events(const trace_input &input, bool modern);

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
// The only version-specific operation is decoding declaration arguments.
bool parse_v5_026_declaration(const std::string &statement, const std::vector<std::string> &args,
                             int slot, trace_event &event);
bool parse_v5_048_declaration(const std::string &statement, const std::vector<std::string> &args,
                             int slot, trace_event &event);

} } } } // namespace picker::parser::verilator::detail
