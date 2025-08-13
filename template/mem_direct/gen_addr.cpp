#include "gen_addr.hpp"

std::map<std::string, int> size_map;

{{__render_varible_info_sub_define__}}

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
    printf("variables:\n");

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

int main()
{
    init_type_size();
    render_varible_info();
    return 0;
}