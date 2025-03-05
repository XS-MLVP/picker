#pragma once
#include <bits/stdc++.h>
#include "V{{__TOP_MODULE_NAME__}}.h"
#include "V{{__TOP_MODULE_NAME__}}___024root.h"

#define SET_SIZE(type) size_map[#type] = sizeof(type)

extern std::map<std::string, int> size_map;
typedef V{{__TOP_MODULE_NAME__}}___024root BaseType;

struct VariableInfo {
    std::string name;
    std::string type;
    int width      = 0;
    int array_size = 0;
    int64_t offset = 0;

    void write_yaml()
    {
        printf("  - name: %s\n", name.c_str());
        printf("    type: %s\n", type.c_str());
        if (size_map.find(type) == size_map.end())
            // type is "VlWide<???>", res as ??? * 4
            printf("    mem_bytes: %d\n", stoi(type.substr(7, type.size() - 8)) * 4);                                                   
        else
            printf("    mem_bytes: %d\n", size_map[type]);
        printf("    rtl_width: %d\n", width);
        if (array_size > 0) { printf("    array_size: %d\n", array_size); }
        printf("    offset: %ld\n", offset);
    }
};

void verilator_type_size();
