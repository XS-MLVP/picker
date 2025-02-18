#include <bits/stdc++.h>
#include "V{{__TOP_MODULE_NAME__}}.h"
#include "V{{__TOP_MODULE_NAME__}}___024root.h"

std::map<std::string, int> size_map;

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

#define SET_SIZE(type) size_map[#type] = sizeof(type)

void verilator_type_size()
{
    SET_SIZE(CData);
    SET_SIZE(SData);
    SET_SIZE(IData);
    SET_SIZE(WData);
    SET_SIZE(QData);
}

void render_varible_info()
{
    typedef V{{__TOP_MODULE_NAME__}}___024root BaseType;
    V{{__TOP_MODULE_NAME__}} *vptr = new V{{__TOP_MODULE_NAME__}};
    BaseType &base                 = *(vptr->rootp);
    printf("variables:\n");

    // {{__yaml_varible_info__}}

    return;
}

int main()
{
    verilator_type_size();
    render_varible_info();
    return 0;
}