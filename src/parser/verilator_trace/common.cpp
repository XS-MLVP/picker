#include <fstream>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "common.hpp"

namespace picker { namespace parser { namespace verilator { namespace detail {

namespace {

std::vector<std::string> extract_member_refs(const std::string &expr)
{
    std::vector<std::string> refs;
    std::unordered_set<std::string> seen;
    std::regex ref_regex(
        R"((?:vlSelfRef\.|vlSelf->)([A-Za-z_][A-Za-z0-9_]*(?:__DOT__[A-Za-z_][A-Za-z0-9_]*)*))");
    auto begin = std::sregex_iterator(expr.begin(), expr.end(), ref_regex);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        auto dotted = normalize_verilator_name((*it)[1].str());
        if (seen.insert(dotted).second) refs.push_back(dotted);
    }
    return refs;
}

std::string replace_member_refs_with_dots(const std::string &expr)
{
    std::string out;
    std::regex ref_regex(
        R"((?:vlSelfRef\.|vlSelf->)([A-Za-z_][A-Za-z0-9_]*(?:__DOT__[A-Za-z_][A-Za-z0-9_]*)*))");
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

bool is_constant_expr(const std::string &expr)
{
    auto compact = expr;
    compact.erase(std::remove_if(compact.begin(), compact.end(),
                                 [](unsigned char ch) { return std::isspace(ch); }),
                  compact.end());
    return !compact.empty()
        && std::regex_match(compact, std::regex(R"([0-9A-Fa-fxXuUlL+\-<>&|~()?:]+)"));
}

bool is_direct_expr(const std::string &expr, const std::string &dep)
{
    auto compact = replace_member_refs_with_dots(expr);
    compact.erase(std::remove_if(compact.begin(), compact.end(),
                                 [](unsigned char ch) { return std::isspace(ch); }),
                  compact.end());
    compact = std::regex_replace(compact, std::regex(R"(\([A-Za-z_][A-Za-z0-9_:<>]*\))"), "");
    compact.erase(std::remove(compact.begin(), compact.end(), '('), compact.end());
    compact.erase(std::remove(compact.begin(), compact.end(), ')'), compact.end());
    return compact == dep;
}

std::string classify_expr(const std::string &expr, const std::vector<std::string> &deps)
{
    if (deps.empty()) return is_constant_expr(expr) ? "const" : "expr";
    if (deps.size() == 1) return is_direct_expr(expr, deps.front()) ? "direct" : "projection";
    return "expr";
}

std::unordered_map<std::string, cpp_variableInfo>
build_var_lookup(const std::vector<cpp_variableInfo> &vars)
{
    std::unordered_map<std::string, cpp_variableInfo> lookup;
    for (const auto &var : vars) lookup[normalize_verilator_name(var.name)] = var;
    return lookup;
}

} // namespace

std::string read_file(const std::string &fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open()) PK_FATAL("Failed to open trace file: %s", fileName.c_str());
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string normalize_verilator_name(const std::string &name)
{
    std::string out;
    out.reserve(name.size());
    for (size_t i = 0; i < name.size(); ++i) {
        if (i + 6 < name.size() && name.compare(i, 7, "__DOT__") == 0) {
            out.push_back('.');
            i += 6;
        } else {
            out.push_back(name[i]);
        }
    }
    return out;
}

std::vector<std::string> split_statements(const std::string &content)
{
    std::vector<std::string> statements;
    std::string current;
    int paren_depth = 0;
    bool in_string = false;
    bool escaping = false;
    for (char ch : content) {
        current.push_back(ch);
        if (in_string) {
            if (escaping) escaping = false;
            else if (ch == '\\') escaping = true;
            else if (ch == '"') in_string = false;
            continue;
        }
        if (ch == '"') in_string = true;
        else if (ch == '(') ++paren_depth;
        else if (ch == ')') --paren_depth;
        else if (ch == ';' && paren_depth == 0) {
            statements.push_back(current);
            current.clear();
        }
    }
    if (!picker::trim(current).empty()) statements.push_back(current);
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
            if (escaping) escaping = false;
            else if (ch == '\\') escaping = true;
            else if (ch == '"') in_string = false;
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
        } else {
            current.push_back(ch);
        }
    }
    auto tail = picker::trim(current);
    if (!tail.empty()) args.push_back(tail);
    return args;
}

bool unwrap_call_args(const std::string &statement, const std::string &marker,
                      std::vector<std::string> &args)
{
    auto pos = statement.find(marker);
    if (pos == std::string::npos) return false;
    auto open = statement.find('(', pos + marker.size());
    auto close = statement.rfind(')');
    if (open == std::string::npos || close == std::string::npos || close <= open) return false;
    args = split_top_level_args(statement.substr(open + 1, close - open - 1));
    return !args.empty();
}

bool parse_slot_arg(const std::string &arg, int &slot)
{
    std::smatch match;
    auto value = picker::trim(arg);
    if (!std::regex_match(value, match, std::regex(R"(^(?:c|oldp)\s*\+\s*(\d+)$)"))) return false;
    slot = std::stoi(match[1]);
    return true;
}

std::string signal_type_from_width(int width)
{
    if (width <= 8) return "CData";
    if (width <= 16) return "SData";
    if (width <= 32) return "IData";
    if (width <= 64) return "QData";
    return "VlWide<" + std::to_string((width + 31) / 32) + ">";
}

std::string strip_quotes(const std::string &input)
{
    auto value = picker::trim(input);
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

std::string join_scope(const std::vector<std::string> &scope, const std::string &leaf)
{
    std::string result;
    for (const auto &part : scope) {
        if (part == "$rootio") continue;
        if (!result.empty()) result += ".";
        result += part;
    }
    if (!leaf.empty()) {
        if (!result.empty()) result += ".";
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
            if (expr[i] == '(') ++depth;
            else if (expr[i] == ')') --depth;
            if (depth == 0 && i + 1 < expr.size()) {
                wraps = false;
                break;
            }
        }
        if (!wraps) break;
        expr = picker::trim(expr.substr(1, expr.size() - 2));
    }
    return expr;
}

std::string normalize_expr(const std::string &expr)
{
    auto normalized = replace_member_refs_with_dots(expr);
    std::string compact;
    compact.reserve(normalized.size());
    bool lastWasSpace = false;
    for (unsigned char ch : normalized) {
        if (std::isspace(ch)) {
            if (!lastWasSpace) compact.push_back(' ');
            lastWasSpace = true;
        } else {
            compact.push_back(static_cast<char>(ch));
            lastWasSpace = false;
        }
    }
    return picker::trim(compact);
}

trace_parse_result build_result(const std::vector<trace_event> &events,
                                const std::vector<cpp_variableInfo> &vars)
{
    std::unordered_map<int, std::set<std::string>> slotToNames;
    std::unordered_map<int, trace_event> slotToDecl;
    std::unordered_map<int, trace_event> slotToSource;
    trace_parse_result result;
    auto varLookup = build_var_lookup(vars);

    for (const auto &event : events) {
        if (event.kind == trace_event_kind::declaration) {
            if (slotToNames[event.slot].insert(event.name).second) ++result.decl_count;
            if (!slotToDecl.contains(event.slot)) slotToDecl[event.slot] = event;
            continue;
        }
        auto source = event;
        if (slotToSource.contains(source.slot)
            && slotToSource[source.slot].is_full && !source.is_full) {
            continue;
        }
        if (!slotToSource.contains(source.slot)) ++result.value_count;
        source.raw_expr = strip_wrapping_parentheses(source.raw_expr);
        source.expr = normalize_expr(source.raw_expr);
        slotToSource[source.slot] = source;
    }

    for (const auto &[slot, names] : slotToNames) {
        auto sourceIt = slotToSource.find(slot);
        if (sourceIt == slotToSource.end()) continue;
        ++result.matched_slot_count;
        const auto &source = sourceIt->second;
        auto deps = extract_member_refs(source.raw_expr);
        auto kind = classify_expr(source.raw_expr, deps);
        trace_event decl;
        if (slotToDecl.contains(slot)) decl = slotToDecl.at(slot);

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
            info.type = decl.type;
            info.width = decl.width;
            info.source_expr = source.expr;
            info.deps = bindings;
            if ((kind == "direct" || kind == "projection") && deps.size() == 1) {
                info.root_name = deps.front();
            }
            result.signals.push_back(std::move(info));
        }
    }

