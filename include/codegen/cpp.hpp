#pragma once

#include <bits/stdc++.h>
#include "picker.hpp"
#include "parser/sv.hpp"

namespace picker {
std::string replace_all(std::string str, const std::string &from,
                        const std::string &to);

namespace codegen {
    void cpp(const cxxopts::ParseResult &opts, nlohmann::json &sync_opts,
             std::vector<sv_signal_define> external_pin,
             std::vector<sv_signal_define> internal_signal);
} // namespace codegen
} // namespace picker