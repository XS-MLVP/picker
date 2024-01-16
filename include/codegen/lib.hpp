#pragma once

#include <bits/stdc++.h>
#include "picker.hpp"
#include "parser/sv.hpp"

namespace picker { namespace codegen {

    /// @brief render all files in src_dir to dst_dir recursively
    /// @param src_dir
    /// @param dst_dir
    /// @param data
    /// @param env
    void recursive_render(std::string &src_dir, std::string &dst_dir,
                          nlohmann::json &data, inja::Environment &env);

    /// @brief render all svdpi files, build related files and copy other files
    /// @param opts
    /// @param sync_opts
    void lib(cxxopts::ParseResult opts, nlohmann::json &sync_opts,
             const std::vector<sv_signal_define> &external_pin,
             const std::vector<sv_signal_define> &internal_pin);
}} // namespace picker::codegen