#include "gen_addr.hpp"

std::map<std::string, int> size_map;

{{__render_varible_info_sub_define__}}

void render_varible_info()
{
    V{{__TOP_MODULE_NAME__}} *vptr = new V{{__TOP_MODULE_NAME__}};
    BaseType &base                 = *(vptr->rootp);
    printf("variables:\n");

    // {{__render_varible_info_sub__}}

    return;
}

void verilator_type_size()
{
    SET_SIZE(CData);
    SET_SIZE(SData);
    SET_SIZE(IData);
    SET_SIZE(WData);
    SET_SIZE(QData);
}

int main()
{
    verilator_type_size();
    render_varible_info();
    return 0;
}