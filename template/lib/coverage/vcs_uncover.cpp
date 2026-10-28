#include "covdb_user.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
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
    std::string kind = "line";
    std::vector<std::string> kinds;
    std::string module;
    std::string instance;
    std::vector<std::string> tests;
};

struct Item {
    std::string kind = "line";
    std::string file;
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
              << "[--module <substr>] [--instance <prefix>] [--test <name>]\n";
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
        } else if (arg == "--kind") {
            std::string kind;
            if (!take(kind)) return false;
            if (std::find(opt.kinds.begin(), opt.kinds.end(), kind) == opt.kinds.end()) {
                opt.kinds.push_back(kind);
            }
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
    if (!contains_filter(item.module, opt.module)) return false;
    if (!opt.instance.empty() && item.instance.rfind(opt.instance, 0) != 0) return false;
    return item.coverable > 0;
}

struct CoverageResult {
    std::vector<Item> uncovered;
    int64_t covered = 0;
    int64_t total = 0;
};

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
        return item.kind + "\x1f" + item.file + "\x1f" + item.module + "\x1f" +
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
        item.line = line;
        item.module = current_module;
        item.instance = current_instance;
        item.object = current_object;
        item.covered = covdb_get(obj, region, test, covdbCovered);
        item.coverable = covdb_get(obj, region, NULL, covdbCoverable);
        item.count = covdb_get(obj, region, test, covdbCovCount);
        item.status = covdb_get(obj, region, test, covdbCovStatus);
        item.detail = covdb_str(obj, covdbName);
        if (opt.kind == "fsm") {
            if (current_fsm_section == "sequences") return;
            item.object = current_fsm;
            if (current_fsm_section == "states") {
                item.counts_toward_rate = false;
                std::string value_name = covdb_str(obj, covdbValueName);
                if (!value_name.empty()) item.detail = value_name;
            } else if (current_fsm_section == "transitions") {
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
        : design(design), test(test), opt(opt) {}

    CoverageResult collect()
    {
        covdb_set_error_callback(ucapi_error_filter, NULL);
        covdbHandle insts = covdb_iterate(design, covdbInstances);
        for (covdbHandle inst = covdb_scan(insts); inst; inst = covdb_scan(insts)) recurse_instance(inst);
        covdb_release_handle(insts);
        for (const auto &[_, item] : normalized) {
            const int64_t total = std::max(0, item.coverable);
            const int64_t covered = std::max<int64_t>(0, std::min<int64_t>(item.covered, total));
            if (item.counts_toward_rate) {
                result.total += total;
                result.covered += covered;
            }
            if (covered < total) result.uncovered.push_back(item);
        }
        return result;
    }
};

std::string render_json(const Options &opt, const std::map<std::string, CoverageResult> &results,
                        const TestLoadResult &loaded)
{
    std::vector<Item> items;
    for (const auto &[_, result] : results)
        items.insert(items.end(), result.uncovered.begin(), result.uncovered.end());
    std::sort(items.begin(), items.end(), [](const Item &lhs, const Item &rhs) {
        return std::tie(lhs.kind, lhs.file, lhs.module, lhs.line, lhs.object, lhs.detail, lhs.instance) <
               std::tie(rhs.kind, rhs.file, rhs.module, rhs.line, rhs.object, rhs.detail, rhs.instance);
    });

    std::string out = "{\n";
    out += "  \"schema_version\": 1,\n";
    out += "  \"success\": true,\n";
    out += "  \"simulator\": \"vcs\",\n";
    out += "  \"query\": {\n";
    out += "    \"kinds\": [";
    for (size_t i = 0; i < opt.kinds.size(); ++i) {
        if (i) out += ", ";
        out += q(opt.kinds[i]);
    }
    out += "],\n";
    out += "    \"module\": " + std::string(opt.module.empty() ? "null" : q(opt.module)) + ",\n";
    out += "    \"instance\": " + std::string(opt.instance.empty() ? "null" : q(opt.instance)) + ",\n";
    out += "    \"tests\": [";
    for (size_t i = 0; i < loaded.loaded_tests.size(); ++i) {
        if (i) out += ", ";
        out += q(loaded.loaded_tests[i]);
    }
    out += "]\n  },\n";
    out += "  \"metrics\": {\n";
    for (size_t i = 0; i < opt.kinds.size(); ++i) {
        const auto &kind = opt.kinds[i];
        const auto &result = results.at(kind);
        const int64_t uncovered = result.total - result.covered;
        const double rate = result.total ? 100.0 * result.covered / result.total : 0.0;
        out += "    " + q(kind) + ": {\"covered\": " + std::to_string(result.covered) +
               ", \"total\": " + std::to_string(result.total) +
               ", \"uncovered\": " + std::to_string(uncovered) +
               ", \"rate\": " + std::to_string(rate) + "}";
        if (i + 1 != opt.kinds.size()) out += ",";
        out += "\n";
    }
    out += "  },\n";
    out += "  \"items\": [\n";
    for (size_t i = 0; i < items.size(); ++i) {
        const auto &item = items[i];
        out += "    {\"kind\": " + q(item.kind) +
               ", \"file\": " + q(item.file) +
               ", \"line\": " + std::to_string(item.line) +
               ", \"column\": " + std::to_string(item.column) +
               ", \"module\": " + q(item.module) +
               ", \"instance\": " + q(item.instance) +
               ", \"object\": " + q(item.object) +
               ", \"count\": " + std::to_string(item.count) +
               ", \"detail\": " + q(item.detail) + "}";
        if (i + 1 != items.size()) out += ",";
        out += "\n";
    }
    out += "  ],\n";
    out += "  \"errors\": [";
    for (size_t i = 0; i < loaded.warnings.size(); ++i) {
        if (i) out += ", ";
        out += q(loaded.warnings[i]);
    }
    out += "]\n}\n";
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
    if (opt.kinds.empty()) opt.kinds = {"line"};
    for (const auto &kind : opt.kinds)
        if (kind != "line" && kind != "toggle" && kind != "branch" &&
            kind != "condition" && kind != "fsm") {
            std::cerr << "coverage kind is not implemented in this helper: " << kind << "\n";
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
