#pragma once

#include <bits/stdc++.h>
#include "cxxopts.hpp"

namespace mcv
{
    struct sv_pin_member
    {
        std::string logic_pin;
        std::string logic_pin_type;
        int logic_pin_length;
    };

    namespace parser
    {
        std::vector<sv_pin_member> sv(cxxopts::ParseResult opts);
    };
}