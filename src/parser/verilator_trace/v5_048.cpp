#include "common.hpp"

namespace picker { namespace parser { namespace verilator { namespace detail {

bool parse_v5_048_declaration(const std::string &statement, const std::vector<std::string> &args,
                             int slot, trace_event &event)
{
    static const std::string marker = "VL_TRACE_DECL_";
    auto declPos = statement.find(marker);
    if (declPos == std::string::npos) return false;
    auto open = statement.find('(', declPos + marker.size());
    if (open == std::string::npos) return false;
    auto kind = picker::trim(statement.substr(declPos + marker.size(),
                                              open - declPos - marker.size()));
    if (args.size() < 4) return false;

    event.kind = trace_event_kind::declaration;
    event.slot = slot;
    event.name = strip_quotes(args[3]);
    if (kind == "BIT" || kind == "EVENT" || kind == "BIT_ARRAY" || kind == "EVENT_ARRAY") {
        event.type = "CData";
        event.width = 1;
        return true;
    }
    if (kind == "BUS" || kind == "QUAD" || kind == "WIDE"
        || kind == "BUS_ARRAY" || kind == "QUAD_ARRAY" || kind == "WIDE_ARRAY") {
        if (args.size() < 2) return false;
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
