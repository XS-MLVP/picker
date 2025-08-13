#pragma once
#include "picker.hpp"
#include "parser/verilator_root.hpp"

namespace picker {
namespace parser {
namespace gsim {
    std::vector<std::string> readVarDeclarations(const std::string &filename);
    std::vector<cpp_variableInfo> processDeclarations(const std::vector<std::string> &declarations);
} // namespace gsim
} // namespace parser
} // namespace picker