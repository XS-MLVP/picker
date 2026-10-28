#pragma once

#include <string>
#include <vector>

namespace picker { namespace parser { namespace verilator { namespace detail {

struct verilator_version {
    int major = 0;
    int minor = 0;
    int patch = 0;
    std::string executable;

    bool known() const { return major > 0; }
    bool at_least(int required_major, int required_minor) const;
};

verilator_version detect_version(const std::string &rootHeaderFile);

} } } } // namespace picker::parser::verilator::detail
