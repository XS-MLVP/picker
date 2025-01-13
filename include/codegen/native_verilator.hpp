
#pragma once
#include "picker.hpp"

namespace picker { namespace native_verilator {
    void native_signal(picker::export_opts &opts, std::vector<picker::sv_signal_define> external_pin,
                std::vector<picker::sv_signal_define> internal_signal, nlohmann::json &result);
}}