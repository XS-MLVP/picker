#include <fstream>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <limits>
#include <cctype>
#include <functional>

#include "common.hpp"

namespace picker { namespace parser { namespace verilator { namespace detail {

namespace {

std::vector<std::string> extract_member_refs(const std::string &expr)
{
    std::vector<std::string> refs;
    std::unordered_set<std::string> seen;
    static const std::regex ref_regex(
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
    static const std::regex ref_regex(
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
    static const std::regex constant_regex(R"([0-9A-Fa-fxXuUlL+\-<>&|~()?:]+)");
    return !compact.empty() && std::regex_match(compact, constant_regex);
}
bool is_direct_expr(const std::string &expr, const std::string &dep)
{
    auto compact = replace_member_refs_with_dots(expr);
    compact.erase(std::remove_if(compact.begin(), compact.end(),
                                 [](unsigned char ch) { return std::isspace(ch); }),
                  compact.end());
    static const std::regex cast_regex(R"(\([A-Za-z_][A-Za-z0-9_:<>]*\))");
    compact = std::regex_replace(compact, cast_regex, "");
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

namespace {

struct cpp_function_definition {
    std::string name;
    std::string body;
    size_t begin;
    size_t end;
};

struct dtype_scope_entry {
    std::string name;
    bool array_unpacked = false;
    bool array_declared = false;
    int array_size = 0;
};

// Only definitions are matched, never prototypes or calls. Braces inside
// debug strings/comments must not terminate a function body. Scanning by hand
// keeps multi-megabyte trace files from spending seconds in std::regex.
template <typename Accept>
std::vector<cpp_function_definition> cpp_function_definitions(const std::string &content,
                                                             Accept accept)
{
    std::vector<cpp_function_definition> definitions;
    size_t search = 0;
    while (search < content.size()) {
        auto decl = content.find("void", search);
        if (decl == std::string::npos) break;
        if (decl > 0
            && (std::isalnum(static_cast<unsigned char>(content[decl - 1]))
                || content[decl - 1] == '_')) {
            search = decl + 4;
            continue;
        }
        auto nameBegin = decl + 4;
        if (nameBegin >= content.size()
            || !std::isspace(static_cast<unsigned char>(content[nameBegin]))) {
            search = decl + 4;
            continue;
        }
        while (nameBegin < content.size()
               && std::isspace(static_cast<unsigned char>(content[nameBegin]))) {
            ++nameBegin;
        }
        auto nameEnd = nameBegin;
        while (nameEnd < content.size()
               && (std::isalnum(static_cast<unsigned char>(content[nameEnd]))
                   || content[nameEnd] == '_')) {
            ++nameEnd;
        }
        if (nameEnd == nameBegin) {
            search = decl + 4;
            continue;
        }
        auto name = content.substr(nameBegin, nameEnd - nameBegin);
        if (!accept(name)) {
            search = nameEnd;
            continue;
        }

        auto open = nameEnd;
        while (open < content.size()
               && std::isspace(static_cast<unsigned char>(content[open]))) {
            ++open;
        }
        if (open >= content.size() || content[open] != '(') {
            search = nameEnd;
            continue;
        }
        int parenDepth = 0;
        size_t close = open;
        for (; close < content.size(); ++close) {
            if (content[close] == '(') ++parenDepth;
            else if (content[close] == ')') {
                --parenDepth;
                if (parenDepth == 0) break;
            }
        }
        if (close >= content.size()) break;
        auto brace = close + 1;
        while (brace < content.size()
               && std::isspace(static_cast<unsigned char>(content[brace]))) {
            ++brace;
        }
        // Prototypes end with ';' and must not be treated as definitions.
        if (brace >= content.size() || content[brace] != '{') {
            search = close + 1;
            continue;
        }

        size_t body = brace + 1;
        int depth = 1;
        bool quoted = false, escaped = false, lineComment = false, blockComment = false;
        size_t end = body;
        for (; end < content.size() && depth; ++end) {
            char ch = content[end];
            char next = end + 1 < content.size() ? content[end + 1] : '\0';
            if (lineComment) { if (ch == '\n') lineComment = false; continue; }
            if (blockComment) { if (ch == '*' && next == '/') { blockComment = false; ++end; } continue; }
            if (quoted) {
                if (escaped) escaped = false;
                else if (ch == '\\') escaped = true;
                else if (ch == '"') quoted = false;
                continue;
            }
            if (ch == '/' && next == '/') { lineComment = true; ++end; }
            else if (ch == '/' && next == '*') { blockComment = true; ++end; }
            else if (ch == '"') quoted = true;
            else if (ch == '{') ++depth;
            else if (ch == '}') --depth;
        }
        if (depth) throw std::runtime_error("unterminated trace dtype helper: " + name);
        definitions.push_back({name, content.substr(body, end - body - 1), decl, end});
        search = end;
    }
    return definitions;
}

int constant_slot(std::string value)
{
    value = picker::trim(value);
    static const std::regex integer(R"((?:0[xX][0-9A-Fa-f]+|[0-9]+)[uUlL]*)");
    if (!std::regex_match(value, integer)) {
        throw std::runtime_error("unsupported trace dtype offset: " + value);
    }
    auto number = std::stoull(value, nullptr, value.rfind("0x", 0) == 0
                                             || value.rfind("0X", 0) == 0 ? 16 : 10);
    if (number > static_cast<unsigned long long>(std::numeric_limits<int>::max())) {
        throw std::runtime_error("trace dtype offset out of range: " + value);
    }
    return static_cast<int>(number);
}

} // namespace

trace_input read_trace_input(const std::vector<std::string> &traceFiles)
{
    trace_input input;
    for (const auto &file : traceFiles) {
        auto content = read_file(file);
        input.modern |= content.find("VL_TRACE_") != std::string::npos;
        input.legacy |= content.find("tracep->decl") != std::string::npos
                     || content.find("tracep->pushPrefix") != std::string::npos
                     || content.find("tracep->popPrefix") != std::string::npos;
        size_t position = 0;
        std::string loose;
        for (const auto &definition : cpp_function_definitions(content, [](const std::string &name) {
                 return name.find("__trace_") != std::string::npos;
             })) {
            loose.append(content, position, definition.begin - position);
            loose += '\n';
            position = definition.end;
            if (!input.functions.emplace(definition.name, split_statements(definition.body)).second) {
                throw std::runtime_error("duplicate trace helper: " + definition.name);
            }
        }
        loose.append(content, position, std::string::npos);
        auto statements = split_statements(loose);
        input.loose.insert(input.loose.end(), std::make_move_iterator(statements.begin()),
                           std::make_move_iterator(statements.end()));
    }
    return input;
}

namespace {

int add_slots(int base, int offset)
{
    if (base < 0 || offset < 0 || base > std::numeric_limits<int>::max() - offset) {
        throw std::runtime_error("trace slot overflow");
    }
    return base + offset;
}

std::vector<trace_event> instantiate_sources(const trace_input &input)
{
    std::vector<trace_event> events;
    static const std::regex dtype_call(
        R"(([A-Za-z_][A-Za-z0-9_]*__trace_(?:full|chg)_dtype[A-Za-z0-9_]*)\s*\()");
    std::set<std::string> active;
    std::function<void(const std::vector<std::string> &, int, const std::string &)> visit =
        [&](const std::vector<std::string> &statements, int base, const std::string &bound) {
        int shift = base;
        for (const auto &statement : statements) {
            std::vector<std::string> args;
            if (unwrap_call_args(statement, "bufp->oldp", args)) {
                auto value = args.at(0);
                value.erase(std::remove_if(value.begin(), value.end(),
                             [](unsigned char ch) { return std::isspace(ch); }), value.end());
                const std::string prefix = "vlSymsp->__Vm_baseCode";
                if (value == prefix) shift = 0;
                else if (value == prefix + "+offset") shift = base;
                else if (value.rfind(prefix + "+", 0) == 0) {
                    shift = constant_slot(value.substr(prefix.size() + 1));
                } else throw std::runtime_error("unsupported trace oldp base: " + value);
                continue;
            }
            bool full = statement.find("bufp->full") != std::string::npos;
            bool change = statement.find("bufp->chg") != std::string::npos;
            if (full || change) {
                int local = 0;
                if (!unwrap_call_args(statement, full ? "bufp->full" : "bufp->chg", args)
                    || args.size() < 2 || !parse_slot_arg(args[0], local)) {
                    throw std::runtime_error("unsupported trace sample: " + statement);
                }
                auto raw = args[1];
                const std::string token = "__VdtypeVar";
                auto pos = raw.find(token);
                if (pos != std::string::npos) {
                    if (bound.empty()) throw std::runtime_error("unbound trace dtype source");
                    do {
                        raw.replace(pos, token.size(), bound);
                        pos = raw.find(token, pos + bound.size());
                    } while (pos != std::string::npos);
                }
                trace_event event;
                event.kind = trace_event_kind::source;
                event.slot = add_slots(shift, local);
                event.raw_expr = std::move(raw);
                event.is_full = full;
                events.push_back(std::move(event));
                continue;
            }
            if (statement.find("_dtype") == std::string::npos) continue;
            std::smatch match;
            if (!std::regex_search(statement, match, dtype_call)) continue;
            auto name = match[1].str();
            if (!unwrap_call_args(statement, name, args)) {
                throw std::runtime_error("invalid trace dtype call: " + name);
            }
            if (args[0] != "vlSelf") continue; // prototype, not a call
            if (args.size() != 4 || args[1] != "bufp") {
                throw std::runtime_error("invalid trace dtype call: " + name);
            }
            auto helper = input.functions.find(name);
            if (helper == input.functions.end()) throw std::runtime_error("missing trace dtype helper: " + name);
            if (!active.insert(name).second) throw std::runtime_error("recursive trace dtype helper: " + name);
            auto actual = args[3];
            auto token = actual.find("__VdtypeVar");
            if (token != std::string::npos) {
                if (bound.empty()) throw std::runtime_error("unbound trace dtype argument");
                actual.replace(token, std::string("__VdtypeVar").size(), bound);
            }
            int offset = args[2] == "offset" ? base : constant_slot(args[2]);
            auto before = events.size();
            visit(helper->second, offset, actual);
            if (events.size() == before) throw std::runtime_error("empty trace dtype source helper: " + name);
            active.erase(name);
        }
    };
    visit(input.loose, 0, "");
    for (const auto &[name, statements] : input.functions) {
        if (name.find("_dtype") == std::string::npos) visit(statements, 0, "");
    }
    return events;
}

// These are Verilator's generated argument forms, not arbitrary C++:
// a constant, c, c+constant, and (for a whole array) c+constant+i*stride.
// The array declaration is emitted once at i=0; array_size already describes
// the remaining elements. No expression evaluator or loop interpreter is needed.
int declaration_slot(std::string arg, int base, bool array = false)
{
    arg.erase(std::remove_if(arg.begin(), arg.end(),
                             [](unsigned char ch) { return std::isspace(ch); }), arg.end());
    if (array) {
        auto index = arg.find("+i*");
        if (index != std::string::npos) {
            if (constant_slot(arg.substr(index + 3)) == 0) {
                throw std::runtime_error("zero trace array stride");
            }
            arg.resize(index);
        }
    }
    if (arg == "c") return base;
    if (arg.rfind("c+", 0) != 0) return constant_slot(arg);
    int offset = constant_slot(arg.substr(2));
    return add_slots(base, offset);
}

std::string declaration_name(const std::string &arg, const std::string &bound)
{
    auto value = picker::trim(arg);
    if (value == "name") return bound;
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') return strip_quotes(value);
    throw std::runtime_error("unsupported trace declaration name: " + value);
}

std::string init_call(const std::string &statement)
{
    for (auto pos = statement.find("__trace_init_"); pos != std::string::npos;
         pos = statement.find("__trace_init_", pos + 1)) {
        auto begin = pos, end = pos;
        auto identifier = [](unsigned char ch) { return std::isalnum(ch) || ch == '_'; };
        while (begin && identifier(statement[begin - 1])) --begin;
        while (end < statement.size() && identifier(statement[end])) ++end;
        auto next = statement.find_first_not_of(" \t\r\n", end);
        if (next != std::string::npos && statement[next] == '(') return statement.substr(begin, end - begin);
    }
    return {};
}

std::vector<trace_event> instantiate_trace_declarations(const trace_input &input, bool modern)
{
    const auto &functions = input.functions;
    std::set<std::string> roots;
    for (const auto &[name, statements] : functions) {
        if (name.find("__trace_init_") != std::string::npos
            && name.find("_dtype") == std::string::npos) roots.insert(name);
    }
    const bool structured = !roots.empty();
    for (const auto &[name, statements] : functions) {
        for (const auto &statement : statements) {
            roots.erase(init_call(statement));
        }
    }
    if (structured && roots.empty()) throw std::runtime_error("trace initialization call cycle");

    std::vector<trace_event> events;
    std::vector<dtype_scope_entry> scope;
    std::set<std::string> active, visited;
    std::function<void(const std::vector<std::string> &, const std::string &, int)> walk;
    auto visit = [&](const std::string &name, const std::string &bound, int base) {
        auto function = functions.find(name);
        if (function == functions.end()) throw std::runtime_error("missing trace initialization helper: " + name);
        if (!active.insert(name).second) throw std::runtime_error("recursive trace initialization helper: " + name);
        visited.insert(name);
        walk(function->second, bound, base);
        active.erase(name);
    };
    walk = [&](const std::vector<std::string> &statements, const std::string &bound, int base) {
            for (const auto &statement : statements) {
                std::vector<std::string> args;
                if (statement.rfind("for", 0) == 0) {
                    // Validate the emitted array pattern, do not interpret C++ loops.
                    static const std::regex loop(
                        R"(for\s*\(\s*int\s+i\s*=\s*0\s*;\s*i\s*<\s*([0-9]+)\s*;\s*\+\+i\s*\)\s*\{)");
                    std::smatch match;
                    if (scope.empty() || !scope.back().array_unpacked
                        || !std::regex_match(statement, match, loop)) {
                        throw std::runtime_error("unsupported trace declaration loop: " + statement);
                    }
                    scope.back().array_size = constant_slot(match[1]);
                    continue;
                }
                if (unwrap_call_args(statement, modern ? "VL_TRACE_PUSH_PREFIX" : "tracep->pushPrefix", args)) {
                    size_t index = modern ? 1 : 0;
                    if (args.size() <= index + 1) throw std::runtime_error("invalid trace prefix");
                    scope.push_back({declaration_name(args[index], bound),
                                     args[index + 1].find("ARRAY_UNPACKED") != std::string::npos, false, 0});
                    continue;
                }
                if (statement.find(modern ? "VL_TRACE_POP_PREFIX" : "tracep->popPrefix") != std::string::npos) {
                    if (scope.empty()) throw std::runtime_error("trace prefix underflow");
                    scope.pop_back();
                    continue;
                }
                auto callee = init_call(statement);
                if (!callee.empty()) {
                    if (!unwrap_call_args(statement, callee, args) || args.size() < 2) {
                        throw std::runtime_error("invalid trace helper call: " + callee);
                    }
                    if (callee.find("_dtype") != std::string::npos) {
                        if (args.size() != 6) throw std::runtime_error("invalid trace dtype call: " + callee);
                        visit(callee, declaration_name(args[2], bound), declaration_slot(args[4], base));
                    } else {
                        // Split helpers share the runtime prefix stack, including
                        // pushes/pops that straddle sibling function calls.
                        visit(callee, bound, base);
                    }
                    continue;
                }
                const char *marker = modern ? "VL_TRACE_DECL_" : "tracep->decl";
                if (statement.find(marker) == std::string::npos) continue;
                if (!unwrap_call_args(statement, marker, args)) {
                    throw std::runtime_error("invalid trace declaration: " + statement);
                }
                size_t slotIndex = modern ? 1 : 0, nameIndex = modern ? 3 : 2;
                if (args.size() <= nameIndex) throw std::runtime_error("invalid trace declaration");
                auto leaf = declaration_name(args[nameIndex], bound);
                bool array = leaf.empty() && !scope.empty() && scope.back().array_unpacked;
                if (array && scope.back().array_declared) continue;
                trace_event event;
                auto parse = modern ? parse_v5_048_declaration : parse_v5_026_declaration;
                if (!parse(statement, args, declaration_slot(args[slotIndex], base, array), event)) {
                    throw std::runtime_error("unsupported trace declaration: " + statement);
                }
                std::vector<std::string> names;
                for (const auto &entry : scope) names.push_back(entry.name);
                event.name = join_scope(names, leaf);
                event.is_array = array;
                if (array) event.array_size = scope.back().array_size;
                if (array) scope.back().array_declared = true;
                if (!event.name.empty()) events.push_back(std::move(event));
            }
        };
    if (!structured) walk(input.loose, "", 0); // Small fixtures without function wrappers.
    if (!structured && !scope.empty()) throw std::runtime_error("unbalanced trace initialization prefixes");
    for (const auto &root : roots) {
        visit(root, "", 0);
        if (!scope.empty()) throw std::runtime_error("unbalanced trace initialization prefixes");
    }
    for (const auto &[name, statements] : functions) {
        if (name.find("__trace_init_") != std::string::npos
            && name.find("_dtype") == std::string::npos && !visited.count(name)) {
            throw std::runtime_error("unreachable trace initialization helper: " + name);
        }
    }
    return events;
}

} // namespace

std::vector<trace_event> parse_trace_events(const trace_input &input, bool modern)
{
    auto events = instantiate_trace_declarations(input, modern);
    auto sources = instantiate_sources(input);
    events.insert(events.end(), std::make_move_iterator(sources.begin()),
                  std::make_move_iterator(sources.end()));
    return events;
}

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
    int depth = 0;
    bool quoted = false, escaped = false, lineComment = false, blockComment = false;
    auto emit = [&] {
        auto text = picker::trim(current);
        if (text.rfind("VL_DEBUG_IF", 0) != 0
            && (text.find("VL_TRACE_") != std::string::npos
                || text.find("tracep->") != std::string::npos
                || text.find("bufp->") != std::string::npos
                || text.find("__trace_") != std::string::npos
                || text.rfind("for", 0) == 0)) statements.push_back(std::move(text));
        current.clear();
    };
    for (size_t i = 0; i < content.size(); ++i) {
        char ch = content[i], next = i + 1 < content.size() ? content[i + 1] : '\0';
        if (lineComment) { if (ch == '\n') lineComment = false; continue; }
        if (blockComment) {
            if (ch == '*' && next == '/') { blockComment = false; ++i; }
            continue;
        }
        if (quoted) {
            current.push_back(ch);
            if (escaped) escaped = false;
            else if (ch == '\\') escaped = true;
            else if (ch == '"') quoted = false;
            continue;
        }
        if (ch == '/' && (next == '/' || next == '*')) {
            lineComment = next == '/'; blockComment = next == '*';
            current.push_back(' '); ++i; continue;
        }
        current.push_back(ch);
        if (ch == '"') quoted = true;
        else if (ch == '(') ++depth;
        else if (ch == ')') --depth;
        else if (depth == 0 && (ch == ';' || ch == '{' || ch == '}')) emit();
    }
    emit();
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
    if (open == std::string::npos) return false;
    int depth = 1;
    bool quoted = false, escaped = false;
    auto close = open + 1;
    for (; close < statement.size(); ++close) {
        char ch = statement[close];
        if (quoted) {
            if (escaped) escaped = false;
            else if (ch == '\\') escaped = true;
            else if (ch == '"') quoted = false;
        } else if (ch == '"') quoted = true;
        else if (ch == '(') ++depth;
        else if (ch == ')' && --depth == 0) break;
    }
    if (depth != 0) return false;
    args = split_top_level_args(statement.substr(open + 1, close - open - 1));
    return !args.empty();
}

bool parse_slot_arg(const std::string &arg, int &slot)
{
    auto value = picker::trim(arg);
    size_t position = 0;
    if (value.compare(0, 4, "oldp") == 0) position = 4;
    else if (value.compare(0, 1, "c") == 0) position = 1;
    else return false;
    while (position < value.size()
           && std::isspace(static_cast<unsigned char>(value[position]))) {
        ++position;
    }
    if (position >= value.size() || value[position] != '+') return false;
    ++position;
    while (position < value.size()
           && std::isspace(static_cast<unsigned char>(value[position]))) {
        ++position;
    }
    if (position >= value.size()
        || !std::isdigit(static_cast<unsigned char>(value[position]))) {
        return false;
    }
    long long number = 0;
    while (position < value.size()
           && std::isdigit(static_cast<unsigned char>(value[position]))) {
        number = number * 10 + (value[position] - '0');
        if (number > std::numeric_limits<int>::max()) return false;
        ++position;
    }
    if (position != value.size()) return false;
    slot = static_cast<int>(number);
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
    std::map<std::pair<int, std::string>, trace_event> declarations;
    std::unordered_map<int, trace_event> slotToSource;
    trace_parse_result result;
    auto varLookup = build_var_lookup(vars);

    for (const auto &event : events) {
        if (event.kind == trace_event_kind::declaration) {
            if (slotToNames[event.slot].insert(event.name).second) ++result.decl_count;
            declarations[{event.slot, event.name}] = event;
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
        auto source = sourceIt->second;
        auto deps = extract_member_refs(source.raw_expr);
        auto kind = classify_expr(source.raw_expr, deps);

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
            const auto &decl = declarations.at({slot, name});
            trace_signal_info info;
            info.name = name;
            info.slot = slot;
            info.kind = kind;
            info.type = decl.type;
            info.width = decl.width;
            info.source_expr = source.expr;
            info.deps = bindings;
            // Only an explicit whole-array declaration may bind the array base.
            // A scalar alias can share its first slot and must remain a projection.
            if (decl.is_array && deps.size() == 1) {
                auto var = varLookup.find(deps.front());
                if (var != varLookup.end() && var->second.array_size > 0
                    && decl.array_size == var->second.array_size
                    && decl.width == var->second.width
                    && strip_wrapping_parentheses(source.expr) == deps.front() + "[0]") {
                    info.kind = "direct";
                    info.source_expr = deps.front();
                }
            }
            if ((info.kind == "direct" || info.kind == "projection") && deps.size() == 1) {
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
