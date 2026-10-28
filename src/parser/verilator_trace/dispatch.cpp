#include "common.hpp"
#include "version.hpp"

namespace picker { namespace parser { namespace verilator {

namespace {

enum class parser_version {
    v5_026,
    v5_048,
};

parser_version select_parser(const std::string *rootHeaderFile,
                             const detail::trace_input &input)
{
    // The generated trace file format is authoritative. PATH may point at a
    // different Verilator version than the one that produced the DUT.
    if (input.modern) return parser_version::v5_048;
    if (input.legacy) return parser_version::v5_026;

    if (rootHeaderFile) {
        auto version = detail::detect_version(*rootHeaderFile);
        if (version.known()) {
            return version.at_least(5, 48) ? parser_version::v5_048
                                           : parser_version::v5_026;
        }
    }
    return parser_version::v5_026;
}

trace_parse_result dispatch_trace_parser(const std::string *rootHeaderFile,
                                         const std::vector<std::string> &traceFiles,
                                         const std::vector<cpp_variableInfo> &vars)
{
    auto input = detail::read_trace_input(traceFiles);
    auto events = detail::parse_trace_events(input,
        select_parser(rootHeaderFile, input) == parser_version::v5_048);
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
