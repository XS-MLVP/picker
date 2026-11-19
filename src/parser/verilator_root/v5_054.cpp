#include "common.hpp"

namespace picker { namespace parser { namespace verilator { namespace detail {

// Verilator 5.054 and newer. Three textual changes appeared together, and none
// of them alters the storage layout:
//
//   - the state section became "TRIVIALLY CONSTRUCTIBLE STATE"
//   - every generated storage type is globally qualified, including nested
//     template arguments such as `::VlUnpacked<::QData, 1>`
//   - a `TOP LEVEL PORTS` union precedes the state section
//
// Decomposed arrays also gain `__BRA__N__KET__` component members. Those are
// ordinary scalar members of the state section, so they need no special syntax
// here; Picker's logical-name mapping already turns them into RTL indices.
bool v5_054_state_section(const std::string &line)
{
    return line.find("TRIVIALLY CONSTRUCTIBLE STATE") != std::string::npos;
}

std::string v5_054_normalize_type(const std::string &type)
{
    // The qualifier is not part of the storage type, and recursion reaches
    // nested template arguments with their own qualifier.
    auto normalized = strip_type_attributes(type);
    while (normalized.rfind("::", 0) == 0) {
        normalized = strip_type_attributes(normalized.substr(2));
    }
    return normalized;
}

} } } } // namespace picker::parser::verilator::detail
