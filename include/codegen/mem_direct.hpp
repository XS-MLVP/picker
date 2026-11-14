#pragma once
#include "picker.hpp"

namespace picker { namespace codegen {

    std::string replace_dot(const std::string &varName);
    void render_md_addr_generator(const std::vector<cpp_variableInfo> &varibles, picker::export_opts &opts,
                                  const nlohmann::json &types = nlohmann::json::object());
    
}}; // namespace picker::codegen
