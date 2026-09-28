#include "common.hpp"

namespace picker { namespace parser { namespace verilator { namespace detail {

namespace {

bool parse_decl(const std::string &statement, const std::vector<std::string> &args,
                trace_event &event)
{
    std::smatch match;
    if (!std::regex_search(statement, match, std::regex(R"(VL_TRACE_DECL_([A-Z_]+)\s*\()"))) {
        return false;
    }
    if (args.size() < 4 || !parse_slot_arg(args[1], event.slot)) return false;

    auto kind = match[1].str();
    event.kind = trace_event_kind::declaration;
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

} // namespace

std::vector<trace_event> parse_v5_048(const std::vector<std::string> &traceFiles)
{
    std::vector<trace_event> events;
    for (const auto &traceFile : traceFiles) {
        std::vector<std::string> scope;
        int oldpBaseShift = 0;
        for (auto statement : split_statements(read_file(traceFile))) {
            statement = picker::trim(statement);
            if (statement.empty()) continue;

            std::smatch baseMatch;
            if (std::regex_search(statement, baseMatch,
                                  std::regex(R"(bufp->oldp\(vlSymsp->__Vm_baseCode(?:\s*\+\s*(\d+))?\))"))) {
                oldpBaseShift = baseMatch[1].matched ? std::stoi(baseMatch[1]) : 0;
                continue;
            }

            std::vector<std::string> args;
            if (unwrap_call_args(statement, "VL_TRACE_PUSH_PREFIX", args)) {
                if (args.size() >= 2) scope.push_back(strip_quotes(args[1]));
                continue;
            }
            if (statement.find("VL_TRACE_POP_PREFIX") != std::string::npos) {
                if (!scope.empty()) scope.pop_back();
                continue;
            }

            if (statement.find("VL_TRACE_DECL_") != std::string::npos
                && unwrap_call_args(statement, "VL_TRACE_DECL_", args)) {
                trace_event event;
                if (parse_decl(statement, args, event)) {
                    event.name = join_scope(scope, event.name);
                    events.push_back(std::move(event));
                }
                continue;
            }

            bool isFull = statement.find("bufp->full") != std::string::npos;
            bool isChange = statement.find("bufp->chg") != std::string::npos;
            if (!isFull && !isChange) continue;
            auto marker = isFull ? "bufp->full" : "bufp->chg";
            if (!unwrap_call_args(statement, marker, args) || args.size() < 2) continue;

            int slot = -1;
            if (!parse_slot_arg(args[0], slot)) continue;
            trace_event event;
            event.kind = trace_event_kind::source;
            event.slot = slot + oldpBaseShift;
            event.raw_expr = args[1];
            event.is_full = isFull;
            events.push_back(std::move(event));
        }
    }
    return events;
}

} } } } // namespace picker::parser::verilator::detail
