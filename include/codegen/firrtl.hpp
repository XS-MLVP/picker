#pragma once

#include <bits/stdc++.h>
#include "picker.hpp"
#include "parser/firrtl.hpp"

namespace picker { namespace codegen {
    std::vector<picker::sv_signal_define> gen_firrtl_param(nlohmann::json &global_render_data,
                                                       const std::vector<picker::sv_module_define> &sv_module_result,
                                                       const std::vector<picker::sv_signal_define> &internal_signal,
                                                       nlohmann::json &signal_tree_json,
                                                       const std::string &wave_file_name, const std::string &simulator, SignalAccessType rw_type);
}} // namespace picker::codegen
