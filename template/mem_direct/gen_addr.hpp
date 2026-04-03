#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
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

#define SET_SIZE(type) size_map[#type] = sizeof(type)
extern std::map<std::string, int> size_map;

struct cpp_variableInfo {
    std::string name;
    std::string type;
    int width       = 0;
    int array_size  = 0;
    uint64_t offset = 0;

    int mem_bytes() const
    {
        if (size_map.find(type) == size_map.end()) {
        {% if __SIMULATOR__ == "verilator" %}
            if (type.rfind("VlWide<", 0) == 0) {
                return stoi(type.substr(7, type.size() - 8)) * 4;
            }
        {% endif %}
        {% if __SIMULATOR__ == "gsim" %}
            return -1;
        {% endif %}
        }
        return size_map[type];
    }

    void write_yaml(std::ostream &out) const
    {
        out << "  - name: " << name << "\n";
        out << "    type: " << type << "\n";
        out << "    mem_bytes: " << mem_bytes() << "\n";
        out << "    rtl_width: " << width << "\n";
        if (array_size > 0) { out << "    array_size: " << array_size << "\n"; }
        out << "    offset: " << offset << "\n";
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

void init_type_size();
