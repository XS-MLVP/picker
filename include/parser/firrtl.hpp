#pragma once

#include "picker.hpp"

namespace picker { namespace parser {
    int firrtl(picker::export_opts &opts, std::vector<picker::sv_module_define> &external_pin);
}; } // namespace picker::parser