    std::sort(result.signals.begin(), result.signals.end(),
              [](const trace_signal_info &a, const trace_signal_info &b) {
                  if (a.name == b.name) return a.slot < b.slot;
                  return a.name < b.name;
              });
    return result;
}

} } } } // namespace picker::parser::verilator::detail
namespace picker { namespace parser {

namespace {

std::string yaml_escape(const std::string &input)
{
    std::string out;
    out.reserve(input.size() + 8);
    for (char ch : input) {
        if (ch == '\\' || ch == '"') out.push_back('\\');
        out.push_back(ch);
    }
    return out;
}

} // namespace

void outputSignalYAML(const std::vector<trace_signal_info> &signals, const std::string &fileName)
{
    std::ofstream file;
    std::ostream *out = &std::cout;
    if (!fileName.empty()) {
        auto absFPath = std::filesystem::absolute(fileName).string();
        file.open(absFPath);
        if (!file.is_open()) PK_FATAL("Failed to open file: %s", absFPath.c_str());
        out = &file;
    }

    *out << "signals:\n";
    for (const auto &signal : signals) {
        *out << "  - name: \"" << yaml_escape(signal.name) << "\"\n";
        *out << "    kind: " << signal.kind << "\n";
        if (signal.kind == "direct") {
            *out << "    source: \"" << yaml_escape(signal.root_name) << "\"\n";
            if (!signal.type.empty()) *out << "    type: \"" << yaml_escape(signal.type) << "\"\n";
            if (signal.width > 0) *out << "    rtl_width: " << signal.width << "\n";
            continue;
        }
        if (signal.kind == "projection") {
            if (!signal.root_name.empty()) {
                *out << "    source: \"" << yaml_escape(signal.root_name) << "\"\n";
            }
            *out << "    expr: \"" << yaml_escape(signal.source_expr) << "\"\n";
            if (!signal.type.empty()) *out << "    type: \"" << yaml_escape(signal.type) << "\"\n";
            if (signal.width > 0) *out << "    rtl_width: " << signal.width << "\n";
            continue;
        }
        if (signal.kind == "expr") {
            if (!signal.deps.empty()) {
                *out << "    deps:\n";
                for (const auto &dep : signal.deps) {
                    *out << "      - \"" << yaml_escape(dep.name) << "\"\n";
                }
            }
            if (!signal.type.empty()) *out << "    type: \"" << yaml_escape(signal.type) << "\"\n";
            if (signal.width > 0) *out << "    rtl_width: " << signal.width << "\n";
            *out << "    expr: \"" << yaml_escape(signal.source_expr) << "\"\n";
            continue;
        }
        if (!signal.type.empty()) *out << "    type: \"" << yaml_escape(signal.type) << "\"\n";
        if (signal.width > 0) *out << "    rtl_width: " << signal.width << "\n";
        *out << "    value: \"" << yaml_escape(signal.source_expr) << "\"\n";
    }
    out->flush();
}

namespace verilator {

std::vector<std::string> findTraceFiles(const std::string &rootHeaderFile)
{
    std::vector<std::string> traceFiles;
    std::filesystem::path rootPath(rootHeaderFile);
    if (!std::filesystem::exists(rootPath)) return traceFiles;

    std::smatch match;
    auto fileName = rootPath.filename().string();
    if (!std::regex_match(fileName, match, std::regex(R"(^V(.+)___024root\.h$)"))) {
        return traceFiles;
    }

    auto parent = rootPath.parent_path();
    auto topName = match[1].str();
    std::regex traceRegex("^V" + topName + R"(__Trace__.*\.cpp$)");
    for (const auto &entry : std::filesystem::directory_iterator(parent)) {
        if (!entry.is_regular_file()) continue;
        if (std::regex_match(entry.path().filename().string(), traceRegex)) {
            traceFiles.push_back(entry.path().string());
        }
    }
    std::sort(traceFiles.begin(), traceFiles.end());
    return traceFiles;
}

} // namespace verilator
} } // namespace picker::parser
