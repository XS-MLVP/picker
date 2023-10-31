#pragma once

#include <bits/stdc++.h>
#include "cxxopts.hpp"
#include "parser/sv.hpp"
#include "codegen/inja.hpp"
#include "codegen/json.hpp"

namespace mcv
{
    namespace codegen
    {
        void cpp(cxxopts::ParseResult opts, std::vector<sv_pin_member> pin);
    } // namespace codegen
}