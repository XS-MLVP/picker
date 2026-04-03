#include <fstream>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "parser/verilator_trace.hpp"

namespace picker { namespace parser {

namespace {

    struct trace_decl_info {
        std::string type;
        int width = 0;
    };

    struct trace_value_source {
        std::string raw_expr;
        std::string expr;
        bool is_full = false;
    };

    std::string normalize_verilator_name(const std::string &name)
    {
        std::string out;
        out.reserve(name.size());
        for (size_t i = 0; i < name.size(); ++i) {
            if (i + 6 < name.size() && name[i] == '_' && name[i + 1] == '_' && name[i + 2] == 'D'
                && name[i + 3] == 'O' && name[i + 4] == 'T' && name[i + 5] == '_' && name[i + 6] == '_') {
                out.push_back('.');
                i += 6;
            } else {
                out.push_back(name[i]);
            }
        }
        return out;
    }

    std::string yaml_escape(const std::string &input)
    {
        std::string out;
        out.reserve(input.size() + 8);
        for (char ch : input) {
            if (ch == '\\' || ch == '"') {
                out.push_back('\\');
            }
            out.push_back(ch);
        }
        return out;
    }

    std::ostream *open_output_stream(const std::string &fileName, std::ofstream &file)
    {
        std::ostream *out = &std::cout;
        if (!fileName.empty()) {
            auto absFPath = std::filesystem::absolute(fileName).string();
            file.open(absFPath);
            if (!file.is_open()) {
                PK_FATAL("Failed to open file: %s", absFPath.c_str());
            }
            out = &file;
        }
        return out;
    }

