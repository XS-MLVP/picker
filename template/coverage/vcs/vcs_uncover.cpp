#include "covdb_user.h"

#include <algorithm>
#include <cstring>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

namespace {

struct Options {
    std::string database;
    std::string source_map;
    std::string kind = "line";
    std::vector<std::string> kinds;
    std::string detail = "all";
    std::string file;
    std::string module;
    std::string instance;
    std::vector<std::string> tests;
};

struct Item {
    std::string kind = "line";
    std::string file;
    std::string source_id;
    std::string logical_file;
    std::string resolved_file;
    int line = 0;
    int column = 0;
    std::string module;
    std::string instance;
    std::string object;
    int covered = 0;
    int coverable = 0;
    int count = 0;
    int status = 0;
    std::string detail;
    std::string atom_kind;
    std::string native_id;
    std::string native_name;
    std::string from_state;
    std::string to_state;
    bool counts_toward_rate = true;
};

class SharedDatabaseLock {
    int fd = -1;

public:
    explicit SharedDatabaseLock(const std::string &database)
    {
        const std::string path = database + ".lock";
        fd = open(path.c_str(), O_CREAT | O_RDONLY, 0666);
        if (fd >= 0 && flock(fd, LOCK_SH) != 0) {
            close(fd);
            fd = -1;
        }
    }

    ~SharedDatabaseLock()
    {
        if (fd >= 0) {
            flock(fd, LOCK_UN);
            close(fd);
        }
    }

    bool acquired() const { return fd >= 0; }
};

void usage(const char *argv0)
{
    std::cerr << "Usage: " << argv0
              << " --database <dir> [--kind line|toggle|branch|condition|fsm] "
              << "[--source-map <path>] "
              << "[--detail summary|all|covered|uncovered] "
              << "[--file <substr>] [--module <substr>] [--instance <prefix>] [--test <name>]\n";
}

bool parse_args(int argc, char **argv, Options &opt)
{
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto take = [&](std::string &out) {
            if (i + 1 >= argc) return false;
            out = argv[++i];
            return true;
        };
        if (arg == "--database") {
            if (!take(opt.database)) return false;
        } else if (arg == "--source-map") {
            if (!take(opt.source_map)) return false;
        } else if (arg == "--kind") {
            std::string kind;
            if (!take(kind)) return false;
            if (kind == "all") {
                opt.kinds = {"line", "toggle", "branch", "condition", "fsm"};
            } else if (std::find(opt.kinds.begin(), opt.kinds.end(), kind) == opt.kinds.end()) {
                opt.kinds.push_back(kind);
            }
        } else if (arg == "--detail") {
            if (!take(opt.detail)) return false;
        } else if (arg == "--file") {
            if (!take(opt.file)) return false;
        } else if (arg == "--module") {
            if (!take(opt.module)) return false;
        } else if (arg == "--instance") {
            if (!take(opt.instance)) return false;
        } else if (arg == "--test") {
            std::string test;
            if (!take(test)) return false;
            if (std::find(opt.tests.begin(), opt.tests.end(), test) == opt.tests.end())
                opt.tests.push_back(test);
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

std::string json_escape(const std::string &s)
{
    std::string out;
    out.reserve(s.size() + 8);
    for (unsigned char c : s) {
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
                char buf[7];
                std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                out += buf;
            } else {
                out += static_cast<char>(c);
            }
        }
    }
    return out;
}

std::string q(const std::string &s)
{
    return "\"" + json_escape(s) + "\"";
}

bool contains_filter(const std::string &value, const std::string &filter)
{
    return filter.empty() || value.find(filter) != std::string::npos;
}

bool item_in_scope(const Item &item, const Options &opt)
{
    if (!contains_filter(item.file, opt.file)) return false;
    if (!contains_filter(item.module, opt.module)) return false;
    if (!opt.instance.empty() && item.instance.rfind(opt.instance, 0) != 0) return false;
    return item.coverable > 0;
}

struct CoverageResult {
    struct Stats {
        int64_t covered = 0;
        int64_t total = 0;
    };

