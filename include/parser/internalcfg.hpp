#pragma once

#include <bits/stdc++.h>
#include "sv.hpp"
#include "cxxopts.hpp"
#include "mcv.hpp"
#include "codegen/json.hpp"
#include "yaml-cpp/yaml.h"

namespace mcv { namespace parser {
    int internal(cxxopts::ParseResult opts,
                 std::vector<sv_signal_define> &internal_pin);
    std::vector<sv_signal_define> internal(std::string internal_pin_filename);
    void recursive_parse(YAML::Node node,
                         std::vector<sv_signal_define> &pin_list,
                         std::string prefix = "");

}; } // namespace mcv::parser