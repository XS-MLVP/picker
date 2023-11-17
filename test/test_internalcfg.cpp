#include "parser/internalcfg.hpp"

int main()
{
    std::string internal_pin_filename = "../example/Cache/Cache.yaml";
    std::vector<mcv::sv_signal_define> pin_list = mcv::parser::internal(internal_pin_filename);
    for (auto pin : pin_list) {
        std::cout << pin.logic_pin << " " << pin.logic_pin_type << " " << pin.logic_pin_hb << " " << pin.logic_pin_lb << std::endl;
    }
    return 0;
}