    std::vector<Item> uncovered;
    std::vector<Item> covered_items;
    int64_t covered = 0;
    int64_t total = 0;
    bool available = false;
    std::string design_revision;
    std::string inventory_revision;
    std::map<std::string, std::map<std::string, Stats>> breakdown;
};

std::string revision_hash(const std::vector<std::string> &values)
{
    // Deterministic FNV-1a revision.  This is an identity/version token, not a
    // security boundary; Toffee still publishes SHA-256 for persisted files.
    uint64_t hash = 1469598103934665603ULL;
    for (const auto &value : values) {
        for (unsigned char byte : value) {
            hash ^= byte;
            hash *= 1099511628211ULL;
        }
        hash ^= 0xff;
        hash *= 1099511628211ULL;
    }
    std::ostringstream output;
    output << std::hex << std::setfill('0') << std::setw(16) << hash;
    return output.str();
}

struct SourceLocation {
    std::string source_id;
    std::string logical_file;
    std::string resolved_file;
};

std::string tsv_unescape(const std::string &value)
{
    std::string result;
    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] != '\\' || i + 1 >= value.size()) {
            result += value[i];
            continue;
        }
        const char next = value[++i];
        if (next == 't') result += '\t';
        else if (next == 'n') result += '\n';
        else result += next;
    }
    return result;
}

std::map<std::string, SourceLocation> load_source_map(const std::string &path)
{
    std::map<std::string, SourceLocation> mappings;
    if (path.empty()) return mappings;
    std::ifstream input(path);
    for (std::string line; std::getline(input, line);) {
        if (line.empty() || line[0] == '#') continue;
        std::vector<std::string> fields;
        std::stringstream stream(line);
        for (std::string field; std::getline(stream, field, '\t');) fields.push_back(tsv_unescape(field));
        if (fields.size() == 4) mappings[fields[0]] = {fields[1], fields[2], fields[3]};
    }
    return mappings;
}

SourceLocation resolve_source(const std::string &native,
                              const std::map<std::string, SourceLocation> &mappings)
{
    auto found = mappings.find(native);
    if (found == mappings.end()) {
        const auto normalized = std::filesystem::path(native).lexically_normal().string();
        found = mappings.find(normalized);
    }
    if (found != mappings.end()) return found->second;
    std::vector<std::string> identity{native};
    return {"external:" + revision_hash(identity) + ":" + std::filesystem::path(native).filename().string(),
            "external/" + revision_hash(identity) + "/" + std::filesystem::path(native).filename().string(),
            native};
}

void ucapi_error_filter(covdbHandle errHdl, void *)
{
    int errcode = covdb_get(errHdl, NULL, NULL, covdbValue);
    if (covdbInvalidPropertyError == errcode ||
        covdbNotImplementedError == errcode ||
        covdbInvalidRelationError == errcode) {
        return;
    }
}

std::string covdb_str(covdbHandle obj, covdbPropertiesT prop)
{
    if (!obj) return "";
    char *s = covdb_get_str(obj, prop);
    return s ? std::string(s) : std::string();
}

void add_unique(std::vector<std::string> &values, const std::string &value)
{
    if (value.empty()) return;
    if (std::find(values.begin(), values.end(), value) == values.end()) values.push_back(value);
}

void collect_relation_names(covdbHandle obj, covdb1ToManyRelationsT relation, std::vector<std::string> &names)
{
    covdbHandle iter = covdb_iterate(obj, relation);
    if (!iter) return;
    for (covdbHandle item = covdb_scan(iter); item; item = covdb_scan(iter)) {
        add_unique(names, covdb_str(item, covdbName));
    }
    covdb_release_handle(iter);
}

void collect_testdata_names(const std::string &database_path, std::vector<std::string> &names)
{
    std::filesystem::path testdata =
        std::filesystem::path(database_path) / "snps" / "coverage" / "db" / "testdata";
    std::error_code ec;
    if (!std::filesystem::is_directory(testdata, ec)) return;

    for (const auto &entry : std::filesystem::directory_iterator(testdata, ec)) {
        if (ec) break;
        if (entry.is_directory(ec)) add_unique(names, entry.path().filename().string());
    }
}

bool is_internal_test(const std::string &name)
{
    return name == "{{__TOP_MODULE_NAME__}}";
}

struct TestLoadResult {
    covdbHandle handle = nullptr;
    std::vector<std::string> warnings;
    std::vector<std::string> loaded_tests;
};

