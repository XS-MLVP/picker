#pragma once

#include <bits/stdc++.h>
#include "cxxopts.hpp"

namespace mcv
{
    struct sv_signal_define
    {
        std::string logic_pin;
        std::string logic_pin_type;
        int logic_pin_hb;
        int logic_pin_lb;
    };

    namespace parser
    {
        int sv(cxxopts::ParseResult opts, std::vector<sv_signal_define> &external_pin, std::string &src_module_name);
    };
}