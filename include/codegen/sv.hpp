#pragma once

#include "picker.hpp"
#include "parser/sv.hpp"

namespace picker { namespace codegen {
    std::vector<picker::sv_signal_define> gen_sv_param(nlohmann::json &global_render_data,
                                                       const std::vector<picker::sv_module_define> &sv_module_result,
                                                       const std::vector<picker::sv_signal_define> &internal_signal,
                                                       nlohmann::json &signal_tree_json,
                                                       const std::string &wave_file_name, const std::string &simulator, SignalAccessType rw_type);
}} // namespace picker::codegen
