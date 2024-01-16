#pragma once

#include <bits/stdc++.h>
#include "picker.hpp"
#include "parser/sv.hpp"

namespace picker { namespace codegen {
    void python(const cxxopts::ParseResult &opts, nlohmann::json &sync_opts,
                std::vector<sv_signal_define> external_pin,
                std::vector<sv_signal_define> internal_signal);
}} // namespace picker::codegen