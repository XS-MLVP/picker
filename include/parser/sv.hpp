#pragma once

#include <bits/stdc++.h>
#include "picker.hpp"

namespace picker {
typedef struct sv_signal_define {
    std::string logic_pin;
    std::string logic_pin_type;
    int logic_pin_hb;
    int logic_pin_lb;
} sv_signal_define;
namespace parser {
    int sv(cxxopts::ParseResult opts,
           std::vector<sv_signal_define> &external_pin,
           nlohmann::json &sync_opts);
};
} // namespace picker