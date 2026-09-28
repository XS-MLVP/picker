#include <cstdio>
#include <fstream>

#include "common.hpp"
#include "version.hpp"

namespace picker { namespace parser { namespace verilator { namespace detail {

namespace {

std::string shell_quote(const std::string &value)
{
    std::string quoted = "'";
    for (char ch : value) {
        if (ch == '\'') quoted += "'\\''";
        else quoted.push_back(ch);
    }
    quoted.push_back('\'');
    return quoted;
}

std::string command_output(const std::string &executable)
{
    auto command = executable.empty() ? "verilator --version"
                                      : shell_quote(executable) + " --version";
    command += " 2>/dev/null";
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) return {};

    std::string output;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) output += buffer;
    pclose(pipe);
    return output;
}

verilator_version parse_version_output(const std::string &output,
                                        const std::string &executable)
{
    verilator_version version;
    version.executable = executable;
    std::smatch match;
    if (!std::regex_search(output, match,
                           std::regex(R"(Verilator\s+([0-9]+)\.([0-9]+)(?:\.([0-9]+))?)"))) {
        return version;
    }
    version.major = std::stoi(match[1]);
    version.minor = std::stoi(match[2]);
    version.patch = match[3].matched ? std::stoi(match[3]) : 0;
    return version;
}

std::string generated_verilator_root(const std::string &rootHeaderFile)
{
    std::filesystem::path rootPath(rootHeaderFile);
    auto fileName = rootPath.filename().string();
    std::smatch match;
    if (!std::regex_match(fileName, match, std::regex(R"(^V(.+)___024root\.h$)"))) return {};

    auto mkPath = rootPath.parent_path() / ("V" + match[1].str() + ".mk");
    std::ifstream mk(mkPath);
    if (!mk.is_open()) return {};

    std::string line;
    std::regex rootRegex(R"(^\s*VERILATOR_ROOT\s*=\s*(.+?)\s*$)");
    while (std::getline(mk, line)) {
        std::smatch rootMatch;
        if (std::regex_match(line, rootMatch, rootRegex)) return picker::trim(rootMatch[1]);
    }
    return {};
}

} // namespace

bool verilator_version::at_least(int required_major, int required_minor) const
{
    return major > required_major || (major == required_major && minor >= required_minor);
}

verilator_version detect_version(const std::string &rootHeaderFile)
{
    auto generatedRoot = generated_verilator_root(rootHeaderFile);
    if (!generatedRoot.empty()) {
        std::filesystem::path root(generatedRoot);
        auto candidate = root.parent_path().parent_path() / "bin" / "verilator";
        if (!std::filesystem::exists(candidate)) {
            return parse_version_output({}, candidate.string());
        }
        return parse_version_output(command_output(candidate.string()), candidate.string());
    }

    return parse_version_output(command_output(""), "verilator");
}

bool trace_uses_v5_048_macros(const std::vector<std::string> &traceFiles)
{
    for (const auto &traceFile : traceFiles) {
        auto content = read_file(traceFile);
        if (content.find("VL_TRACE_DECL_") != std::string::npos
            || content.find("VL_TRACE_PUSH_PREFIX") != std::string::npos) {
            return true;
        }
    }
    return false;
}

} } } } // namespace picker::parser::verilator::detail
