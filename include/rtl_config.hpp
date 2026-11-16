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

    // A macro value travels through three layers that all interpret their own
    // characters: the RTL frontend argv, the generated Makefile, and CMake's
    // separate_arguments(NATIVE_COMMAND). Reject characters that any of them
    // would consume or reinterpret instead of silently changing the value.
    inline std::string describe_macro_value_character(char c)
    {
        switch (c) {
        case ' ': return "a space";
        case '\t': return "a tab";
        case '\n': return "a newline";
        case '\r': return "a carriage return";
        case '\f': return "a form feed";
        case '\v': return "a vertical tab";
        default: return std::string("'") + c + "'";
        }
    }

    // Single quotes are legal in SystemVerilog literals and casts. Keep those
    // usable while rejecting stray quotes, which CMake's argument splitting
    // would strip (turning 1'b0 into 1b0, or 'foo' into foo).
    inline bool is_systemverilog_apostrophe(std::string_view value, size_t position)
    {
        const size_t next = position + 1;
        if (next >= value.size()) { return false; }
        const char c = value[next];
        if (c == '{') { return true; } // assignment pattern: '{1, 2}
        if (c == '(') { // cast: int'(...), 8'(...), my_type'(...)
            if (position == 0) { return false; }
            const char previous = value[position - 1];
            return std::isalnum(static_cast<unsigned char>(previous)) || previous == '_' || previous == ']';
        }
        size_t base = next;
        if (c == 's' || c == 'S') { base = next + 1; }
        if (base >= value.size()) { return false; }
        const char b = value[base];
        if (b == 'b' || b == 'B' || b == 'o' || b == 'O' || b == 'd' || b == 'D' || b == 'h' || b == 'H') {
            return true; // sized or unsized based literal: 1'b0, 32'hdeadbeef, 4'sb1010
        }
        return c == '0' || c == '1' || c == 'x' || c == 'X' || c == 'z' || c == 'Z'; // '0, '1, 'x, 'z
    }

    // Returns an empty string when VALUE can be forwarded unchanged, otherwise
    // a diagnostic naming the macro, the offending character, the layer that
    // would corrupt it, and a usable alternative.
    inline std::string macro_value_error(const std::string &macro, const std::string &value)
    {
        for (size_t i = 0; i < value.size(); ++i) {
            const char c = value[i];
            std::string reason;
            switch (c) {
            case '$':
                reason = "the generated Makefile expands '$' while parsing, so '$(shell ...)' would run a "
                         "command during the build";
                break;
            case ';':
                reason = "CMake treats ';' as an argument-list separator, so the value would be split";
                break;
            case '"':
                reason = "CMake consumes double quotes while splitting native arguments";
                break;
            case '`':
                reason = "backticks are shell metacharacters and are not safe in generated build files";
                break;
            case '\\':
                reason = "backslashes are treated as escapes while splitting native arguments";
                break;
            case '#':
                reason = "the generated Makefile treats '#' as the start of a comment";
                break;
            case '\'':
                if (is_systemverilog_apostrophe(value, i)) { continue; }
                reason = "a single quote outside a SystemVerilog literal is consumed as quoting by CMake";
                break;
            default:
                if (std::isspace(static_cast<unsigned char>(c))) {
                    reason = "whitespace separates simulator arguments, so the value would be split";
                }
                break;
            }
            if (reason.empty()) { continue; }
            return "macro '" + macro + "' contains " + describe_macro_value_character(c) + ": " + reason +
                   ". Use a single-token literal; if the value needs spaces or shell characters, put "
                   "+define+" + macro + "=VALUE in a filelist and pass it with --fs instead";
        }
        return {};
    }

    // The generated Makefile stores the flag string in a make variable, then
    // CMake splits that environment value with separate_arguments(NATIVE_COMMAND),
    // which recognizes quoting and strips an unescaped apostrophe. Escape the
    // apostrophes of SystemVerilog literals so 1'b0 arrives intact.
    inline std::string escape_vflag_apostrophes(const std::string &argument)
    {
        std::string escaped;
        escaped.reserve(argument.size());
        for (char c : argument) {
            if (c == '\'') { escaped += '\\'; }
            escaped += c;
        }
        return escaped;
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
            if (define.has_value) {
                const auto error = macro_value_error(define.name, define.value);
                if (!error.empty()) { PK_FATAL("%s '%s': %s", option.c_str(), entry.c_str(), error.c_str()); }
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
