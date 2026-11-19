#include "common.hpp"

#include <fstream>
#include <limits>
#include <map>

namespace picker { namespace parser { namespace verilator { namespace detail {

std::string strip_type_attributes(const std::string &type)
{
    static const std::regex alignment(R"(^alignas\s*\([^)]*\)\s*)");
    return picker::trim(std::regex_replace(picker::trim(type), alignment, ""));
}

const root_grammar &grammar_for(root_format format)
{
    static const root_grammar v5_026{"5.026", v5_026_state_section,
                                     v5_026_normalize_type, false};
    static const root_grammar v5_054{"5.054", v5_054_state_section,
                                     v5_054_normalize_type, true};
    return format == root_format::v5_054 ? v5_054 : v5_026;
}

root_format detect_root_format(const std::string &filename)
{
    // 5.054 is the only grammar that renames the state section and qualifies
    // storage types, so the header itself identifies it. Everything older uses
    // the same grammar, whatever the generator version was.
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("TRIVIALLY CONSTRUCTIBLE STATE") != std::string::npos) {
            return root_format::v5_054;
        }
        if (picker::trim(line).rfind("::", 0) == 0) return root_format::v5_054;
    }
    return root_format::v5_026;
}

std::string normalize_root_type(const std::string &type, root_format format)
{
    return grammar_for(format).normalize_type(type);
}

std::vector<std::string> read_root_declarations(const std::string &filename, root_format format)
{
    const auto &grammar = grammar_for(format);
    std::vector<std::string> declarations;
    std::ifstream file(filename);
    std::string line;
    // XXX.h -> XXX
    std::string class_name = filename.substr(0, filename.find_last_of('.')) + "(";
    int structDepth = 0, no_struct = -1;

    // 1. We are scanning for variable declarations inside struct blocks.
    // Increase structDepth on "struct {" and decrease it on "};".
    // There might be multiple struct blocks or nested struct blocks.
    // 2. Otherwise, there is no struct block and we are scanning for Data
    // declarations. Start with a state section marker and end with
    // class_name::class_name().
    while (getline(file, line)) {
        line = picker::trim(line);
        if (line.empty()) continue;
        if (grammar.is_state_section(line) && no_struct == -1) {
            no_struct = 1;
            continue;
        }

        // The 5.054 `TOP LEVEL PORTS` prelude nests a struct inside a union;
        // honoring it would swallow the state section that follows.
        if (line.find("struct {") != std::string::npos
            && (!grammar.skip_prelude || no_struct != -1)) {
            structDepth++;
            no_struct = 0;
            continue;
        } // 1

        // Headers without a state marker fall back to the first storage
        // declaration. 5.054 members carry a `::` qualifier and a `/*..*/`
        // width comment; the port prelude macros carry neither.
        const bool global_member = line.rfind("::", 0) == 0
                                   && line.find("/*") != std::string::npos;
        if (no_struct == -1
            && (global_member || (!grammar.skip_prelude
                                  && line.find("Data") != std::string::npos))) {
            no_struct = 1;
        } // 2

        if (structDepth > 0 && line.find("};") != std::string::npos) {
            structDepth--;
            continue;
        } // 1

        if (line.find(class_name) != std::string::npos) {
            no_struct = -1;
            break;
        } // 2

        if (structDepth > 0 || no_struct > 0) {
            size_t commentPos = line.find("//");
            if (commentPos != std::string::npos) { line = line.substr(0, commentPos); }
            line = picker::trim(line);
            if (!line.empty()) { declarations.push_back(line); }
        }
    }
    return declarations;
}

cpp_variableInfo parse_root_type(const std::string &typeStr, root_format format)
{
    cpp_variableInfo info;
    std::smatch match;
    const auto normalizedType = normalize_root_type(typeStr, format);

    // Match VlUnpacked type
    if (std::regex_match(normalizedType, match, std::regex(R"(^VlUnpacked<(.+),\s*(\d+)>$)"))) {
        std::string innerType = picker::trim(match[1]);
        info.array_size       = stoi(match[2]);
        cpp_variableInfo inner    = parse_root_type(innerType, format);
        info.type             = inner.type;
        info.width            = inner.width;
        info.array_dimensions = {info.array_size};
        info.array_dimensions.insert(info.array_dimensions.end(), inner.array_dimensions.begin(),
                                     inner.array_dimensions.end());
        if (inner.array_size > 0) {
            if (info.array_size > std::numeric_limits<int>::max() / inner.array_size) {
                PK_FATAL("Verilator array is too large: %s", typeStr.c_str());
            }
            info.array_size *= inner.array_size;
        }
    }
    // Match VlWide type
    else if (std::regex_match(normalizedType, match,
                              std::regex(R"(^VlWide<(\d+)>/\*(\d+:\d+)\*/$)"))) {
        info.type         = "VlWide<" + match[1].str() + ">";
        std::string range = match[2];
        size_t colon      = range.find(':');
        int high          = stoi(range.substr(0, colon));
        int low           = stoi(range.substr(colon + 1));
        info.width        = high - low + 1;
    }
    // Match Basic type (only CData/SData/QData/IData/WData)
    else if (std::regex_match(normalizedType, match,
                              std::regex(R"(^(CData|SData|QData|IData|WData)/\*(\d+:\d+)\*/$)"))) {
        info.type         = match[1];
        std::string range = match[2];
        size_t colon      = range.find(':');
        int high          = stoi(range.substr(0, colon));
        int low           = stoi(range.substr(colon + 1));
        info.width        = high - low + 1;
    }

    if (info.array_size > 0) {
        PK_DEBUG("type: %s, width: %d, array_size: %d", info.type.c_str(), info.width,
                 info.array_size);
    } else {
        PK_DEBUG("type: %s, width: %d", info.type.c_str(), info.width);
    }

    return info;
}

std::vector<cpp_variableInfo> parse_root_declarations(
    const std::vector<std::string> &declarations, root_format format)
{
    std::vector<cpp_variableInfo> vars;
    std::regex varRegex(R"(^(.+[^\s])\s+([a-zA-Z_]\w*);$)");
    // Large models contain hundreds of thousands of members but far fewer
    // distinct storage types. Parse each type once, never each array value.
    std::map<std::string, cpp_variableInfo> type_cache;

    for (const auto &decl : declarations) {
        std::smatch match;
        if (std::regex_match(decl, match, varRegex)) {
            std::string typeStr = picker::trim(match[1]);
            std::string varName = picker::trim(match[2]);

            PK_DEBUG("typeStr: %s, varName: %s", typeStr.c_str(), varName.c_str());
            auto [cached, inserted] = type_cache.try_emplace(typeStr);
            if (inserted) cached->second = parse_root_type(typeStr, format);
            cpp_variableInfo info = cached->second;
            info.name             = varName;
            if (!info.type.empty()) vars.push_back(info);
        }
    }
    return vars;
}

} } } } // namespace picker::parser::verilator::detail

namespace picker { namespace parser {

    void outputYAML(const std::vector<cpp_variableInfo> &vars, const std::string &fileName)
    {
        std::ostream *out = &std::cout;
        std::ofstream file;
        if (!fileName.empty()) {
            std::string absFPath = std::filesystem::absolute(fileName);
            file.open(absFPath);
            if (!file.is_open()) {
                PK_FATAL("Failed to open file: %s", absFPath.c_str());
                return;
            }
            out = &file;
        }
        *out << "variables:\n";
        for (const auto &var : vars) {
            *out << "  - name: " << var.name << "\n"
                 << "    type: " << var.type << "\n"
                 << "    width: " << var.width << "\n";
            if (var.array_size > 0) { *out << "    array_size: " << var.array_size << "\n"; }
            if (!var.layout.empty()) *out << "    layout_id: " << var.layout << "\n";
            if (!var.public_name.empty()) *out << "    public_name: " << var.public_name << "\n";
            if (!var.view_error.empty()) *out << "    view_error: \"Native RTL/storage layout mismatch\"\n";
        }
        *out << "  - total: " << vars.size() << "\n";
        out->flush();
        if (file.is_open()) { file.close(); }
    }

}} // namespace picker::parser
