#pragma once
#include <map>
#include <string>
#include <functional>

#include "parser/gsim_root.hpp"
#include "parser/verilator_root.hpp"

namespace picker{namespace parser {

    std::function<std::vector<std::string>(const std::string &)> GetReadVarDeclarations(const std::string &sim);
    std::function<std::vector<picker::cpp_variableInfo>(const std::vector<std::string> &)> GetProcessDeclarations(const std::string &sim);
    std::function<int(picker::export_opts &, std::vector<picker::sv_module_define> &)> GetInputParser(const std::string &sim);

}} // namespace parser
