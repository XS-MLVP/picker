#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace {

struct Options {
    std::string database;
    std::string kind = "line";
    std::string module;
    std::string instance;
};

struct RawEntry {
    std::string file;
    std::set<int> lines;
    int column = 0;
    std::string module;
    std::string instance;
    std::string object;
    int64_t count = 0;
};

struct LineRecord {
    std::string file;
    int line = 0;
    int column = 0;
    std::string module;
    std::string instance;
    std::string object;
    int64_t count = 0;
};

void usage(const char *argv0)
{
    std::cerr << "Usage: " << argv0
              << " --database <coverage.dat> [--kind line] "
              << "[--module <substr>] [--instance <prefix>]\n";
}

bool take_arg(int argc, char **argv, int &index, std::string &out)
{
    if (index + 1 >= argc) return false;
    out = argv[++index];
    return true;
}

bool parse_args(int argc, char **argv, Options &opt)
{
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--database") {
            if (!take_arg(argc, argv, i, opt.database)) return false;
        } else if (arg == "--kind") {
            if (!take_arg(argc, argv, i, opt.kind)) return false;
        } else if (arg == "--module") {
            if (!take_arg(argc, argv, i, opt.module)) return false;
        } else if (arg == "--instance") {
            if (!take_arg(argc, argv, i, opt.instance)) return false;
        } else if (arg == "-h" || arg == "--help") {
            usage(argv[0]);
            std::exit(0);
        } else {
            std::cerr << "unknown argument: " << arg << "\n";
            return false;
        }
    }
    return !opt.database.empty();
}

std::string lower_copy(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string json_escape(const std::string &value)
{
    std::string out;
    for (unsigned char c : value) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c < 0x20) {
                char buffer[7];
                std::snprintf(buffer, sizeof(buffer), "\\u%04x", c);
                out += buffer;
            } else {
                out += static_cast<char>(c);
            }
        }
    }
    return out;
}

std::string q(const std::string &value)
{
    return "\"" + json_escape(value) + "\"";
}


std::string unescape_control_markers(std::string value)
{
    for (size_t pos = 0; (pos = value.find("\\001", pos)) != std::string::npos;) {
        value.replace(pos, 4, 1, '\x01');
    }
    for (size_t pos = 0; (pos = value.find("\\002", pos)) != std::string::npos;) {
        value.replace(pos, 4, 1, '\x02');
    }
    if (value.size() >= 2 && (value.front() == '\'' || value.front() == '"') &&
        value.back() == value.front()) {
        value = value.substr(1, value.size() - 2);
    }
    return value;
}

std::map<std::string, std::string> parse_fields(const std::string &raw)
{
    std::map<std::string, std::string> fields;
    for (size_t pos = 0; pos < raw.size();) {
        if (raw[pos++] != '\x01') continue;
        const size_t key_end = raw.find('\x02', pos);
        if (key_end == std::string::npos) break;
        const std::string key = raw.substr(pos, key_end - pos);
        pos = key_end + 1;
        const size_t value_end = raw.find('\x01', pos);
        fields[key] = raw.substr(pos, value_end == std::string::npos ? std::string::npos : value_end - pos);
        if (value_end == std::string::npos) break;
        pos = value_end;
    }
    return fields;
}

bool parse_int(const std::string &text, int &value)
{
    try {
        size_t used = 0;
        value = std::stoi(text, &used);
        return used == text.size();
    } catch (...) {
        return false;
    }
}

bool parse_ranges(const std::string &text, std::set<int> &lines, std::string &error)
{
    std::stringstream stream(text);
    std::string item;
    while (std::getline(stream, item, ',')) {
        if (item.empty()) continue;
        const size_t dash = item.find('-');
        int first = 0;
        int last = 0;
        if (dash == std::string::npos) {
            if (!parse_int(item, first)) {
                error = "invalid Verilator block range '" + item + "'";
                return false;
            }
            last = first;
        } else if (!parse_int(item.substr(0, dash), first) || !parse_int(item.substr(dash + 1), last) || first > last) {
            error = "invalid Verilator block range '" + item + "'";
            return false;
        }
        for (int line = first; line <= last; ++line) lines.insert(line);
    }
    return true;
}

bool parse_entry(const std::string &raw_record, int64_t count, RawEntry &entry, std::string &error)
{
    const auto fields = parse_fields(unescape_control_markers(raw_record));
    const auto page_it = fields.find("page");
    if (page_it == fields.end() || !page_it->second.starts_with("v_")) return false;
    const std::string page = page_it->second.substr(2);
    const size_t slash = page.find('/');
    if (slash == std::string::npos || page.substr(0, slash) != "line") return false;

    const auto file_it = fields.find("f");
    const auto line_it = fields.find("l");
    if (file_it == fields.end() || line_it == fields.end()) {
        error = "line coverage entry is missing file or line metadata";
        return false;
    }
    int line = 0;
    if (!parse_int(line_it->second, line) || line <= 0) {
        error = "line coverage entry has invalid source line";
        return false;
    }
    entry.file = file_it->second;
    entry.module = page.substr(slash + 1);
    entry.instance = fields.contains("h") ? fields.at("h") : "";
    entry.object = fields.contains("o") && !fields.at("o").empty() ? fields.at("o") : "line";
    entry.count = count;
    if (fields.contains("n") && !parse_int(fields.at("n"), entry.column)) {
        error = "line coverage entry has invalid column";
        return false;
    }
    entry.lines.insert(line);
    if (fields.contains("S") && !parse_ranges(fields.at("S"), entry.lines, error)) return false;
    return true;
}

