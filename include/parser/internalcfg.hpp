#pragma once

#include <bits/stdc++.h>
#include "picker.hpp"
#include "parser/sv.hpp"
#include "yaml-cpp/yaml.h"

namespace picker { namespace parser {
    std::vector<sv_signal_define> internal(std::string internal_pin_filename);
    int internal(cxxopts::ParseResult opts,
                 std::vector<sv_signal_define> &internal_pin,
                 nlohmann::json &sync_opts);
    void recursive_parse(YAML::Node node,
                         std::vector<sv_signal_define> &pin_list,
                         std::string prefix = "");

}; } // namespace picker::parser