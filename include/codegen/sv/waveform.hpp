#pragma once

#include "json.hpp"
#include <string>

namespace picker::codegen::sv {

void render_sv_waveform(const std::string &simulator, const std::string &wave_file_name,
                        nlohmann::json &data);

} // namespace picker::codegen::sv
