#pragma once
#include "picker.hpp"

namespace picker {
struct VariableInfo {
    std::string name;
    std::string type;
    int width      = 0;
    int array_size = 0;
    int64_t offset = 0;
};
namespace parser {
    std::vector<std::string> readVarDeclarations(const std::string &filename);
    std::vector<VariableInfo> processDeclarations(const std::vector<std::string> &declarations);
    void outputYAML(const std::vector<VariableInfo> &vars, const std::string &fileName = "");
} // namespace parser
} // namespace picker