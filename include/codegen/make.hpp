#pragma once

#include <bits/stdc++.h>
#include "cxxopts.hpp"
#include "parser/sv.hpp"
#include "codegen/inja.hpp"
#include "codegen/json.hpp"

namespace mcv { namespace codegen {
    void set_make_param(nlohmann::json &global_render_data,
                        const std::string &src_module_name,
                        std::string &dst_module_name,
                        const std::string &src_dir, const std::string &dst_dir,
                        const std::string &wave_file_name,
                        const std::string &simulator, const std::string &vflag);
    void render(const cxxopts::ParseResult &opts,
                const std::string &src_module_name,
                const std::vector<sv_signal_define> &external_pin,
                const std::vector<sv_signal_define> &internal_signal);
}} // namespace mcv::codegen