TestLoadResult load_tests(covdbHandle design, const Options &opt)
{
    TestLoadResult result;
    std::vector<std::string> test_names;
    if (!opt.tests.empty()) {
        test_names = opt.tests;
    } else {
        collect_relation_names(design, covdbAvailableTests, test_names);
        collect_relation_names(design, covdbAvailableTestsWithoutSkinny, test_names);
        collect_relation_names(design, covdbTests, test_names);
        collect_testdata_names(opt.database, test_names);
        add_unique(test_names, std::filesystem::path(opt.database).stem().string());
        const bool has_named_test = std::any_of(test_names.begin(), test_names.end(),
                                                [](const std::string &name) {
                                                    return !is_internal_test(name);
                                                });
        if (has_named_test) {
            test_names.erase(std::remove_if(test_names.begin(), test_names.end(),
                                            [](const std::string &name) {
                                                return is_internal_test(name);
                                            }), test_names.end());
        }
    }

    covdbHandle test = nullptr;
    for (const auto &name : test_names) {
        if (!test) {
            test = covdb_load(covdbTest, design, name.c_str());
            if (!test) {
                result.warnings.push_back("failed to load VCS coverage test '" + name + "' from " + opt.database);
            } else {
                add_unique(result.loaded_tests, name);
            }
        } else {
            covdbHandle merged = covdb_loadmerge(covdbTest, test, name.c_str());
            if (merged) {
                test = merged;
                add_unique(result.loaded_tests, name);
            } else {
                result.warnings.push_back("failed to merge VCS coverage test '" + name + "' from " + opt.database);
            }
        }
    }
    result.handle = test;
    return result;
}

bool is_requested_metric(covdbHandle metric, const std::string &kind)
{
    if (kind == "line") return isLineMetric(metric);
    if (kind == "toggle") return isTglMetric(metric);
    if (kind == "branch") return isBranchMetric(metric);
    if (kind == "condition") return isCondMetric(metric);
    if (kind == "fsm") return isFsmMetric(metric);
    return false;
}

class CoverageCollector {
    covdbHandle design;
    covdbHandle test;
    const Options &opt;
    CoverageResult result;
    std::string current_instance;
    std::string current_module;
    std::string current_object;
    std::string current_fsm;
    std::string current_fsm_section;
    std::string current_file;
    int current_line = 0;
    std::map<std::string, Item> normalized;
    std::map<std::string, SourceLocation> source_map;

    bool is_picker_generated_top(const Item &item) const
    {
        const std::filesystem::path path(item.file);
        // Exclude only Picker's generated testbench wrapper.  The generated
        // {{__TOP_MODULE_NAME__}}.v is the DUT RTL and must remain in the
        // coverage denominator even when its compile-time path contains build/.
        return item.module == "{{__TOP_MODULE_NAME__}}_top" &&
               path.filename() == "{{__TOP_MODULE_NAME__}}_top.sv";
    }

    std::string item_key(const Item &item) const
    {
        // Keep instance and object identity until report aggregation.  Collapsing
        // instances here turns "one instance covered" into "all instances covered".
        return item.kind + "\x1f" + item.source_id + "\x1f" + item.logical_file + "\x1f" + item.module + "\x1f" +
               item.instance + "\x1f" + std::to_string(item.line) + "\x1f" +
               std::to_string(item.column) + "\x1f" + item.object + "\x1f" + item.detail;
    }

