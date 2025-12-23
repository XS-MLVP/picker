#pragma once

#include "picker.hpp"

namespace picker { namespace parser {
    // ============================================================================
    // RTL Parsing (export command)
    // ============================================================================

    /// @brief Parse SystemVerilog/Verilog RTL files to extract module definitions
    int sv(picker::export_opts &opts, std::vector<picker::sv_module_define> &external_pin);

    /// @brief Collect .v/.sv files from --fs/--filelist options for module search
    void collect_verilog_from_filelists(const std::vector<std::string> &filelists,
                                        std::vector<std::string> &out);

}; } // namespace picker::parser
