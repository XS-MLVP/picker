#pragma once

#include <bits/stdc++.h>
#include "picker.hpp"
#include "parser/sv.hpp"
#include "parser/firrtl.hpp"

namespace picker { namespace codegen {

    /// @brief render all files in src_dir to dst_dir recursively
    /// @param src_dir
    /// @param dst_dir
    /// @param data
    /// @param env
    void recursive_render(std::string &src_dir, std::string &dst_dir, nlohmann::json &data, inja::Environment &env);

    /// @brief render all svdpi files, build related files and copy other files
    /// @param opts
    /// @param external_pin
    /// @param internal_pin
    std::vector<picker::sv_signal_define> lib(picker::export_opts &opts,
                                              const std::vector<picker::sv_module_define> sv_module_result,
                                              const std::vector<picker::sv_signal_define> &internal_pin,
                                              nlohmann::json &signal_tree_json);

    using native_func = void (*)(picker::export_opts &opts, std::vector<picker::sv_signal_define> external_pin,
                std::vector<picker::sv_signal_define> internal_signal, nlohmann::json &result);

    native_func get_native_signal_processer(picker::export_opts &opts, nlohmann::json &result);
}} // namespace picker::codegen