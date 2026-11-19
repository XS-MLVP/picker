#include "common.hpp"

namespace picker { namespace parser { namespace verilator {

namespace {

// Type normalization is the only version-dependent step applied to scanned
// lines, and only the 5.054 grammar changes it: it removes the generated `::`
// qualifier. Everything older normalizes identically, so the declarations alone
// identify which grammar produced them. Deriving the format here keeps
// `processDeclarations` independent of any earlier call or call order.
detail::root_format declaration_format(const std::vector<std::string> &declarations)
{
    for (const auto &declaration : declarations) {
        if (picker::trim(declaration).rfind("::", 0) == 0) return detail::root_format::v5_054;
    }
    return detail::root_format::v5_026;
}

} // namespace

std::vector<std::string> readVarDeclarations(const std::string &filename)
{
    const auto format = detail::detect_root_format(filename);
    auto declarations = detail::read_root_declarations(filename, format);
    if (declarations.empty()) {
        // A header without a state section marker still exposes its members
        // through the "Data" fallback of the older grammar. The 5.054 prelude is
        // only skipped there, so try that first, then the original scan.
        const auto other = format == detail::root_format::v5_054
                               ? detail::root_format::v5_026
                               : detail::root_format::v5_054;
        declarations = detail::read_root_declarations(filename, other);
    }
    return declarations;
}

std::vector<cpp_variableInfo> processDeclarations(const std::vector<std::string> &declarations)
{
    return detail::parse_root_declarations(declarations, declaration_format(declarations));
}

} } } // namespace picker::parser::verilator
