#pragma once

#include <cctype>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "picker.hpp"

namespace picker { namespace rtl {

    // A preprocessor macro requested on the picker command line. Macros are the
    // one setting that must reach both the RTL frontend and the simulator, so
    // they are kept symbolic instead of being evaluated like parameters.
    struct macro_define {
        std::string name;
        std::string value;
        bool has_value = false;
    };

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

    // --rtl-define NAME or NAME=VALUE. A bare name is a defined-but-empty
    // macro, matching -DNAME semantics.
    inline std::vector<macro_define> parse_defines(const std::vector<std::string> &raw, const std::string &option)
    {
        std::vector<macro_define> defines;
        std::unordered_set<std::string> seen;
        for (const auto &entry : raw) {
            const auto text = picker::trim(entry);
            macro_define define;
            const auto equal = text.find('=');
            if (equal == std::string::npos) {
                define.name = text;
            } else {
                define.name  = picker::trim(text.substr(0, equal));
                define.value = picker::trim(text.substr(equal + 1));
                define.has_value = true;
            }
            if (!is_identifier(define.name)) {
                PK_FATAL("%s expects NAME or NAME=VALUE, got '%s'", option.c_str(), entry.c_str());
            }
            if (define.has_value && define.value.empty()) {
                PK_FATAL("%s '%s' has an empty value; omit '=' to define a bare macro", option.c_str(),
                         entry.c_str());
            }
            if (!seen.insert(define.name).second) {
                PK_FATAL("%s defines macro '%s' more than once", option.c_str(), define.name.c_str());
            }
            defines.push_back(std::move(define));
        }
        return defines;
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

    // Argument forms for the RTL frontend.
    inline std::string slang_define_argument(const macro_define &define)
    {
        return "-D" + define.name + (define.has_value ? "=" + define.value : "");
    }

    // Argument form understood by Verilator, VCS and UVS, and by filelists.
    inline std::string simulator_define_argument(const macro_define &define)
    {
        return "+define+" + define.name + (define.has_value ? "=" + define.value : "");
    }

    // Rewrite or drop a filelist token so it no longer defines the macros that
    // the command line already defines. Returns false when nothing is left.
    inline bool filter_filelist_define(const std::string &name, const std::vector<std::string> &argv,
                                       const std::unordered_set<std::string> &overridden,
                                       std::string &replacement)
    {
        if (name == "+define") {
            if (argv.empty()) { return true; }
            const std::string_view text = argv.front();
            constexpr std::string_view prefix = "+define+";
            if (!text.starts_with(prefix)) { return true; }
            std::vector<std::string_view> kept;
            bool removed = false;
            size_t position = prefix.size();
            while (position < text.size()) {
                auto next = text.find('+', position);
                const auto entry = text.substr(position, next - position);
                const auto equal = entry.find('=');
                const auto macro = entry.substr(0, equal);
                if (overridden.find(std::string(macro)) == overridden.end()) {
                    kept.push_back(entry);
                } else {
                    removed = true;
                }
                if (next == std::string_view::npos) { break; }
                position = next + 1;
            }
            if (!removed) { return true; }
            if (kept.empty()) { return false; }
            replacement = std::string(prefix);
            for (size_t i = 0; i < kept.size(); ++i) {
                if (i != 0) { replacement += "+"; }
                replacement += std::string(kept[i]);
            }
            return true;
        }
        // A single-macro token is either redundant or keeps its place untouched.
        if (name == "-D") {
            if (argv.size() < 2) { return true; }
            const auto equal = argv[1].find('=');
            return overridden.find(argv[1].substr(0, equal)) == overridden.end();
        }
        if (name.starts_with("-D")) { return overridden.find(name.substr(2)) == overridden.end(); }
        return true;
    }

}} // namespace picker::rtl
