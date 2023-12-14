#pragma once

#include <bits/stdc++.h>
#include "mcv.hpp"
#include "parser/sv.hpp"

namespace mcv { namespace codegen {
    void python(const cxxopts::ParseResult &opts, nlohmann::json &sync_opts,
                std::vector<sv_signal_define> external_pin,
                std::vector<sv_signal_define> internal_signal);
}} // namespace mcv::codegen