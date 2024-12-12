#pragma once

#include <bits/stdc++.h>
#include "picker.hpp"
#include "parser/sv.hpp"

namespace picker {
std::string replace_all(std::string str, const std::string &from, const std::string &to);

namespace codegen {
    void cpp(picker::export_opts &opts, std::vector<picker::sv_signal_define> external_pin,
             std::vector<picker::sv_signal_define> internal_signal, nlohmann::json &signal_tree_json);
} // namespace codegen
} // namespace picker