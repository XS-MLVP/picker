#include "codegen/sv/aggregate.hpp"
#include "codegen/sv/templates.hpp"
#include "picker.hpp"

#include <cstdlib>
#include <functional>
#include <limits>

namespace picker::codegen::sv {

AggregatePortRender render_aggregate_port(
    const picker::sv_signal_define &pin, const std::string &name,
    const std::string &instance, SignalAccessType access,
    const std::set<std::string> &reserved, std::size_t &serial)
{
    using J = nlohmann::json;
    AggregatePortRender rendered;
    const auto layout = J::parse(*pin.type_layout);
    const bool packed = layout["packed"].get<bool>();
    const auto length = pin.logic_pin_hb == -1 ? "" :
        "[" + std::to_string(pin.logic_pin_hb) + ":0]";
    // A packed value is connection-compatible with its flat vector.
    // Unpacked values retain the port's exact elaborated type, including
    // named/anonymous structs, without synthesizing field bridge nets.
    const auto declaration = packed ? pin.unpacked_element_type + " " + length + " " + name + ";\n" :
        "type(" + instance + "." + pin.logic_pin + ") " + name + ";\n";
    rendered.logic += "  " + declaration;
    rendered.wire += "  " + declaration;

    inja::Environment env;
    auto dpi = [&](const picker::sv_signal_define &leaf, const std::string &expression,
                   uint64_t size) {
        if (access != SignalAccessType::DPI) return;
        J data{{"pin_func_name", leaf.array_name.empty() ? leaf.logic_pin : leaf.array_name},
               {"__LIB_DPI_FUNC_NAME_HASH__", std::string(lib_random_hash)},
               {"logic_pin", expression}, {"array_size", size},
               {"logic_pin_length", leaf.logic_pin_hb == -1 ? "" :
                   "[" + std::to_string(leaf.logic_pin_hb) + ":0]"}};
        rendered.dpi_export += env.render(templates::dpi_get_export, data);
        auto getter = leaf.array_name.empty() ? templates::dpi_get_impl : templates::dpi_array_get_impl;
        auto setter = leaf.array_name.empty() ? templates::dpi_set_impl : templates::dpi_array_set_impl;
        // Preserve the exact elaborated field type, including enums,
        // signing and two-state conversion, at the SV assignment.
        setter = str_replace_all(setter, "=value;", "=type({{logic_pin}})'(value);");
        rendered.dpi_implementation += env.render(getter, data);
        if (leaf.logic_pin_type != "output") {
            rendered.dpi_export += env.render(templates::dpi_set_export, data);
            rendered.dpi_implementation += env.render(setter, data);
        }
    };

    if (packed) {
        auto leaf = pin;
        leaf.logic_pin = name;
        leaf.aggregate_name = name;
        leaf.signal_path = name;
        dpi(leaf, name, 1);
        rendered.pins.push_back(std::move(leaf));
        return rendered;
    }

    auto unique = [&](const std::string &prefix) {
        std::string candidate;
        do { candidate = prefix + std::to_string(serial++); } while (reserved.contains(candidate));
        return candidate;
    };
    // Walk the type (not all values) to group a homogeneous field across
    // every enclosing unpacked dimension. Indexed DPI and native bases
    // therefore scale with field types, not array element count.
    std::function<void(const J &, std::string, std::string, std::vector<std::pair<int, int>>,
                       std::vector<std::string>)> group;
    group = [&](const J &type, std::string expression, std::string native,
                std::vector<std::pair<int, int>> dimensions, std::vector<std::string> strides) {
        if (type["packed"].get<bool>()) {
            auto leaf = pin;
            const int width = type["width"].get<int>();
            leaf.logic_pin_hb = width ? width - 1 : -1;
            leaf.logic_pin_lb = 0;
            leaf.aggregate_name = name;
            leaf.array_name = unique("_picker_binding_");
            leaf.binding_native = native;
            leaf.binding_strides = strides;
            if (dimensions.empty()) {
                dimensions.emplace_back(0, 0);
                leaf.binding_strides.push_back(native); // singleton; stride is never used
            }
            leaf.unpacked_dimensions = dimensions;
            uint64_t size = 1;
            std::vector<uint64_t> flat_strides(dimensions.size());
            for (size_t d = flat_strides.size(); d-- > 0;) {
                flat_strides[d] = size;
                const auto [left, right] = dimensions[d];
                const uint64_t count = uint64_t(std::abs(int64_t(left) - right)) + 1;
                if (size > uint64_t(std::numeric_limits<int>::max()) / count)
                    PK_FATAL("Aggregate %s exceeds the DPI int index range", name.c_str());
                size *= count;
            }
            auto selected = expression;
            for (size_t d = 0; d < dimensions.size(); ++d) {
                const auto [left, right] = dimensions[d];
                const auto count = std::abs(int64_t(left) - right) + 1;
                const auto index = std::to_string(left) + (count == 1 ? "" :
                    std::string(left <= right ? " + " : " - ") + "((index / " +
                    std::to_string(flat_strides[d]) + ") % " + std::to_string(count) + ")");
                selected = str_replace_all(selected, "@" + std::to_string(d) + "@", index);
            }
            dpi(leaf, selected, size);
            std::function<void(size_t, std::string)> elements = [&](size_t d, std::string path) {
                if (d == dimensions.size()) {
                    leaf.logic_pin = unique("_picker_leaf_");
                    leaf.signal_path = path;
                    rendered.pins.push_back(leaf);
                    return;
                }
                const auto [left, right] = dimensions[d];
                for (int64_t i = left;; i += left <= right ? 1 : -1) {
                    leaf.array_indices.push_back(int(i));
                    elements(d + 1, str_replace_all(path, "@" + std::to_string(d) + "@", std::to_string(i)));
                    leaf.array_indices.pop_back();
                    if (i == right) break;
                }
            };
            elements(0, expression);
        } else if (type["kind"] == "struct") {
            for (const auto &field : type["fields"]) {
                const auto member = "." + field["name"].get<std::string>();
                // Verilator's generated unpacked struct members use __PVT__ names;
                // offsets/strides still come from C++ expressions against the actual type.
                group(field["type"], expression + member,
                      native + ".__PVT__" + field["name"].get<std::string>(), dimensions, strides);
            }
        } else {
            const int left = type["left"].get<int>(), right = type["right"].get<int>();
            const auto token = "@" + std::to_string(dimensions.size()) + "@";
            dimensions.emplace_back(left, right);
            native += "[0]";
            strides.push_back(native);
            group(type["element"], expression + "[" + token + "]", native, dimensions, strides);
        }
    };
    group(layout, name, name, {}, {});
    return rendered;
}

} // namespace picker::codegen::sv
