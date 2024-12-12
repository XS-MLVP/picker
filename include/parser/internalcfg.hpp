#pragma once

#include <bits/stdc++.h>
#include "picker.hpp"
#include "parser/sv.hpp"
#include "yaml-cpp/yaml.h"

namespace picker { namespace parser {
    std::vector<picker::sv_signal_define> internal(std::string internal_pin_filename);
    int internal(picker::export_opts &opts, std::vector<picker::sv_signal_define> &internal_pin);
    void recursive_parse(YAML::Node node, std::vector<picker::sv_signal_define> &pin_list, std::string prefix = "");

}; } // namespace picker::parser