bool contains_filter(const std::string &value, const std::string &filter)
{
    return filter.empty() || value.find(filter) != std::string::npos;
}

bool in_scope(const RawEntry &entry, const Options &opt)
{
    return contains_filter(entry.module, opt.module) &&
           (opt.instance.empty() || entry.instance.starts_with(opt.instance));
}

bool is_picker_generated_top(const RawEntry &entry)
{
    const std::filesystem::path path(entry.file);
    // Exclude only Picker's generated testbench wrapper.  The generated DUT
    // RTL remains part of coverage, matching the VCS helper semantics.
    return entry.module == "{{__TOP_MODULE_NAME__}}_top" &&
           path.filename() == "{{__TOP_MODULE_NAME__}}_top.sv";
}

bool collect_records(const Options &opt, std::vector<LineRecord> &records, std::string &error)
{
    std::ifstream input(opt.database);
    if (!input) {
        error = "cannot read coverage database " + opt.database;
        return false;
    }
    std::string text;
    if (!std::getline(input, text) || text != "# SystemC::Coverage-3") {
        error = "not a Verilator coverage.dat (missing Coverage-3 header)";
        return false;
    }

    using Key = std::tuple<std::string, int, std::string, std::string>;
    std::map<Key, LineRecord> merged;
    for (int line_number = 2; std::getline(input, text); ++line_number) {
        if (text.empty() || text.starts_with("#")) continue;
        const size_t first_space = text.find(' ');
        const size_t last_space = text.rfind(' ');
        if (first_space == std::string::npos || last_space <= first_space) {
            error = "line " + std::to_string(line_number) + ": malformed coverage record";
            return false;
        }
        int64_t count = 0;
        try {
            size_t used = 0;
            count = std::stoll(text.substr(last_space + 1), &used);
            if (used != text.size() - last_space - 1 || count < 0) throw std::invalid_argument("count");
        } catch (...) {
            error = "line " + std::to_string(line_number) + ": invalid hit count";
            return false;
        }
        RawEntry entry;
        if (!parse_entry(text.substr(first_space + 1, last_space - first_space - 1), count, entry, error)) {
            if (error.empty()) continue;
            error = "line " + std::to_string(line_number) + ": " + error;
            return false;
        }
        if (is_picker_generated_top(entry) || !in_scope(entry, opt)) continue;
        for (int source_line : entry.lines) {
            Key key{entry.file, source_line, entry.module, entry.instance};
            auto [it, inserted] = merged.try_emplace(key, LineRecord{
                entry.file, source_line, source_line == *entry.lines.begin() ? entry.column : 0,
                entry.module, entry.instance, entry.object, 0});
            it->second.count += entry.count;
        }
    }
    for (const auto &[key, record] : merged) records.push_back(record);
    return true;
}

std::string make_report_json(const Options &opt, const std::vector<LineRecord> &records)
{
    int64_t covered = 0;
    for (const auto &record : records) covered += record.count > 0;
    const int64_t total = records.size();
    const int64_t uncovered = total - covered;
    const double rate = total ? 100.0 * covered / total : 0.0;

    std::string output = "{\n";
    output += "  \"schema_version\": 1,\n";
    output += "  \"success\": true,\n";
    output += "  \"simulator\": \"verilator\",\n";
    output += "  \"query\": {\"kinds\": [\"line\"], \"module\": " +
              std::string(opt.module.empty() ? "null" : q(opt.module)) +
              ", \"instance\": " + std::string(opt.instance.empty() ? "null" : q(opt.instance)) +
              ", \"tests\": []},\n";
    output += "  \"metrics\": {\"line\": {\"covered\": " + std::to_string(covered) +
              ", \"total\": " + std::to_string(total) +
              ", \"uncovered\": " + std::to_string(uncovered) +
              ", \"rate\": " + std::to_string(rate) + "}},\n";
    output += "  \"items\": [\n";
    bool first = true;
    for (const auto &record : records) {
        if (record.count != 0) continue;
        if (!first) output += ",\n";
        first = false;
        output += "    {\"kind\": \"line\", \"file\": " + q(record.file) +
                  ", \"line\": " + std::to_string(record.line) +
                  ", \"column\": " + std::to_string(record.column) +
                  ", \"module\": " + q(record.module) +
                  ", \"instance\": " + q(record.instance) +
                  ", \"object\": " + q(record.object) +
                  ", \"count\": " + std::to_string(record.count) +
                  ", \"detail\": \"Verilator line coverage count is zero\"}";
    }
    output += "\n  ],\n";
    output += "  \"errors\": []\n";
    output += "}\n";
    return output;
}
} // namespace

int main(int argc, char **argv)
{
    Options opt;
    if (!parse_args(argc, argv, opt)) {
        usage(argv[0]);
        return 2;
    }
    if (lower_copy(opt.kind) != "line") {
        std::cerr << "verilator coverage helper: only line coverage is implemented in this helper\n";
        return 2;
    }
    std::vector<LineRecord> records;
    std::string error;
    if (!collect_records(opt, records, error)) {
        std::cerr << "verilator coverage helper: " << error << "\n";
        return 2;
    }
    const std::string output = make_report_json(opt, records);
    std::cout << output;
    return 0;
}