    std::string read_file(const std::string &fileName)
    {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            PK_FATAL("Failed to open trace file: %s", fileName.c_str());
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::vector<std::string> split_statements(const std::string &content)
    {
        std::vector<std::string> statements;
        std::string current;
        int paren_depth = 0;
        bool in_string  = false;
        bool escaping   = false;

        for (char ch : content) {
            current.push_back(ch);
            if (in_string) {
                if (escaping) {
                    escaping = false;
                } else if (ch == '\\') {
                    escaping = true;
                } else if (ch == '"') {
                    in_string = false;
                }
                continue;
            }

            if (ch == '"') {
                in_string = true;
            } else if (ch == '(') {
                ++paren_depth;
            } else if (ch == ')') {
                --paren_depth;
            } else if (ch == ';' && paren_depth == 0) {
                statements.push_back(current);
                current.clear();
            }
        }

        return statements;
    }

    std::vector<std::string> split_top_level_args(const std::string &input)
    {
        std::vector<std::string> args;
        std::string current;
        int paren_depth = 0;
        int brace_depth = 0;
        int bracket_depth = 0;
        bool in_string = false;
        bool escaping = false;

        for (char ch : input) {
            if (in_string) {
                current.push_back(ch);
                if (escaping) {
                    escaping = false;
                } else if (ch == '\\') {
                    escaping = true;
                } else if (ch == '"') {
                    in_string = false;
                }
                continue;
            }

            if (ch == '"') {
                in_string = true;
                current.push_back(ch);
                continue;
            }
            if (ch == '(') ++paren_depth;
            else if (ch == ')') --paren_depth;
            else if (ch == '{') ++brace_depth;
            else if (ch == '}') --brace_depth;
            else if (ch == '[') ++bracket_depth;
            else if (ch == ']') --bracket_depth;

            if (ch == ',' && paren_depth == 0 && brace_depth == 0 && bracket_depth == 0) {
                args.push_back(picker::trim(current));
                current.clear();
                continue;
            }

            current.push_back(ch);
        }

        auto tail = picker::trim(current);
        if (!tail.empty()) {
            args.push_back(tail);
        }
        return args;
    }

    bool unwrap_call_args(const std::string &statement, const std::string &marker, std::vector<std::string> &args)
    {
        auto pos = statement.find(marker);
        if (pos == std::string::npos) {
            return false;
        }
        auto open = statement.find('(', pos + marker.size());
        auto close = statement.rfind(')');
        if (open == std::string::npos || close == std::string::npos || close <= open) {
            return false;
        }
        args = split_top_level_args(statement.substr(open + 1, close - open - 1));
        return !args.empty();
    }

    bool parse_slot_arg(const std::string &arg, int &slot)
    {
        std::smatch match;
        if (!std::regex_match(arg, match, std::regex(R"(^(?:c|oldp)\+(\d+)$)"))) {
            return false;
        }
        slot = stoi(match[1]);
        return true;
    }

    std::string signal_type_from_width(int width)
    {
        if (width <= 1) {
            return "CData";
        }
        if (width <= 8) {
            return "CData";
        }
        if (width <= 16) {
            return "SData";
        }
        if (width <= 32) {
            return "IData";
        }
        if (width <= 64) {
            return "QData";
        }
        return "VlWide<" + std::to_string((width + 31) / 32) + ">";
    }

    bool parse_decl_info(const std::string &statement, const std::vector<std::string> &args, trace_decl_info &info)
    {
        std::smatch match;
        if (!std::regex_search(statement, match, std::regex(R"(tracep->decl([A-Za-z]+))"))) {
            return false;
        }
        auto kind = match[1].str();
        if (kind == "Bit") {
            info.width = 1;
            info.type = "CData";
            return true;
        }
        if ((kind == "Bus" || kind == "Quad") && args.size() >= 2) {
            int high = std::stoi(args[args.size() - 2]);
            int low = std::stoi(args[args.size() - 1]);
            info.width = high - low + 1;
            info.type = signal_type_from_width(info.width);
            return true;
        }
        return false;
    }

    std::string strip_quotes(const std::string &input)
    {
        if (input.size() >= 2 && input.front() == '"' && input.back() == '"') {
            return input.substr(1, input.size() - 2);
        }
        return input;
    }

    std::string join_scope(const std::vector<std::string> &scope, const std::string &leaf)
    {
        std::string result;
        for (const auto &part : scope) {
            if (!result.empty()) {
                result += ".";
            }
            result += part;
        }
        if (!leaf.empty()) {
            if (!result.empty()) {
                result += ".";
            }
            result += leaf;
        }
        return result;
    }

    std::string strip_wrapping_parentheses(std::string expr)
    {
        expr = picker::trim(expr);
        while (expr.size() >= 2 && expr.front() == '(' && expr.back() == ')') {
            int depth = 0;
            bool wraps = true;
            for (size_t i = 0; i < expr.size(); ++i) {
                char ch = expr[i];
                if (ch == '(') ++depth;
                else if (ch == ')') --depth;
                if (depth == 0 && i + 1 < expr.size()) {
                    wraps = false;
                    break;
                }
            }
            if (!wraps) {
                break;
            }
            expr = picker::trim(expr.substr(1, expr.size() - 2));
        }
        return expr;
    }

    std::vector<std::string> extract_member_refs(const std::string &expr)
    {
        std::vector<std::string> refs;
        std::unordered_set<std::string> seen;
        std::regex ref_regex(R"(vlSelfRef\.([A-Za-z_][A-Za-z0-9_]*(?:__DOT__[A-Za-z_][A-Za-z0-9_]*)*))");
        auto begin = std::sregex_iterator(expr.begin(), expr.end(), ref_regex);
        auto end = std::sregex_iterator();
        for (auto it = begin; it != end; ++it) {
            auto raw = (*it)[1].str();
            auto dotted = normalize_verilator_name(raw);
            if (seen.insert(dotted).second) {
                refs.push_back(dotted);
            }
        }
        return refs;
    }

    std::string replace_member_refs_with_dots(const std::string &expr)
    {
        std::string out;
        std::regex ref_regex(R"(vlSelfRef\.([A-Za-z_][A-Za-z0-9_]*(?:__DOT__[A-Za-z_][A-Za-z0-9_]*)*))");
        auto begin = std::sregex_iterator(expr.begin(), expr.end(), ref_regex);
        auto end = std::sregex_iterator();
        size_t last = 0;
        for (auto it = begin; it != end; ++it) {
            const auto &match = *it;
            out.append(expr.substr(last, match.position() - last));
            out.append(normalize_verilator_name(match[1].str()));
            last = match.position() + match.length();
        }
        out.append(expr.substr(last));
        return out;
    }

    std::string normalize_expr(const std::string &expr)
    {
        auto normalized = replace_member_refs_with_dots(expr);
        std::string compact;
        compact.reserve(normalized.size());
        bool lastWasSpace = false;
        for (unsigned char ch : normalized) {
            if (std::isspace(ch)) {
                if (!lastWasSpace) {
                    compact.push_back(' ');
                    lastWasSpace = true;
                }
                continue;
            }
            compact.push_back(static_cast<char>(ch));
            lastWasSpace = false;
        }
        return picker::trim(compact);
    }

    bool is_constant_expr(const std::string &expr)
    {
        auto compact = expr;
        compact.erase(std::remove_if(compact.begin(), compact.end(), [](unsigned char ch) { return std::isspace(ch); }),
                      compact.end());
        if (compact.empty()) {
            return false;
        }
        return std::regex_match(compact, std::regex(R"([0-9A-Fa-fxXuUlL+\-<>&|~()?:]+)"));
    }

    bool is_direct_expr(const std::string &expr, const std::string &dep)
    {
        auto compact = replace_member_refs_with_dots(expr);
        compact.erase(std::remove_if(compact.begin(), compact.end(), [](unsigned char ch) { return std::isspace(ch); }),
                      compact.end());
        compact = std::regex_replace(compact, std::regex(R"(\([A-Za-z_][A-Za-z0-9_:<>]*\))"), "");
        compact.erase(std::remove(compact.begin(), compact.end(), '('), compact.end());
        compact.erase(std::remove(compact.begin(), compact.end(), ')'), compact.end());
        return compact == dep;
    }

    std::string classify_expr(const std::string &expr, const std::vector<std::string> &deps)
    {
        if (deps.empty()) {
            return is_constant_expr(expr) ? "const" : "expr";
        }
        if (deps.size() == 1) {
            return is_direct_expr(expr, deps.front()) ? "direct" : "projection";
        }
        return "expr";
    }

    std::unordered_map<std::string, cpp_variableInfo> build_var_lookup(const std::vector<cpp_variableInfo> &vars)
    {
        std::unordered_map<std::string, cpp_variableInfo> lookup;
        for (const auto &var : vars) {
            auto normalized = normalize_verilator_name(var.name);
            lookup[normalized] = var;
        }
        return lookup;
    }

} // namespace

void outputSignalYAML(const std::vector<trace_signal_info> &signals, const std::string &fileName)
{
    std::ofstream file;
    std::ostream *out = open_output_stream(fileName, file);

    *out << "signals:\n";
    for (const auto &signal : signals) {
        *out << "  - name: \"" << yaml_escape(signal.name) << "\"\n";
        *out << "    kind: " << signal.kind << "\n";

        if (signal.kind == "direct") {
            *out << "    source: \"" << yaml_escape(signal.root_name) << "\"\n";
            if (!signal.type.empty()) {
                *out << "    type: \"" << yaml_escape(signal.type) << "\"\n";
            }
            if (signal.width > 0) {
                *out << "    rtl_width: " << signal.width << "\n";
            }
            continue;
        }

        if (signal.kind == "projection") {
            if (!signal.root_name.empty()) {
                *out << "    source: \"" << yaml_escape(signal.root_name) << "\"\n";
            }
            *out << "    expr: \"" << yaml_escape(signal.source_expr) << "\"\n";
            if (!signal.type.empty()) {
                *out << "    type: \"" << yaml_escape(signal.type) << "\"\n";
            }
            if (signal.width > 0) {
                *out << "    rtl_width: " << signal.width << "\n";
            }
            continue;
        }

        if (signal.kind == "expr" && !signal.deps.empty()) {
            *out << "    deps:\n";
            for (const auto &dep : signal.deps) {
                *out << "      - \"" << yaml_escape(dep.name) << "\"\n";
            }
        }
        if (signal.kind == "expr") {
            if (!signal.type.empty()) {
                *out << "    type: \"" << yaml_escape(signal.type) << "\"\n";
            }
            if (signal.width > 0) {
                *out << "    rtl_width: " << signal.width << "\n";
            }
            *out << "    expr: \"" << yaml_escape(signal.source_expr) << "\"\n";
            continue;
        }

        if (!signal.type.empty()) {
            *out << "    type: \"" << yaml_escape(signal.type) << "\"\n";
        }
        if (signal.width > 0) {
            *out << "    rtl_width: " << signal.width << "\n";
        }
        *out << "    value: \"" << yaml_escape(signal.source_expr) << "\"\n";
    }

    out->flush();
    if (file.is_open()) {
        file.close();
    }
}

namespace verilator {

