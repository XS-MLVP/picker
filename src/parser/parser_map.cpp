
#include "parser/parser_map.hpp"


namespace picker{namespace parser {

    std::map<std::string, std::function<std::vector<std::string>(const std::string &)>> readVarDeclarations = {
        {"verilator", picker::parser::verilator::readVarDeclarations},
        {"gsim", picker::parser::gsim::readVarDeclarations}
    };

    std::map<std::string, std::function<std::vector<picker::cpp_variableInfo>(const std::vector<std::string> &)>> processDeclarations = {
        {"verilator", picker::parser::verilator::processDeclarations},
        {"gsim", picker::parser::gsim::processDeclarations}
    };

    std::map<std::string, std::function<int(picker::export_opts &opts, std::vector<picker::sv_module_define> &external_pin)>> inputParser = {
        {"vcs", picker::parser::sv},
        {"verilator", picker::parser::sv},
        {"gsim", picker::parser::firrtl}
    };

    std::function<std::vector<std::string>(const std::string &)> GetReadVarDeclarations(const std::string &sim) {
        if (readVarDeclarations.find(sim) != readVarDeclarations.end()) {
            return readVarDeclarations[sim];
        } else {
            throw std::runtime_error("Unsupported simulator: " + sim);
        }
    }

    std::function<std::vector<picker::cpp_variableInfo>(const std::vector<std::string> &)> GetProcessDeclarations(const std::string &sim) {
        if (processDeclarations.find(sim) != processDeclarations.end()) {
            return processDeclarations[sim];
        } else {
            throw std::runtime_error("Unsupported simulator: " + sim);
        }
    }

    std::function<int(picker::export_opts &, std::vector<picker::sv_module_define> &)> GetInputParser(const std::string &sim){
        if (inputParser.find(sim) != inputParser.end()) {
            return inputParser[sim];
        } else {
            throw std::runtime_error("Unsupported simulator: " + sim);
        }
    }

}} // namespace parser