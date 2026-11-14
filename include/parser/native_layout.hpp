#pragma once
#include "picker.hpp"
#include "parser/verilator_trace.hpp"

namespace picker::parser {
// Join elaborated semantic types to retained C++ storage. The returned table
// contains only types reachable from verified bindings, never the entire AST.
nlohmann::json bind_native_layouts(const std::string &root_header,
                                  const nlohmann::json &metadata,
                                  std::vector<cpp_variableInfo> &variables,
                                  const std::vector<trace_signal_info> &aliases = {});
}
