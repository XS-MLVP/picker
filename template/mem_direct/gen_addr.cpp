#include "gen_addr.hpp"
#include <iostream>

std::vector<cpp_variableInfo> all_vars;

{{__RENDER_VARIABLE_INFO_SUB_DEFINE__}}

namespace {
std::unordered_map<std::string, cpp_variableInfo> build_var_lookup()
{
    std::unordered_map<std::string, cpp_variableInfo> lookup;
    for (const auto &var : all_vars) lookup[var.name] = var;
    return lookup;
}

cpp_variableInfo resolve_source(const std::unordered_map<std::string, cpp_variableInfo> &lookup,
                                const std::string &source)
{
    auto found = lookup.find(source);
    if (found != lookup.end()) return found->second;
    // Trace selections use C++ storage coordinates. Only sizeof-derived
    // strides may be used to resolve their actual native offsets.
    auto bracket = source.find('[');
    auto base = lookup.find(source.substr(0, bracket));
    if (bracket != std::string::npos && base != lookup.end()) {
        auto var = base->second;
        size_t dimension = 0;
        while (bracket < source.size() && source[bracket] == '[') {
            auto end = source.find(']', bracket);
            if (end == std::string::npos) break;
            auto text = source.substr(bracket + 1, end - bracket - 1);
            if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos) break;
            auto index = std::stoull(text);
            if (dimension >= var.array_dimensions.size() || index >= uint64_t(var.array_dimensions[dimension])) break;
            var.offset += index * var.array_strides[dimension++];
            bracket = end + 1;
        }
        if (bracket == source.size() && dimension == var.array_dimensions.size()) {
            var.name = source; var.array_size = 0;
            var.array_dimensions.clear(); var.array_strides.clear();
            // These describe the complete array, not this selected element.
            // Recursive lookup must reuse the verified parent's RTL view and
            // its cached leaf instead of inventing an element layout/binding.
            var.layout.clear(); var.groups.clear();
            return var;
        }
    }
    return {}; // A trace expression alone is not evidence of native storage.
}
}

void render_variable_info()
{
{% if __SIMULATOR__ == "verilator" %}
    auto model = std::make_unique<V{{__TOP_MODULE_NAME__}}>();
    BaseType &base = *model->rootp;
{% endif %}
{% if __SIMULATOR__ == "gsim" %}
    auto model = std::make_unique<S{{__TOP_MODULE_NAME__}}>();
    BaseType &base = *model;
{% endif %}
{{__RENDER_VARIABLE_INFO_SUB__}}
}

int main(int argc, char **argv)
{
    try {
        render_variable_info();
        const auto path = argc > 1 ? std::filesystem::path(argv[1]) :
            std::filesystem::absolute(argv[0]).parent_path().parent_path() / "{{__TOP_MODULE_NAME__}}.xscfg";
        std::unique_ptr<cfgdb::Database> input;
        if (std::filesystem::exists(path)) input = std::make_unique<cfgdb::Database>(path.string());
        cfgdb::Writer writer(input ? input->model() : "");
        if (input) writer.copy(*input, "mvs");
        for (const auto &var : all_vars) writer.add('v', var.name, var.value());
        if (input) {
            {% for id in __NATIVE_TYPE_IDS__ %}
            if (auto node = input->get('t', {{id}})) writer.add('t', {{id}}, *node);
            {% endfor %}
            const auto lookup = build_var_lookup();
            input->each('q', [&](const std::string &name, const cfgdb::Value &raw) {
                using V = cfgdb::Value;
                auto node = raw;
                const auto kind = node.object.at("kind").text;
                if (kind == "direct" || kind == "projection") {
                    const auto source = node.object.at("source").text;
                    auto var = resolve_source(lookup, source);
                    if (var.name.empty() || !var.width) {
                        node.object["kind"] = V::str("unavailable");
                    } else if (kind == "direct" && node.object.at("rtl_width").number == uint64_t(var.width)) {
                        auto own = lookup.find(name);
                        if (own != lookup.end()) var = own->second;
                        else if (var.name != name) { var.layout.clear(); var.groups.clear(); }
                        writer.add('v', name, var.value());
                        return;
                    } else {
                        node.object["kind"] = V::str("projection");
                        if (node.object.at("expr").text.empty()) node.object["expr"] = V::str(source);
                    }
                } else if (kind == "expr" && node.object.at("deps").array.empty())
                    node.object["kind"] = V::str("unavailable");
                writer.add('s', name, node);
            });
        }
        writer.write(path.string());
        return 0;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n'; return 1;
    }
}
