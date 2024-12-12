#pragma once

#include <bits/stdc++.h>
#include "picker.hpp"
#include "parser/sv.hpp"

namespace picker { namespace codegen {
    void golang(picker::export_opts &opts, std::vector<picker::sv_signal_define> external_pin,
                std::vector<picker::sv_signal_define> internal_signal, nlohmann::json &signal_tree_json);
}} // namespace picker::codegen