    std::vector<std::string> findTraceFiles(const std::string &rootHeaderFile)
    {
        std::vector<std::string> traceFiles;
        std::filesystem::path rootPath(rootHeaderFile);
        if (!std::filesystem::exists(rootPath)) {
            return traceFiles;
        }

        auto fileName = rootPath.filename().string();
        std::smatch match;
        if (!std::regex_match(fileName, match, std::regex(R"(^V(.+)___024root\.h$)"))) {
            return traceFiles;
        }
        auto topName = match[1].str();
        auto parent = rootPath.parent_path();
        std::regex traceRegex("^V" + topName + R"(__Trace__.*\.cpp$)");

        for (const auto &entry : std::filesystem::directory_iterator(parent)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            auto current = entry.path().filename().string();
            if (std::regex_match(current, traceRegex)) {
                traceFiles.push_back(entry.path().string());
            }
        }

        std::sort(traceFiles.begin(), traceFiles.end());
        return traceFiles;
    }

    trace_parse_result processTraceFiles(const std::vector<std::string> &traceFiles,
                                         const std::vector<cpp_variableInfo> &vars)
    {
        trace_parse_result result;
        std::unordered_map<int, std::set<std::string>> slotToNames;
        std::unordered_map<int, trace_decl_info> slotToDecl;
        std::unordered_map<int, trace_value_source> slotToSource;
        auto varLookup = build_var_lookup(vars);

        for (const auto &traceFile : traceFiles) {
            auto statements = split_statements(read_file(traceFile));
            std::vector<std::string> scope;
            int oldpBaseShift = 0;

            for (auto statement : statements) {
                statement = picker::trim(statement);
                if (statement.empty()) {
                    continue;
                }

                if (statement.find("bufp->oldp(vlSymsp->__Vm_baseCode + 1)") != std::string::npos) {
                    oldpBaseShift = 1;
                    continue;
                }
                if (statement.find("bufp->oldp(vlSymsp->__Vm_baseCode)") != std::string::npos) {
                    oldpBaseShift = 0;
                    continue;
                }

                std::vector<std::string> args;
                if (unwrap_call_args(statement, "tracep->pushPrefix", args)) {
                    if (!args.empty()) {
                        scope.push_back(strip_quotes(args[0]));
                    }
                    continue;
                }
                if (statement.find("tracep->popPrefix") != std::string::npos) {
                    if (!scope.empty()) {
                        scope.pop_back();
                    }
                    continue;
                }
                if (statement.find("tracep->decl") != std::string::npos) {
                    if (!unwrap_call_args(statement, "tracep->decl", args) || args.size() < 3) {
                        continue;
                    }
                    int slot = -1;
                    if (!parse_slot_arg(args[0], slot)) {
                        continue;
                    }
                    auto fullName = join_scope(scope, strip_quotes(args[2]));
                    if (!fullName.empty()) {
                        if (slotToNames[slot].insert(fullName).second) {
                            ++result.decl_count;
                        }
                    }
                    trace_decl_info declInfo;
                    if (parse_decl_info(statement, args, declInfo) && slotToDecl.find(slot) == slotToDecl.end()) {
                        slotToDecl[slot] = declInfo;
                    }
                    continue;
                }
                if (statement.find("bufp->full") != std::string::npos || statement.find("bufp->chg") != std::string::npos) {
                    auto marker = statement.find("bufp->full") != std::string::npos ? "bufp->full" : "bufp->chg";
                    if (!unwrap_call_args(statement, marker, args) || args.size() < 2) {
                        continue;
                    }
                    int slot = -1;
                    if (!parse_slot_arg(args[0], slot)) {
                        continue;
                    }

                    auto isFull = marker == std::string("bufp->full");
                    slot += oldpBaseShift;
                    if (slotToSource.find(slot) != slotToSource.end() && slotToSource[slot].is_full && !isFull) {
                        continue;
                    }
                    auto rawExpr = strip_wrapping_parentheses(args[1]);
                    if (slotToSource.find(slot) == slotToSource.end()) {
                        ++result.value_count;
                    }
                    slotToSource[slot] = {rawExpr, normalize_expr(rawExpr), isFull};
                }
            }
        }

        std::vector<trace_signal_info> signals;
        for (const auto &[slot, names] : slotToNames) {
            auto sourceIt = slotToSource.find(slot);
            if (sourceIt == slotToSource.end()) {
                continue;
            }
            ++result.matched_slot_count;
            auto deps = extract_member_refs(sourceIt->second.raw_expr);
            auto kind = classify_expr(sourceIt->second.raw_expr, deps);
            trace_decl_info declInfo;
            if (slotToDecl.find(slot) != slotToDecl.end()) {
                declInfo = slotToDecl[slot];
            }

            std::vector<trace_source_binding> bindings;
            for (const auto &dep : deps) {
                trace_source_binding binding;
                binding.name = dep;
                auto varIt = varLookup.find(dep);
                if (varIt != varLookup.end()) {
                    binding.found = true;
                    binding.type = varIt->second.type;
                    binding.width = varIt->second.width;
                    binding.array_size = varIt->second.array_size;
                }
                bindings.push_back(binding);
            }

            for (const auto &name : names) {
                trace_signal_info info;
                info.name = name;
                info.slot = slot;
                info.kind = kind;
                info.type = declInfo.type;
                info.width = declInfo.width;
                info.source_expr = sourceIt->second.expr;
                info.deps = bindings;
                if ((kind == "direct" || kind == "projection") && deps.size() == 1) {
                    info.root_name = deps.front();
                }
                signals.push_back(info);
            }
        }

        std::sort(signals.begin(), signals.end(), [](const trace_signal_info &a, const trace_signal_info &b) {
            if (a.name == b.name) {
                return a.slot < b.slot;
            }
            return a.name < b.name;
        });
        result.signals = std::move(signals);
        return result;
    }

} // namespace verilator

}} // namespace picker::parser
