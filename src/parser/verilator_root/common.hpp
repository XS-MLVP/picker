#pragma once

#include "parser/verilator_root.hpp"

namespace picker { namespace parser { namespace verilator { namespace detail {

// The generated root header has had exactly two grammars:
//
//   v5_026  5.026 .. 5.052  "DESIGN SPECIFIC STATE", plain types, alignas(...)
//                           from 5.050. Unchanged across this whole range even
//                           though the emitted member set differed (trigger
//                           bookkeeping became `VlUnpacked<QData, N>` in 5.048).
//                           Those members are scheduler flags, not RTL signals,
//                           and both spellings are handled by the same rules, so
//                           no version split is needed.
//
//   v5_054  5.054 ..        "TRIVIALLY CONSTRUCTIBLE STATE", `::`-qualified
//                           types, a `TOP LEVEL PORTS` union before the state
//                           section, and decomposed arrays emitted as
//                           `__BRA__N__KET__` components.
//
// Only the emitted text changed; the storage layout is identical. The two
// grammars are distinguished by content, never by PATH, because the header is
// authoritative.
enum class root_format {
    v5_026,
    v5_054,
};

struct root_grammar {
    const char *name;
    // Section that starts the addressable state.
    bool (*is_state_section)(const std::string &line);
    // Translate this version's spelling into the canonical one before matching
    // storage types.
    std::string (*normalize_type)(const std::string &type);
    // 5.054 emits a `TOP LEVEL PORTS` union before the state section. Only the
    // state section is addressable storage, so the prelude is skipped instead
    // of being mistaken for a struct body.
    bool skip_prelude;
};

bool v5_026_state_section(const std::string &line);
std::string v5_026_normalize_type(const std::string &type);
bool v5_054_state_section(const std::string &line);
std::string v5_054_normalize_type(const std::string &type);

const root_grammar &grammar_for(root_format format);
root_format detect_root_format(const std::string &filename);
std::string normalize_root_type(const std::string &type, root_format format);

// `alignas(...)` is a C++ attribute, not part of the storage type. It appeared
// in 5.050 and persists, so every grammar strips it.
std::string strip_type_attributes(const std::string &type);

// Shared scanning and type grammar.
std::vector<std::string> read_root_declarations(const std::string &filename, root_format format);
cpp_variableInfo parse_root_type(const std::string &typeStr, root_format format);
std::vector<cpp_variableInfo> parse_root_declarations(const std::vector<std::string> &declarations,
                                                      root_format format);

} } } } // namespace picker::parser::verilator::detail
