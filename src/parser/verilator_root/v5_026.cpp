#include "common.hpp"

namespace picker { namespace parser { namespace verilator { namespace detail {

// Verilator before 5.054: the original generated header spelling. This covers
// 5.026 through 5.052 in one grammar, because the emitted text changed inside
// that range without changing anything the parser acts on. In particular 5.048
// switched trigger bookkeeping from `VlTriggerVec<N>` to `VlUnpacked<QData, N>`
// and 5.050 added `alignas(...)`:
//
//   - `VlTriggerVec` has no operator[] and keeps its flags in a private array,
//     so it cannot be addressed. The shared type grammar does not recognize the
//     name and drops those members, which is correct: they are Verilator
//     scheduler flags, not RTL signals. The later array spelling is exported
//     normally by the generic array path.
//   - `alignas(...)` is a C++ attribute, stripped by `strip_type_attributes`.
//
// Other 5.026-only differences (`: public VerilatedModule`, `const vlSymsp`,
// the `v__name` constructor parameter) are not state members, so member
// scanning never sees them.
bool v5_026_state_section(const std::string &line)
{
    return line.find("DESIGN SPECIFIC STATE") != std::string::npos;
}

std::string v5_026_normalize_type(const std::string &type)
{
    return strip_type_attributes(type);
}

} } } } // namespace picker::parser::verilator::detail
