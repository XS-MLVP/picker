#include "gen_addr.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

std::map<std::string, int> size_map;
std::vector<cpp_variableInfo> all_vars;

{{__render_varible_info_sub_define__}}

namespace {

std::string trim(const std::string &input)
{
    auto begin = input.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }
    auto end = input.find_last_not_of(" \t\r\n");
    return input.substr(begin, end - begin + 1);
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

std::string yaml_unquote(const std::string &input)
{
    auto value = trim(input);
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
    }

    std::string out;
    out.reserve(value.size());
    bool escaping = false;
    for (char ch : value) {
        if (escaping) {
            out.push_back(ch);
            escaping = false;
            continue;
        }
        if (ch == '\\') {
            escaping = true;
            continue;
        }
        out.push_back(ch);
    }
    return out;
}

bool starts_with(const std::string &line, const std::string &prefix)
{
    return line.rfind(prefix, 0) == 0;
}

std::unordered_map<std::string, cpp_variableInfo> build_var_lookup()
{
    std::unordered_map<std::string, cpp_variableInfo> lookup;
    for (const auto &var : all_vars) {
        lookup[var.name] = var;
    }
    return lookup;
}

std::vector<raw_signal_info> load_raw_signal_map(const std::filesystem::path &rawFile)
{
    std::ifstream in(rawFile);
    if (!in.is_open()) {
        return {};
    }

    std::vector<raw_signal_info> signals;
    raw_signal_info current;
    bool inSignal = false;
    bool inDeps = false;
    std::string line;

    while (std::getline(in, line)) {
        auto trimmed = trim(line);
        if (trimmed.empty() || trimmed == "signals:") {
            continue;
        }

        if (starts_with(line, "  - name: ")) {
            if (inSignal) {
                signals.push_back(current);
            }
            current = {};
            current.name = yaml_unquote(line.substr(std::string("  - name: ").size()));
            inSignal = true;
            inDeps = false;
            continue;
        }

        if (!inSignal) {
            continue;
        }

        if (starts_with(line, "    kind: ")) {
            current.kind = trim(line.substr(std::string("    kind: ").size()));
            inDeps = false;
            continue;
        }
        if (starts_with(line, "    source: ")) {
            current.source = yaml_unquote(line.substr(std::string("    source: ").size()));
            inDeps = false;
            continue;
        }
        if (starts_with(line, "    type: ")) {
            current.type = yaml_unquote(line.substr(std::string("    type: ").size()));
            inDeps = false;
            continue;
        }
        if (starts_with(line, "    rtl_width: ")) {
            current.rtl_width = (uint32_t)std::stoul(trim(line.substr(std::string("    rtl_width: ").size())));
            inDeps = false;
            continue;
        }
        if (starts_with(line, "    expr: ")) {
            current.expr = yaml_unquote(line.substr(std::string("    expr: ").size()));
            inDeps = false;
            continue;
        }
        if (starts_with(line, "    value: ")) {
            current.value = yaml_unquote(line.substr(std::string("    value: ").size()));
            inDeps = false;
            continue;
        }
        if (starts_with(line, "    deps:")) {
            inDeps = true;
            continue;
        }
        if (inDeps && starts_with(line, "      - ")) {
            current.deps.push_back(yaml_unquote(line.substr(std::string("      - ").size())));
            continue;
        }
        if (starts_with(line, "    ")) {
            inDeps = false;
        }
    }

    if (inSignal) {
        signals.push_back(current);
    }
    return signals;
}

void write_offset_yaml(std::ostream &out)
{
    out << "variables:\n";
    for (const auto &var : all_vars) {
        var.write_yaml(out);
    }
}

void write_signal_dep(std::ostream &out, const cpp_variableInfo &var, const std::string &prefix)
{
    out << prefix << "type: \"" << yaml_escape(var.type) << "\"\n";
    out << prefix << "mem_bytes: " << var.mem_bytes() << "\n";
    out << prefix << "rtl_width: " << var.width << "\n";
    if (var.array_size > 0) {
        out << prefix << "array_size: " << var.array_size << "\n";
    }
    out << prefix << "offset: " << var.offset << "\n";
}

void write_signal_source(std::ostream &out, const cpp_variableInfo &var, const std::string &prefix)
{
    out << prefix << "source_type: \"" << yaml_escape(var.type) << "\"\n";
    out << prefix << "source_mem_bytes: " << var.mem_bytes() << "\n";
    out << prefix << "source_rtl_width: " << var.width << "\n";
    if (var.array_size > 0) {
        out << prefix << "source_array_size: " << var.array_size << "\n";
    }
    out << prefix << "source_offset: " << var.offset << "\n";
}