    void visit_leaf(covdbHandle obj, covdbHandle region, covdbHandle parent)
    {
        const auto type = (covdbObjTypesT)covdb_get(obj, region, NULL, covdbType);
        // UCAPI Condition containers also expose aggregate/scalar helper nodes.
        // covdbCross is the native truth-vector bin shown by VCS (00/01/10/11...).
        if (opt.kind == "condition" && type != covdbCross) return;
        int line = covdb_get(obj, region, NULL, covdbLineNo);
        if (line < 0 && parent) line = covdb_get(parent, region, NULL, covdbLineNo);
        if (line < 0) line = current_line;

        Item item;
        item.kind = opt.kind;
        item.file = current_file;
        const auto source = resolve_source(current_file, source_map);
        item.source_id = source.source_id;
        item.logical_file = source.logical_file;
        item.resolved_file = source.resolved_file;
        item.line = line;
        item.module = current_module;
        item.instance = current_instance;
        item.object = current_object;
        item.covered = covdb_get(obj, region, test, covdbCovered);
        item.coverable = covdb_get(obj, region, NULL, covdbCoverable);
        item.count = covdb_get(obj, region, test, covdbCovCount);
        item.status = covdb_get(obj, region, test, covdbCovStatus);
        item.detail = covdb_str(obj, covdbName);
        if (opt.kind == "line") item.atom_kind = "statement";
        if (opt.kind == "branch") item.atom_kind = "branch_arm";
        if (opt.kind == "toggle") item.atom_kind = "toggle_transition";
        if (opt.kind == "condition") {
            item.atom_kind = "condition_bin";
            item.native_id = item.detail;
            item.native_name = item.detail;
        }
        if (opt.kind == "fsm") {
            if (current_fsm_section == "sequences") return;
            item.object = current_fsm;
            if (current_fsm_section == "states") {
                item.atom_kind = "fsm_state";
                item.counts_toward_rate = false;
                std::string value_name = covdb_str(obj, covdbValueName);
                if (!value_name.empty()) item.detail = value_name;
                item.native_id = item.detail;
                item.native_name = item.detail;
            } else if (current_fsm_section == "transitions") {
                item.atom_kind = "fsm_transition";
                item.native_id = item.detail;
                item.native_name = item.detail;
                const auto arrow = item.detail.find("->");
                if (arrow != std::string::npos) {
                    item.from_state = item.detail.substr(0, arrow);
                    item.to_state = item.detail.substr(arrow + 2);
                }
            } else {
                return;
            }
        }
        if (!item_in_scope(item, opt) || is_picker_generated_top(item)) return;

        auto [position, inserted] = normalized.emplace(item_key(item), item);
        if (!inserted) {
            Item &existing = position->second;
            existing.covered = std::max(existing.covered, item.covered);
            existing.coverable = std::max(existing.coverable, item.coverable);
            existing.count = std::max(existing.count, item.count);
            existing.status = std::max(existing.status, item.status);
        }
    }

    void recurse_objects(covdbHandle obj, covdbHandle region, covdbHandle parent)
    {
        auto ty = (covdbObjTypesT)covdb_get(obj, region, NULL, covdbType);
        switch (ty) {
        case covdbContainer: {
            covdbHandle pobj = covdb_make_persistent_handle(obj);
            std::string prev_object = current_object;
            std::string prev_fsm = current_fsm;
            std::string prev_fsm_section = current_fsm_section;
            int prev_line = current_line;
            const std::string name = covdb_str(pobj, covdbName);
            current_object = name;
            if (opt.kind == "fsm") {
                if (current_fsm.empty()) current_fsm = name;
                else if (current_fsm_section.empty()) current_fsm_section = name;
            }
            int line = covdb_get(pobj, region, NULL, covdbLineNo);
            if (line >= 0) current_line = line;
            covdbHandle kids = covdb_iterate(pobj, covdbObjects);
            for (covdbHandle kid = covdb_scan(kids); kid; kid = covdb_scan(kids)) recurse_objects(kid, region, pobj);
            covdb_release_handle(kids);
            current_object = prev_object;
            current_fsm = prev_fsm;
            current_fsm_section = prev_fsm_section;
            current_line = prev_line;
            covdb_release_handle(pobj);
            break;
        }
        case covdbBlock:
        case covdbSequence:
        case covdbCross:
        case covdbIntegerValue:
        case covdbScalarValue:
        case covdbValueSet:
            visit_leaf(obj, region, parent);
            break;
        default:
            break;
        }
    }

    void recurse_qualified_region(covdbHandle region, covdbObjTypesT ty)
    {
        if (!region) return;
        std::string prev_instance = current_instance;
        std::string prev_module = current_module;
        std::string prev_file = current_file;
        int prev_line = current_line;
        if (ty == covdbSourceInstance) {
            current_instance = covdb_str(region, covdbFullName);
            covdbHandle def = covdb_get_handle(region, covdbDefinition);
            current_module = covdb_str(def, covdbName);
        } else if (ty == covdbSourceDefinition) {
            current_module = covdb_str(region, covdbName);
        }
        current_file = covdb_str(region, covdbFileName);
        current_line = 0;

        covdbHandle objs = covdb_iterate(region, covdbObjects);
        for (covdbHandle obj = covdb_scan(objs); obj; obj = covdb_scan(objs)) recurse_objects(obj, region, NULL);
        covdb_release_handle(objs);
        current_instance = prev_instance;
        current_module = prev_module;
        current_file = prev_file;
        current_line = prev_line;
    }

