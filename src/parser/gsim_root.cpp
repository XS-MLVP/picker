
#include "parser/gsim_root.hpp"

namespace picker { namespace parser { namespace gsim {
    std::vector<std::string> readVarDeclarations(const std::string &filename){
        std::vector<std::string> declarations;
        std::ifstream file(filename);
        std::string line;
        // eg:
        // uint32_t _var_start;
        // ....
        // uint32_t _var_end;
        bool find_var_start = false;
        while (getline(file, line)) {
            line = picker::trim(line);
            if (line.empty()) continue;
            if (find_var_start == false && line.find("uint32_t") == 0 && line.find("_var_start") != std::string::npos) {
                find_var_start = true;
                continue; // start of variable declarations
            }
            if (find_var_start) {
                if (line.find("uint32_t") == 0 && line.find("_var_end") != std::string::npos) {
                    find_var_start = false;
                    break; // end of variable declarations
                }
            }
            if (find_var_start)declarations.push_back(line);
        }
        return declarations;
    }
    std::vector<cpp_variableInfo> processDeclarations(const std::vector<std::string> &declarations){
        std::vector<cpp_variableInfo> vars;
        std::regex pattern(R"(^((?:unsigned\s+)?(?:uint\d+_t|_BitInt\(\d+\)))\s+([a-zA-Z_$][a-zA-Z0-9_$]*)(\[[\d\[\]]+\])?\s*;\s*//\s*width\s*=\s*(\d+)(?:,\s*lineno\s*=\s*\d+)?)");
        std::regex array_pattern(R"(\[(\d+)\])");
        for (const auto &declaration : declarations) {
            cpp_variableInfo var_info;
            std::istringstream iss(declaration);
            std::string type, name;
            std::smatch match;
            // Eg:
            // uint8_t soc$nutcore$backend$exu$mdu$div$state$NEXT; // width = 3, lineno = 6692
            // unsigned _BitInt(192) soc$nutcore$backend$exu$mdu$div$shiftReg$NEXT; // width = 129, lineno = 6697
            // uint64_t soc$nutcore$backend$exu$mdu$div$bReg$NEXT; // width = 64, lineno = 6719
            // uint8_t soc$nutcore$backend$exu$mdu$div$cnt_value$NEXT; // width = 6, lineno = 6726
            // uint64_t soc$nutcore$backend$isu$rf[32]; // width = 64, lineno = 5605
            // uint32_t soc$nutcore$io_imem_cache$metaArray$ram$array[128][4]; // width = 21, lineno = 12965
            if (std::regex_match(declaration, match, pattern)) {
                var_info.type = match[1];
                var_info.name = match[2];
                std::string array_part = match[3];
                var_info.width = std::stoi(match[4]);
                int array_size = 1;
                if (!array_part.empty()) {
                    std::sregex_iterator iter(array_part.begin(), array_part.end(), array_pattern);
                    std::sregex_iterator end;
                    for (; iter != end; ++iter) {
                        int dimension = std::stoi((*iter)[1]);
                        array_size *= dimension;
                    }
                }
                var_info.array_size = array_size;
            } else {
                PK_ERROR("Find Invalid declaration: %s", declaration.c_str());
                continue; // skip invalid declarations
            }
            vars.push_back(var_info);
        }
        return vars;
    }
}}}; // namespace picker::parser::gsim