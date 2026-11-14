#pragma once

#include <cctype>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "picker.hpp"

namespace picker { namespace rtl {

    inline bool is_identifier(std::string_view text)
    {
        if (text.empty()) { return false; }
        const auto first = static_cast<unsigned char>(text.front());
        if (!(std::isalpha(first) || first == '_')) { return false; }
        for (char c : text) {
            const auto ch = static_cast<unsigned char>(c);
            if (!(std::isalnum(ch) || ch == '_')) { return false; }
        }
        return true;
    }

    // --rtl-param NAME=VALUE. Values stay textual until slang evaluates them, so
    // expressions such as $clog2(256) are allowed; the generated wrapper only
    // ever contains the elaborated constant.
    inline std::vector<std::pair<std::string, std::string>>
    parse_params(const std::vector<std::string> &raw, const std::string &option)
    {
        std::vector<std::pair<std::string, std::string>> params;
        std::unordered_set<std::string> seen;
        for (const auto &entry : raw) {
            const auto text  = picker::trim(entry);
            const auto equal = text.find('=');
            if (equal == std::string::npos) {
                PK_FATAL("%s expects NAME=VALUE, got '%s'", option.c_str(), entry.c_str());
            }
            const auto name  = picker::trim(text.substr(0, equal));
            const auto value = picker::trim(text.substr(equal + 1));
            if (name.find('.') != std::string::npos) {
                // slang only treats bare names as top-level CLI overrides; a
                // hierarchical name goes through the defparam path, which marks
                // tops with required parameters as invalid.
                PK_FATAL("%s '%s': hierarchical names are not supported, pass the parameter name only",
                         option.c_str(), entry.c_str());
            }
            if (!is_identifier(name)) {
                PK_FATAL("%s expects NAME=VALUE with an identifier name, got '%s'", option.c_str(), entry.c_str());
            }
            if (value.empty()) {
                PK_FATAL("%s '%s' has an empty value", option.c_str(), entry.c_str());
            }
            if (!seen.insert(name).second) {
                PK_FATAL("%s sets parameter '%s' more than once", option.c_str(), name.c_str());
            }
            params.emplace_back(name, value);
        }
        return params;
    }

}} // namespace picker::rtl
