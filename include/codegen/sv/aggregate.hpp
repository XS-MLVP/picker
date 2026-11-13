#pragma once

#include "type.hpp"
#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace picker::codegen::sv {

struct AggregatePortRender {
    std::string logic;
    std::string wire;
    std::string dpi_export;
    std::string dpi_implementation;
    // Lowered binding metadata; the generated SV retains aggregate storage.
    std::vector<picker::sv_signal_define> pins;
};

AggregatePortRender render_aggregate_port(
    const picker::sv_signal_define &pin, const std::string &name,
    const std::string &instance, SignalAccessType access,
    const std::set<std::string> &reserved, std::size_t &serial);

} // namespace picker::codegen::sv
