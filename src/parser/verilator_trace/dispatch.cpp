#include "common.hpp"
#include "version.hpp"

namespace picker { namespace parser { namespace verilator {

namespace {

enum class parser_version {
    v5_026,
    v5_048,
};

parser_version select_parser(const std::string *rootHeaderFile,
                             const std::vector<std::string> &traceFiles)
{
    if (rootHeaderFile) {
        auto version = detail::detect_version(*rootHeaderFile);
        if (version.known()) {
            return version.at_least(5, 48) ? parser_version::v5_048
                                           : parser_version::v5_026;
        }
    }
    return detail::trace_uses_v5_048_macros(traceFiles) ? parser_version::v5_048
                                                        : parser_version::v5_026;
}

trace_parse_result dispatch_trace_parser(const std::string *rootHeaderFile,
                                         const std::vector<std::string> &traceFiles,
                                         const std::vector<cpp_variableInfo> &vars)
{
    auto parser = select_parser(rootHeaderFile, traceFiles);
    auto events = parser == parser_version::v5_048
        ? detail::parse_v5_048(traceFiles)
        : detail::parse_v5_026(traceFiles);
    return detail::build_result(events, vars);
}

} // namespace

trace_parse_result processTraceFiles(const std::string &rootHeaderFile,
                                     const std::vector<std::string> &traceFiles,
                                     const std::vector<cpp_variableInfo> &vars)
{
    return dispatch_trace_parser(&rootHeaderFile, traceFiles, vars);
}

trace_parse_result processTraceFiles(const std::vector<std::string> &traceFiles,
                                     const std::vector<cpp_variableInfo> &vars)
{
    return dispatch_trace_parser(nullptr, traceFiles, vars);
}

} } } // namespace picker::parser::verilator
