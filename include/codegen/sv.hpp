#pragma once

#include <bits/stdc++.h>
#include "picker.hpp"
#include "parser/sv.hpp"

namespace picker { namespace codegen {
    void gen_sv_param(nlohmann::json &global_render_data,
                      const std::vector<sv_signal_define> &external_pin,
                      const std::vector<sv_signal_define> &internal_signal,
                      const std::string &wave_file_name,
                      const std::string &simulator);
}} // namespace picker::codegen