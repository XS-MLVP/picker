#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "xspcomm/detail/config_db.h"

namespace cfgdb = xspcomm::detail::cfgdb;

{% if __SIMULATOR__ == "verilator" %}
#include "V{{__TOP_MODULE_NAME__}}.h"
#include "V{{__TOP_MODULE_NAME__}}___024root.h"
typedef V{{__TOP_MODULE_NAME__}}___024root BaseType;
{% endif %}

{% if __SIMULATOR__ == "gsim" %}
#include "{{__TOP_MODULE_NAME__}}.h"
typedef S{{__TOP_MODULE_NAME__}} BaseType;
{% endif %}

struct cpp_native_group {
    std::string path;
    int width;
    uint64_t mem_bytes;
    uint64_t offset;
    std::vector<int> dimensions;
    std::vector<uint64_t> strides;
};

struct cpp_variableInfo {
    std::string name;
    std::string type;
    int width       = 0;
    int array_size  = 0;
    uint64_t offset = 0;
    std::vector<int> array_dimensions;
    std::vector<uint64_t> array_strides;
    uint32_t mem_bytes = 0;
    std::string layout;
    std::string view_error;
    std::vector<cpp_native_group> groups;

    cfgdb::Value value() const {
        using V = cfgdb::Value;
        V out; out.kind = V::Object;
        out.object = { {"type", V::str(type)}, {"mem_bytes", V::uint(mem_bytes)},
            {"rtl_width", V::uint(width)}, {"offset", V::uint(offset)}};
        auto array = [](const auto &values) {
            V node; node.kind = V::Array;
            for (auto value : values) node.array.push_back(V::uint(value));
            return node;
        };
        if (array_size) out.object["array_size"] = V::uint(array_size);
        if (!array_dimensions.empty()) {
            out.object["array_dimensions"] = array(array_dimensions);
            out.object["array_strides"] = array(array_strides);
        }
        if (!layout.empty()) out.object["layout_id"] = V::str(layout);
        if (!view_error.empty()) out.object["view_error"] = V::str(view_error);
        if (!groups.empty()) {
            V bindings; bindings.kind = V::Array;
            for (const auto &g : groups) {
                V node; node.kind = V::Object;
                node.object = { {"path", V::str(g.path)}, {"rtl_width", V::uint(g.width)},
                    {"mem_bytes", V::uint(g.mem_bytes)}, {"offset", V::uint(g.offset)},
                    {"array_dimensions", array(g.dimensions)}, {"array_strides", array(g.strides)}};
                bindings.array.push_back(std::move(node));
            }
            out.object["bindings"] = std::move(bindings);
        }
        return out;
    }

};

extern std::vector<cpp_variableInfo> all_vars;
