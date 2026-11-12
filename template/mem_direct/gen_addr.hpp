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

{% if __SIMULATOR__ == "verilator" %}
#include "V{{__TOP_MODULE_NAME__}}.h"
#include "V{{__TOP_MODULE_NAME__}}___024root.h"
typedef V{{__TOP_MODULE_NAME__}}___024root BaseType;
{% endif %}

{% if __SIMULATOR__ == "gsim" %}
#include "{{__TOP_MODULE_NAME__}}.h"
typedef S{{__TOP_MODULE_NAME__}} BaseType;
{% endif %}

struct cpp_variableInfo {
    std::string name;
    std::string type;
    int width       = 0;
    int array_size  = 0;
    uint64_t offset = 0;
    std::vector<int> array_dimensions;
    std::vector<uint64_t> array_strides;
    uint32_t mem_bytes = 0;

    void write_yaml(std::ostream &out) const
    {
        out << "  - name: " << name << "\n";
        out << "    type: " << type << "\n";
        out << "    mem_bytes: " << mem_bytes << "\n";
        out << "    rtl_width: " << width << "\n";
        if (array_size > 0) { out << "    array_size: " << array_size << "\n"; }
        write_array_layout(out, "    ");
        out << "    offset: " << offset << "\n";
    }

    void write_array_layout(std::ostream &out, const std::string &prefix) const
    {
        if (array_dimensions.empty()) { return; }
        out << prefix << "array_dimensions: [";
        for (size_t i = 0; i < array_dimensions.size(); ++i) {
            if (i) { out << ", "; }
            out << array_dimensions[i];
        }
        out << "]\n" << prefix << "array_strides: [";
        for (size_t i = 0; i < array_strides.size(); ++i) {
            if (i) { out << ", "; }
            out << array_strides[i];
        }
        out << "]\n";
    }
};

struct raw_signal_info {
    std::string name;
    std::string kind;
    std::string type;
    uint32_t rtl_width = 0;
    std::string source;
    std::string expr;
    std::string value;
    std::vector<std::string> deps;
};

extern std::vector<cpp_variableInfo> all_vars;