    void recurse_instance(covdbHandle inst)
    {
        if (!inst) return;
        covdbHandle preg = covdb_make_persistent_handle(inst);
        covdbHandle mets = covdb_iterate(test, covdbMetrics);
        for (covdbHandle met = covdb_scan(mets); met; met = covdb_scan(mets)) {
            if (is_requested_metric(met, opt.kind)) {
                result.available = true;
                covdbHandle pmet = covdb_make_persistent_handle(met);
                covdbHandle qreg = covdb_get_qualified_handle(preg, pmet, covdbIdentity);
                recurse_qualified_region(qreg, covdbSourceInstance);
                covdb_release_handle(qreg);
                covdb_release_handle(pmet);
            }
        }
        covdb_release_handle(mets);

        covdbHandle kids = covdb_iterate(preg, covdbInstances);
        for (covdbHandle kid = covdb_scan(kids); kid; kid = covdb_scan(kids)) recurse_instance(kid);
        covdb_release_handle(kids);
        covdb_release_handle(preg);
    }

public:
    CoverageCollector(covdbHandle design, covdbHandle test, const Options &opt)
        : design(design), test(test), opt(opt), source_map(load_source_map(opt.source_map)) {}

    CoverageResult collect()
    {
        covdb_set_error_callback(ucapi_error_filter, NULL);
        covdbHandle insts = covdb_iterate(design, covdbInstances);
        for (covdbHandle inst = covdb_scan(insts); inst; inst = covdb_scan(insts)) recurse_instance(inst);
        covdb_release_handle(insts);
        std::vector<std::string> design_items;
        std::vector<std::string> inventory_items;
        design_items.reserve(normalized.size());
        inventory_items.reserve(normalized.size());
        for (const auto &[key, item] : normalized) {
            design_items.push_back(key);
            const int64_t total = std::max(0, item.coverable);
            const int64_t covered = std::max<int64_t>(0, std::min<int64_t>(item.covered, total));
            if (item.counts_toward_rate) {
                result.total += total;
                result.covered += covered;
            }
            auto &stats = result.breakdown[item.logical_file][item.module];
            if (item.counts_toward_rate) {
                stats.total += total;
                stats.covered += covered;
            }
            if (covered < total) inventory_items.push_back(key);
            if ((opt.detail == "all" || opt.detail == "uncovered") && covered < total)
                result.uncovered.push_back(item);
            if ((opt.detail == "all" || opt.detail == "covered") && covered > 0)
                result.covered_items.push_back(item);
        }
        result.design_revision = revision_hash(design_items);
        result.inventory_revision = revision_hash(inventory_items);
        return result;
    }
};

