#pragma once

#include <bits/stdc++.h>
#include "cxxopts.hpp"
#include "parser/sv.hpp"
#include "codegen/inja.hpp"
#include "codegen/json.hpp"

namespace mcv { namespace codegen {
    void set_sv_param(nlohmann::json &global_render_data,
                      const std::vector<sv_signal_define> &external_pin,
                      const std::vector<sv_signal_define> &internal_signal);
}} // namespace mcv::codegen