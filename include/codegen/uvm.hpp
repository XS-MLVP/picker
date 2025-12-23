#pragma once

#include "picker.hpp"
#include "parser/sv.hpp"

namespace picker { namespace codegen {
    // ============================================================================
    // UVM Package Generation
    // ============================================================================

    /// @brief Generate UVM package files from prepared template data
    /// @param data Complete template data (from parser::prepare_uvm_package_data)
    /// @param opts Pack options (for directory setup and example generation)
    /// @param package_name Package name
    void generate_uvm_package(const inja::json& data, const pack_opts& opts, const std::string& package_name);

}} // namespace picker::codegen