void write_signals_yaml(std::ostream &out, const std::vector<raw_signal_info> &signals)
{
    auto lookup = build_var_lookup();
    size_t unresolvedExprDeps = 0;

    out << "signals:\n";
    for (const auto &signal : signals) {
        out << "  - name: \"" << yaml_escape(signal.name) << "\"\n";
        out << "    kind: " << signal.kind << "\n";

        if (signal.kind == "direct" || signal.kind == "projection") {
            auto it = lookup.find(signal.source);
            if (it == lookup.end()) {
                fprintf(stderr, "Signal source not found in mem_direct vars: %s (%s)\n",
                        signal.name.c_str(), signal.source.c_str());
                exit(1);
            }
            out << "    source: \"" << yaml_escape(signal.source) << "\"\n";
            if (signal.kind == "direct") {
                write_signal_dep(out, it->second, "    ");
            } else {
                if (!signal.type.empty()) {
                    out << "    type: \"" << yaml_escape(signal.type) << "\"\n";
                }
                if (signal.rtl_width > 0) {
                    out << "    rtl_width: " << signal.rtl_width << "\n";
                }
                out << "    expr: \"" << yaml_escape(signal.expr) << "\"\n";
                write_signal_source(out, it->second, "    ");
            }
            continue;
        }

        if (signal.kind == "expr") {
            if (!signal.type.empty()) {
                out << "    type: \"" << yaml_escape(signal.type) << "\"\n";
            }
            if (signal.rtl_width > 0) {
                out << "    rtl_width: " << signal.rtl_width << "\n";
            }
            if (!signal.deps.empty()) {
                out << "    deps:\n";
                for (const auto &dep : signal.deps) {
                    out << "      - name: \"" << yaml_escape(dep) << "\"\n";
                    auto it = lookup.find(dep);
                    if (it == lookup.end()) {
                        ++unresolvedExprDeps;
                        continue;
                    }
                    write_signal_dep(out, it->second, "        ");
                }
            }
            out << "    expr: \"" << yaml_escape(signal.expr) << "\"\n";
            continue;
        }

        if (!signal.type.empty()) {
            out << "    type: \"" << yaml_escape(signal.type) << "\"\n";
        }
        if (signal.rtl_width > 0) {
            out << "    rtl_width: " << signal.rtl_width << "\n";
        }
        out << "    value: \"" << yaml_escape(signal.value) << "\"\n";
    }

    if (unresolvedExprDeps > 0) {
        fprintf(stderr, "Merged signal map with %zu unresolved expr dependency binding(s)\n", unresolvedExprDeps);
    }
}

std::filesystem::path executable_dir(const char *argv0)
{
    return std::filesystem::absolute(argv0).parent_path();
}

std::vector<raw_signal_info> try_load_raw_signal_map(const char *argv0)
{
    auto exeDir = executable_dir(argv0);
    auto rawFile = exeDir / "{{__TOP_MODULE_NAME__}}_signal_map.raw.yaml";
    if (!std::filesystem::exists(rawFile)) {
        return {};
    }

    auto signals = load_raw_signal_map(rawFile);
    if (signals.empty()) {
        fprintf(stderr, "Raw signal map exists but could not be parsed: %s\n", rawFile.string().c_str());
        exit(1);
    }
    return signals;
}

} // namespace

void render_varible_info()
{
{% if __SIMULATOR__ == "verilator" %}
    V{{__TOP_MODULE_NAME__}} *vptr = new V{{__TOP_MODULE_NAME__}};
    BaseType &base                 = *(vptr->rootp);
{% endif %}
{% if __SIMULATOR__ == "gsim" %}
    S{{__TOP_MODULE_NAME__}} *vptr = new S{{__TOP_MODULE_NAME__}};
    BaseType &base                 = *vptr;
{% endif %}

    // {{__render_varible_info_sub__}}

    return;
}

void init_type_size()
{
{% if __SIMULATOR__ == "verilator" %}
    SET_SIZE(CData);
    SET_SIZE(SData);
    SET_SIZE(IData);
    SET_SIZE(WData);
    SET_SIZE(QData);
{% endif %}
{% if __SIMULATOR__ == "gsim" %}
    {% for t in __yaml_varible_type_set__ %}
    SET_SIZE({{t}});
    {% endfor %}
{% endif %}
}

int main(int argc, char **argv)
{
    init_type_size();
    render_varible_info();
    auto signals = try_load_raw_signal_map(argv[0]);
    write_offset_yaml(std::cout);
    if (!signals.empty()) {
        write_signals_yaml(std::cout, signals);
    }
    return 0;
}
