#pragma once

#include "picker.hpp"

namespace picker { namespace parser {

    int parse_sv_with_slang(const picker::export_opts &opts,
                            const std::map<std::string, int> &requested_module_counts,
                            std::vector<std::string> &resolved_module_names,
                            std::vector<picker::sv_module_define> &sv_module_result);

}} // namespace picker::parser