std::string render_json(const Options &opt, const std::map<std::string, CoverageResult> &results,
                        const TestLoadResult &loaded)
{
    std::vector<Item> items;
    std::vector<Item> covered_items;
    for (const auto &[_, result] : results)
        items.insert(items.end(), result.uncovered.begin(), result.uncovered.end());
    for (const auto &[_, result] : results)
        covered_items.insert(covered_items.end(), result.covered_items.begin(), result.covered_items.end());
    auto item_less = [](const Item &lhs, const Item &rhs) {
        return std::tie(lhs.kind, lhs.file, lhs.module, lhs.line, lhs.object, lhs.detail, lhs.instance) <
               std::tie(rhs.kind, rhs.file, rhs.module, rhs.line, rhs.object, rhs.detail, rhs.instance);
    };
    std::sort(items.begin(), items.end(), item_less);
    std::sort(covered_items.begin(), covered_items.end(), item_less);
    std::vector<std::string> design_revisions;
    std::vector<std::string> inventory_revisions;
    for (const auto &[kind, result] : results) {
        design_revisions.push_back(kind + ":" + result.design_revision);
        inventory_revisions.push_back(kind + ":" + result.inventory_revision);
    }
    std::map<std::string, std::vector<size_t>> by_kind;
    std::map<std::string, std::vector<size_t>> by_file;
    std::map<std::string, std::vector<size_t>> by_module;
    for (size_t i = 0; i < items.size(); ++i) {
        by_kind[items[i].kind].push_back(i);
        by_file[items[i].logical_file].push_back(i);
        by_module[items[i].module].push_back(i);
    }
    auto render_counts = [](const std::map<std::string, std::vector<size_t>> &values) {
        std::string out = "{\n";
        bool first = true;
        for (const auto &[key, indexes] : values) {
            if (!first) out += ",\n";
            first = false;
            out += "      " + q(key) + ": " + std::to_string(indexes.size());
        }
        if (!values.empty()) out += "\n";
        out += "    }";
        return out;
    };
    auto render_indexes = [](const std::map<std::string, std::vector<size_t>> &values) {
        std::string out = "{\n";
        bool first_key = true;
        for (const auto &[key, indexes] : values) {
            if (!first_key) out += ",\n";
            first_key = false;
            out += "    " + q(key) + ": [";
            if (!indexes.empty()) {
                out += "\n";
            }
            for (size_t i = 0; i < indexes.size(); ++i) {
                out += "      " + std::to_string(indexes[i]);
                if (i + 1 != indexes.size()) out += ",";
                out += "\n";
            }
            if (!indexes.empty()) {
                out += "    ";
            }
            out += "]";
        }
        if (!values.empty()) out += "\n";
        out += "  }";
        return out;
    };
    auto render_stats = [&](const CoverageResult::Stats &stats) {
        const int64_t uncovered = stats.total - stats.covered;
        const double rate = stats.total ? 100.0 * stats.covered / stats.total : 0.0;
        return "{\"covered\": " + std::to_string(stats.covered) +
               ", \"total\": " + std::to_string(stats.total) +
               ", \"uncovered\": " + std::to_string(uncovered) +
               ", \"rate\": " + std::to_string(rate) + "}";
    };

    std::string out;
    out += "{\n";
    out += "  \"schema_version\": 5,\n";
    out += "  \"success\": true,\n";
    out += "  \"design_revision\": " + q(revision_hash(design_revisions)) + ",\n";
    out += "  \"inventory_revision\": " + q(revision_hash(inventory_revisions)) + ",\n";
    out += "  \"provider\": {\n";
    out += "    \"simulator\": \"vcs\",\n";
    out += "    \"backend\": \"ucapi-helper\",\n";
    out += "    \"identity\": \"picker.vcs.ucapi\",\n";
    out += "    \"identity_version\": 3,\n";
    out += "    \"contract_version\": 3,\n";
    out += "    \"databases\": [" + q(opt.database) + "],\n";
    out += "    \"source_map\": " + q(opt.source_map) + ",\n";
    out += "    \"helper\": \"\",\n";
    out += "    \"capabilities\": {\"supported_kinds\": [\"line\", \"toggle\", \"branch\", \"condition\", \"fsm\"], \"named_tests\": true, \"multiple_databases\": false, \"detail_modes\": true}\n";
    out += "  },\n";
    out += "  \"query\": {\n";
    out += "    \"kinds\": [";
    for (size_t i = 0; i < opt.kinds.size(); ++i) {
        if (i) out += ", ";
        out += q(opt.kinds[i]);
    }
    out += "],\n";
    out += "    \"module\": ";
    out += opt.module.empty() ? "null" : q(opt.module);
    out += ",\n";
    out += "    \"instance\": ";
    out += opt.instance.empty() ? "null" : q(opt.instance);
    out += ",\n";
    // Report the tests that actually contributed to the merged handle.  A
    // requested test can be absent when partial loading is enabled.
    const auto &reported_tests = loaded.loaded_tests;
    out += "    \"tests\": [";
    for (size_t i = 0; i < reported_tests.size(); ++i) {
        if (i) out += ", ";
        out += q(reported_tests[i]);
    }
    out += "],\n";
    out += "    \"detail_mode\": " + q(opt.detail) + "\n";
    out += "  },\n";
    out += "  \"summary\": {\n";
    out += "    \"total_uncovered\": " + std::to_string(items.size()) + ",\n";
    out += "    \"metrics\": {\n";
    bool first_metric = true;
    for (const auto &kind : opt.kinds) {
        const auto &result = results.at(kind);
        if (!first_metric) out += ",\n";
        first_metric = false;
        out += "      " + q(kind) + ": {\"covered\": " + std::to_string(result.covered) +
               ", \"total\": " + std::to_string(result.total) +
               ", \"uncovered\": " + std::to_string(result.total - result.covered) +
               ", \"rate\": " + std::to_string(result.total ? 100.0 * result.covered / result.total : 0.0) +
               ", \"available\": " + (result.available ? "true" : "false") +
               ", \"details_available\": " + (result.available ? "true" : "false") + "}";
    }
    out += "\n";
    out += "    },\n";
    out += "    \"by_kind\": " + render_counts(by_kind) + ",\n";
    out += "    \"by_file\": " + render_counts(by_file) + ",\n";
    out += "    \"by_module\": " + render_counts(by_module) + "\n";
    out += "  },\n";
    out += "  \"breakdown\": {\n";
    std::set<std::string> files;
    for (const auto &[_, result] : results)
        for (const auto &[file, __] : result.breakdown) files.insert(file);
    out += "    \"files\": {";
    if (!files.empty()) out += "\n";
    bool first_file = true;
    for (const auto &file : files) {
        if (!first_file) out += ",\n";
        first_file = false;
        out += "      " + q(file) + ": {\n";
        out += "        \"metrics\": {";
        bool first_file_metric = true;
        for (const auto &kind : opt.kinds) {
            CoverageResult::Stats file_stats;
            const auto file_it = results.at(kind).breakdown.find(file);
            if (file_it != results.at(kind).breakdown.end()) {
                for (const auto &[_, stats] : file_it->second) {
                    file_stats.covered += stats.covered;
                    file_stats.total += stats.total;
                }
            }
            if (!first_file_metric) out += ", ";
            first_file_metric = false;
            out += q(kind) + ": " + render_stats(file_stats);
        }
        out += "},\n";
        std::set<std::string> modules;
        for (const auto &[_, result] : results) {
            const auto file_it = result.breakdown.find(file);
            if (file_it != result.breakdown.end())
                for (const auto &[module, __] : file_it->second) modules.insert(module);
        }
        out += "        \"modules\": {";
        if (!modules.empty()) out += "\n";
        bool first_module = true;
        for (const auto &module : modules) {
            if (!first_module) out += ",\n";
            first_module = false;
            out += "          " + q(module) + ": {\"metrics\": {";
            bool first_module_metric = true;
            for (const auto &kind : opt.kinds) {
                CoverageResult::Stats stats;
                const auto file_it = results.at(kind).breakdown.find(file);
                if (file_it != results.at(kind).breakdown.end()) {
                    const auto module_it = file_it->second.find(module);
                    if (module_it != file_it->second.end()) stats = module_it->second;
                }
                if (!first_module_metric) out += ", ";
                first_module_metric = false;
                out += q(kind) + ": " + render_stats(stats);
            }
            out += "}}";
        }
        if (!modules.empty()) out += "\n        ";
        out += "}\n      }";
    }
    if (!files.empty()) out += "\n    ";
    out += "}\n  },\n";
    out += "  \"items\": [\n";
    for (size_t i = 0; i < items.size(); ++i) {
        const auto &it = items[i];
        out += "    {\n";
        out += "      \"kind\": " + q(it.kind) + ",\n";
        out += "      \"source_id\": " + q(it.source_id) + ",\n";
        out += "      \"logical_file\": " + q(it.logical_file) + ",\n";
        out += "      \"resolved_file\": " + q(it.resolved_file) + ",\n";
        out += "      \"line\": " + std::to_string(it.line) + ",\n";
        out += "      \"column\": " + std::to_string(it.column) + ",\n";
        out += "      \"module\": " + q(it.module) + ",\n";
        out += "      \"instance\": " + q(it.instance) + ",\n";
        out += "      \"object\": " + q(it.object) + ",\n";
        out += "      \"covered\": " + std::to_string(it.covered) + ",\n";
        out += "      \"coverable\": " + std::to_string(it.coverable) + ",\n";
        out += "      \"count\": " + std::to_string(it.count) + ",\n";
        out += "      \"status\": " + std::to_string(it.status) + ",\n";
        out += "      \"detail\": " + q(it.detail) + ",\n";
        out += "      \"atom_kind\": " + q(it.atom_kind) + ",\n";
        out += "      \"native_id\": " + q(it.native_id) + ",\n";
        out += "      \"native_name\": " + q(it.native_name) + ",\n";
        out += "      \"from_state\": " + q(it.from_state) + ",\n";
        out += "      \"to_state\": " + q(it.to_state) + ",\n";
        out += "      \"counts_toward_rate\": " + std::string(it.counts_toward_rate ? "true" : "false") + "\n";
        out += "    }";
        if (i + 1 != items.size()) out += ",";
        out += "\n";
    }
    out += "  ],\n";
    out += "  \"covered_items\": [\n";
    for (size_t i = 0; i < covered_items.size(); ++i) {
        const auto &it = covered_items[i];
        out += "    {\n";
        out += "      \"kind\": " + q(it.kind) + ",\n";
        out += "      \"source_id\": " + q(it.source_id) + ",\n";
        out += "      \"logical_file\": " + q(it.logical_file) + ",\n";
        out += "      \"resolved_file\": " + q(it.resolved_file) + ",\n";
        out += "      \"line\": " + std::to_string(it.line) + ",\n";
        out += "      \"column\": " + std::to_string(it.column) + ",\n";
        out += "      \"module\": " + q(it.module) + ",\n";
        out += "      \"instance\": " + q(it.instance) + ",\n";
        out += "      \"object\": " + q(it.object) + ",\n";
        out += "      \"covered\": " + std::to_string(it.covered) + ",\n";
        out += "      \"coverable\": " + std::to_string(it.coverable) + ",\n";
        out += "      \"count\": " + std::to_string(it.count) + ",\n";
        out += "      \"status\": " + std::to_string(it.status) + ",\n";
        out += "      \"detail\": " + q(it.detail) + ",\n";
        out += "      \"atom_kind\": " + q(it.atom_kind) + ",\n";
        out += "      \"native_id\": " + q(it.native_id) + ",\n";
        out += "      \"native_name\": " + q(it.native_name) + ",\n";
        out += "      \"from_state\": " + q(it.from_state) + ",\n";
        out += "      \"to_state\": " + q(it.to_state) + ",\n";
        out += "      \"counts_toward_rate\": " + std::string(it.counts_toward_rate ? "true" : "false") + "\n";
        out += "    }";
        if (i + 1 != covered_items.size()) out += ",";
        out += "\n";
    }
    out += "  ],\n";
    out += "  \"errors\": [";
    for (size_t i = 0; i < loaded.warnings.size(); ++i) {
        if (i) out += ", ";
        out += q(loaded.warnings[i]);
    }
    out += "]\n";
    out += "}\n";
    return out;
}

} // namespace

