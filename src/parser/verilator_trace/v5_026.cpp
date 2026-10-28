#include "common.hpp"

namespace picker { namespace parser { namespace verilator { namespace detail {

bool parse_v5_026_declaration(const std::string &statement, const std::vector<std::string> &args,
                             int slot, trace_event &event)
{
    std::smatch match;
    static const std::regex decl_regex(R"(tracep->decl([A-Za-z]+))");
    if (!std::regex_search(statement, match, decl_regex)) return false;
    if (args.size() < 3) return false;

    auto kind = match[1].str();
    event.kind = trace_event_kind::declaration;
    event.slot = slot;
    event.name = join_scope({}, strip_quotes(args[2]));
    if (kind == "Bit") {
        event.type = "CData";
        event.width = 1;
        return true;
    }
    if ((kind == "Array" || kind == "Bus" || kind == "Quad" || kind == "Wide")
        && args.size() >= 2) {
        try {
            int high = std::stoi(picker::trim(args[args.size() - 2]));
            int low = std::stoi(picker::trim(args[args.size() - 1]));
            event.width = high - low + 1;
            event.type = signal_type_from_width(event.width);
            return true;
        } catch (...) {
            return false;
        }
    }
    return true;
}

} } } } // namespace picker::parser::verilator::detail
