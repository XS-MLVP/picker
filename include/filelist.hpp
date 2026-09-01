#pragma once
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "picker.hpp"

namespace picker { namespace filelist {

// How picker treats one filelist token.
enum class token_kind {
    path,   // source file or directory, resolved by the caller
    incdir, // +incdir+/-I, values are include directories
    arg,    // compile arg passed through to the simulator untouched
};

struct token {
    token_kind kind = token_kind::path;
    std::string name;                // canonical option name, matches slang --cmd-ignore lookup
    int values      = 0;             // value count following `name`, used by slang --cmd-ignore
    std::string base_dir;            // directory of the filelist this token comes from
    std::vector<std::string> argv;   // path/incdir: raw entries; others: the full arg with its values
};

namespace detail {

    inline bool is_command_file(const std::string &path)
    {
        return path.ends_with(".f") || path.ends_with(".txt");
    }

    inline std::string resolve_against(const std::string &path, const std::string &base_dir)
    {
        std::filesystem::path resolved(path);
        if (!base_dir.empty() && !resolved.is_absolute()) {
            resolved = std::filesystem::path(base_dir) / resolved;
        }
        return resolved.lexically_normal().string();
    }

    inline std::string strip_comment(const std::string &line)
    {
        auto text = picker::trim(line);
        if (text.starts_with("//")) { return ""; }
        return picker::trim(text.substr(0, text.find_first_of("#")));
    }

    // Split a line into tokens, double quotes keep spaces together.
    inline std::vector<std::string> tokenize(const std::string &line)
    {
        std::vector<std::string> tokens;
        std::string current;
        bool started = false, quoted = false;
        for (char c : line) {
            if (c == '"') {
                quoted  = !quoted;
                started = true;
                continue;
            }
            if (!quoted && std::isspace(static_cast<unsigned char>(c))) {
                if (started) { tokens.push_back(current); }
                current.clear();
                started = false;
                continue;
            }
            current += c;
            started = true;
        }
        if (started) { tokens.push_back(current); }
        return tokens;
    }

    inline void expand_file(const std::filesystem::path &path, std::vector<token> &out,
                            std::unordered_set<std::string> &visited);

    inline void expand_command_file(const std::string &entry, const std::string &base_dir, std::vector<token> &out,
                                    std::unordered_set<std::string> &visited)
    {
        auto resolved = resolve_against(entry, base_dir);
        if (!std::filesystem::exists(resolved)) { PK_FATAL("Filelist not found: %s\n", entry.c_str()); }
        expand_file(resolved, out, visited);
    }

    inline void classify_line(const std::string &line, const std::string &base_dir, std::vector<token> &out,
                              std::unordered_set<std::string> &visited)
    {
        auto text = strip_comment(line);
        if (text.empty()) { return; }

        auto tokens = tokenize(text);
        for (size_t i = 0; i < tokens.size(); ++i) {
            const auto tok = tokens[i];
            auto take_value = [&]() {
                if (i + 1 >= tokens.size()) { PK_FATAL("Missing value for '%s' in filelist\n", tok.c_str()); }
                return tokens[++i];
            };

            if (tok.starts_with("+incdir+")) {
                // one token can carry several dirs: +incdir+a+b
                std::vector<std::string> dirs;
                for (size_t pos = strlen("+incdir+"); pos <= tok.size();) {
                    size_t next = tok.find('+', pos);
                    dirs.push_back(tok.substr(pos, next - pos));
                    if (next == std::string::npos) { break; }
                    pos = next + 1;
                }
                out.push_back({token_kind::incdir, "+incdir", 0, base_dir, dirs});
                continue;
            }
            if (tok.starts_with("+")) {
                // plus args match on the part before their first value
                out.push_back({token_kind::arg, tok.substr(0, tok.find('+', 1)), 0, base_dir, {tok}});
                continue;
            }

            if (tok == "-I" || tok == "-y" || tok == "-v" || tok == "-f" || tok == "-F") {
                auto value = take_value();
                if (tok == "-I") {
                    out.push_back({token_kind::incdir, "-I", 0, base_dir, {value}});
                } else if (tok == "-f" || tok == "-F") {
                    // inline nested filelists, the generated one must be self contained
                    expand_command_file(value, base_dir, out, visited);
                } else {
                    out.push_back({token_kind::arg, tok, 1, base_dir, {tok, resolve_against(value, base_dir)}});
                }
                continue;
            }
            if (tok.size() > 2 && (tok.starts_with("-I") || tok.starts_with("-y") || tok.starts_with("-v"))) {
                auto flag  = tok.substr(0, 2);
                auto value = tok.substr(2);
                if (flag == "-I") {
                    out.push_back({token_kind::incdir, "-I", 0, base_dir, {value}});
                } else {
                    out.push_back({token_kind::arg, flag, 0, base_dir, {flag + resolve_against(value, base_dir)}});
                }
                continue;
            }
            if (tok.starts_with("-")) {
                // Options are passed through untouched: their values are none of picker's
                // business, so the rest of the line belongs to them.
                std::vector<std::string> argv(tokens.begin() + i, tokens.end());
                out.push_back({token_kind::arg, tok.substr(0, tok.find('=')), (int)argv.size() - 1, base_dir,
                               argv});
                return;
            }

            out.push_back({token_kind::path, "", 0, base_dir, {tok}});
        }
    }

    inline void expand_file(const std::filesystem::path &path, std::vector<token> &out,
                            std::unordered_set<std::string> &visited)
    {
        auto normalized = std::filesystem::absolute(path).lexically_normal().string();
        if (!visited.insert(normalized).second) {
            PK_FATAL("Filelist includes itself recursively: %s\n", normalized.c_str());
        }

        std::ifstream ifs(path);
        if (!ifs.is_open()) { PK_FATAL("Failed to open filelist: %s\n", normalized.c_str()); }

        auto base_dir = std::filesystem::absolute(path).parent_path().string();
        std::string line;
        while (std::getline(ifs, line)) { classify_line(line, base_dir, out, visited); }
        visited.erase(normalized);
    }

} // namespace detail

/// @brief Expand '--fs/--filelist' inputs into classified tokens, in filelist order.
/// '*.f/*.txt' inputs are read as filelists (nested '-f/-F' entries inlined), any
/// other input is treated as a comma separated list of paths.
inline void expand(const std::vector<std::string> &inputs, std::vector<token> &out)
{
    std::unordered_set<std::string> visited;
    for (const auto &input : inputs) {
        if (detail::is_command_file(input)) {
            detail::expand_command_file(input, "", out, visited);
            continue;
        }
        std::stringstream ss(input);
        std::string item;
        while (std::getline(ss, item, ',')) { detail::classify_line(item, "", out, visited); }
    }
}

}} // namespace picker::filelist