int main(int argc, char **argv)
{
    Options opt;
    if (!parse_args(argc, argv, opt)) {
        usage(argv[0]);
        return 2;
    }
    if (opt.kinds.empty()) opt.kinds = {"line", "toggle", "branch", "condition", "fsm"};
    for (const auto &kind : opt.kinds)
        if (kind != "line" && kind != "toggle" && kind != "branch" &&
            kind != "condition" && kind != "fsm") {
            std::cerr << "coverage kind is not implemented in this helper: " << kind << "\n";
            return 2;
        }
    if (opt.detail != "summary" && opt.detail != "all" && opt.detail != "covered" &&
        opt.detail != "uncovered") {
        std::cerr << "invalid --detail mode: " << opt.detail << "\n";
        return 2;
    }
    if (!std::filesystem::exists(opt.database)) {
        std::cerr << "coverage database path does not exist: " << opt.database << "\n";
        return 1;
    }

    SharedDatabaseLock lock(opt.database);
    if (!lock.acquired()) {
        std::cerr << "failed to acquire coverage database lock: " << opt.database << ".lock\n";
        return 1;
    }

    covdb_configure(covdbDisplayErrors, (char*)"false");
    covdbHandle design = covdb_load(covdbDesign, NULL, opt.database.c_str());
    if (!design) {
        std::cerr << "failed to load VCS coverage database: " << opt.database << "\n";
        return 1;
    }
    TestLoadResult loaded = load_tests(design, opt);
    if (!loaded.handle) {
        if (loaded.warnings.empty()) {
            std::cerr << "no usable VCS coverage testdata found in " << opt.database << "\n";
        } else {
            for (const auto &warning : loaded.warnings) std::cerr << "warning: " << warning << "\n";
        }
        covdb_unload(design);
        return 1;
    }

    for (const auto &warning : loaded.warnings) std::cerr << "warning: " << warning << "\n";

    std::map<std::string, CoverageResult> results;
    for (const auto &kind : opt.kinds) {
        opt.kind = kind;
        CoverageCollector collector(design, loaded.handle, opt);
        results.emplace(kind, collector.collect());
    }
    std::cout << render_json(opt, results, loaded);

    covdb_unload(loaded.handle);
    covdb_unload(design);
    return 0;
}
