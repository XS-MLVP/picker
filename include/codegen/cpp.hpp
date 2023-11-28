#pragma once

#include <bits/stdc++.h>
#include "cxxopts.hpp"
#include "parser/sv.hpp"
#include "codegen/inja.hpp"
#include "codegen/json.hpp"

namespace mcv {
std::string replace_all(std::string str, const std::string &from,
                        const std::string &to);
std::string capitalize_first_letter(const std::string &str);
namespace codegen {
    void set_cpp_param(nlohmann::json &global_render_data,
                       std::vector<sv_signal_define> external_pin,
                       std::vector<sv_signal_define> internal_signal,
                       const std::string &frequency);
} // namespace codegen
} // namespace mcv