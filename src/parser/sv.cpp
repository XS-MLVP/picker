#include "picker.hpp"
#include "parser/sv.hpp"

#include "slang/ast/Compilation.h"
#include "slang/ast/SemanticFacts.h"
#include "slang/ast/symbols/CompilationUnitSymbols.h"
#include "slang/ast/symbols/InstanceSymbols.h"
#include "slang/ast/symbols/PortSymbols.h"
#include "slang/ast/types/AllTypes.h"
#include "slang/diagnostics/DiagnosticEngine.h"
#include "slang/diagnostics/Diagnostics.h"
#include "slang/driver/Driver.h"
#include "slang/syntax/SyntaxVisitor.h"
#include "slang/text/SourceManager.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace picker { namespace parser {

    namespace {

        using slang::Diagnostic;
        using slang::DiagnosticEngine;
        using slang::Diagnostics;
        using slang::SourceManager;
        using slang::ast::ArgumentDirection;
        using slang::ast::Compilation;
        using slang::ast::InstanceSymbol;
        using slang::ast::PackedArrayType;
        using slang::ast::PortSymbol;
        using slang::ast::SymbolKind;
        using slang::ast::Type;
        using slang::driver::Driver;
        using slang::syntax::ModuleDeclarationSyntax;
        using slang::syntax::SyntaxVisitor;

        bool is_verilog_src(const std::string &path)
        {
            return path.ends_with(".sv") || path.ends_with(".v");
        }

        std::string normalize_path(const std::filesystem::path &path)
        {
            std::error_code ec;
            auto normalized = std::filesystem::weakly_canonical(path, ec);
            if (!ec) { return normalized.lexically_normal().string(); }
            return std::filesystem::absolute(path).lexically_normal().string();
        }

        std::string strip_comment_and_trim(const std::string &line)
        {
            auto s = picker::trim(line);
            if (s.starts_with("#")) { return ""; }
            auto pos = s.find('#');
            if (pos != std::string::npos) { s = picker::trim(s.substr(0, pos)); }
            return s;
        }

        bool diagnostics_have_errors(const std::span<const Diagnostic> diags)
        {
            for (const auto &diag : diags) {
                if (diag.isError()) { return true; }
            }
            return false;
        }

        void fatal_if_errors(const SourceManager &source_manager, const std::span<const Diagnostic> diags,
                             const std::string &context)
        {
            if (!diagnostics_have_errors(diags)) { return; }
            auto rendered = DiagnosticEngine::reportAll(source_manager, diags);
            PK_FATAL("Failed to %s with slang:\n%s", context.c_str(), rendered.c_str());
        }

        void collect_tree_diagnostics(const Driver &driver)
        {
            Diagnostics diags;
            for (const auto &tree : driver.syntaxTrees) { diags.append_range(tree->diagnostics()); }
            fatal_if_errors(driver.sourceManager, diags, "parse RTL sources");
        }

        void append_sorted_recursive_verilog_files(const std::filesystem::path &dir,
                                                   std::vector<std::string> &out)
        {
            std::vector<std::string> files;
            for (const auto &entry : std::filesystem::recursive_directory_iterator(dir)) {
                if (!entry.is_regular_file()) { continue; }
                auto path = entry.path().string();
                if (is_verilog_src(path)) { files.push_back(normalize_path(entry.path())); }
            }
            std::sort(files.begin(), files.end());
            out.insert(out.end(), files.begin(), files.end());
        }

        void append_direct_filelist_entry(const std::string &entry, std::vector<std::string> &out)
        {
            auto path = strip_comment_and_trim(entry);
            if (path.empty()) { return; }

            std::filesystem::path fs_path(path);
            if (!std::filesystem::exists(fs_path)) {
                return;
            }

            if (std::filesystem::is_directory(fs_path)) {
                append_sorted_recursive_verilog_files(fs_path, out);
                return;
            }

            if (std::filesystem::is_regular_file(fs_path) && is_verilog_src(path)) {
                out.push_back(normalize_path(fs_path));
            }
        }

        void append_source_arg(const std::string &file, std::vector<std::string> &args)
        {
            args.push_back(normalize_path(file));
        }

        void append_filelists_to_driver_args(const std::vector<std::string> &filelists, std::vector<std::string> &args)
        {
            std::vector<std::string> direct_files;
            for (const auto &filelist : filelists) {
                if (filelist.ends_with(".txt") || filelist.ends_with(".f")) {
                    args.push_back("-F");
                    args.push_back(normalize_path(filelist));
                    continue;
                }

                std::stringstream ss(filelist);
                std::string token;
                while (std::getline(ss, token, ',')) { append_direct_filelist_entry(token, direct_files); }
            }

            for (const auto &file : direct_files) { append_source_arg(file, args); }
        }

        void append_explicit_files_to_driver_args(const std::vector<std::string> &files, std::vector<std::string> &args)
        {
            for (const auto &file : files) { append_source_arg(file, args); }
        }

        std::vector<const char *> build_argv(const std::vector<std::string> &args)
        {
            std::vector<const char *> argv;
            argv.reserve(args.size());
            for (const auto &arg : args) { argv.push_back(arg.c_str()); }
            return argv;
        }

        struct ModuleCollector : SyntaxVisitor<ModuleCollector> {
            const SourceManager &source_manager;
            const std::string target_file;
            std::vector<std::string> names;

            ModuleCollector(const SourceManager &source_manager, std::string target_file)
                : source_manager(source_manager), target_file(std::move(target_file))
            {}

            void handle(const ModuleDeclarationSyntax &node)
            {
                auto loc = source_manager.getFullyOriginalLoc(node.header->name.location());
                auto full_path = source_manager.getFullPath(loc.buffer());
                if (!full_path.empty() && normalize_path(full_path) == target_file) {
                    names.push_back(std::string(node.header->name.valueText()));
                }
                visitDefault(node);
            }
        };

        std::string infer_last_module_name(const Driver &driver, const std::string &source_file)
        {
            ModuleCollector collector(driver.sourceManager, normalize_path(source_file));
            for (const auto &tree : driver.syntaxTrees) { tree->root().visit(collector); }

            if (collector.names.empty()) {
                PK_FATAL("No module declarations found in input file: %s", source_file.c_str());
            }
            return collector.names.back();
        }

        std::string port_direction_to_logic_pin_type(ArgumentDirection direction, const std::string &module_name,
                                                     const std::string &port_name)
        {
            switch (direction) {
            case ArgumentDirection::In:
                return "input";
            case ArgumentDirection::Out:
                return "output";
            case ArgumentDirection::InOut:
                return "inout";
            case ArgumentDirection::Ref:
                PK_FATAL("Module %s port %s uses unsupported direction 'ref'",
                         module_name.c_str(), port_name.c_str());
            default:
                PK_FATAL("Module %s port %s uses unsupported direction", module_name.c_str(), port_name.c_str());
            }
        }

        bool is_supported_flat_port_type(const Type &type)
        {
            const auto &canonical = type.getCanonicalType();
            if (canonical.kind == SymbolKind::ScalarType) { return true; }

            if (canonical.kind != SymbolKind::PackedArrayType) { return false; }

            const auto &packed = canonical.as<PackedArrayType>();
            return packed.elementType.getCanonicalType().kind == SymbolKind::ScalarType;
        }

        std::pair<int, int> extract_port_range(const Type &type)
        {
            const auto &canonical = type.getCanonicalType();
            if (canonical.kind == SymbolKind::ScalarType) { return {-1, 0}; }

            const auto &packed = canonical.as<PackedArrayType>();
            return {packed.range.left, packed.range.right};
        }

        void append_simple_port(std::vector<picker::sv_signal_define> &pins, const PortSymbol &port,
                                const std::string &module_name)
        {
            auto pin_type = port_direction_to_logic_pin_type(port.direction, module_name, std::string(port.name));
            const auto &type = port.getType();
            if (!is_supported_flat_port_type(type)) {
                PK_FATAL("Module %s port %s uses unsupported type '%s'. "
                         "picker export currently supports only scalar and single-dimension packed bit/logic ports.",
                         module_name.c_str(), std::string(port.name).c_str(), type.toString().c_str());
            }

            auto [high_bit, low_bit] = extract_port_range(type);
            pins.push_back({std::string(port.name), pin_type, high_bit, low_bit});
        }

        picker::sv_module_define convert_instance_to_module_define(const InstanceSymbol &instance, int module_nums)
        {
            picker::sv_module_define result;
            result.module_name = std::string(instance.name);
            result.module_nums = module_nums;

            for (const auto *port_symbol : instance.body.getPortList()) {
                switch (port_symbol->kind) {
                case SymbolKind::Port:
                    append_simple_port(result.pins, port_symbol->as<PortSymbol>(), result.module_name);
                    break;
                case SymbolKind::MultiPort:
                    PK_FATAL("Module %s uses unsupported multi-port declaration at port '%s'",
                             result.module_name.c_str(), std::string(port_symbol->name).c_str());
                case SymbolKind::InterfacePort:
                    PK_FATAL("Module %s uses unsupported interface port '%s'",
                             result.module_name.c_str(), std::string(port_symbol->name).c_str());
                default:
                    PK_FATAL("Module %s contains unsupported port symbol kind for '%s'",
                             result.module_name.c_str(), std::string(port_symbol->name).c_str());
                }
            }

            return result;
        }

        void prepare_driver(Driver &driver, const picker::export_opts &opts, const bool use_filelists)
        {
            driver.addStandardArgs();

            std::vector<std::string> args = {"picker-slang", "--single-unit", "--ignore-unknown-modules"};
            if (opts.sim == "vcs") {
                args.push_back("--compat");
                args.push_back("vcs");
            }

            if (use_filelists) { append_filelists_to_driver_args(opts.filelists, args); }
            append_explicit_files_to_driver_args(opts.file, args);

            auto argv = build_argv(args);
            if (!driver.parseCommandLine((int)argv.size(), argv.data())) {
                PK_FATAL("Failed to parse slang command line for RTL inputs");
            }
            if (!driver.processOptions()) {
                PK_FATAL("Failed to process slang options for RTL inputs");
            }
            if (!driver.parseAllSources()) {
                PK_FATAL("Failed to load RTL inputs for slang parsing");
            }

            collect_tree_diagnostics(driver);
        }

        std::map<std::string, int> parse_mname_and_numbers(std::vector<std::string> &name_and_nums)
        {
            // eg: MouduleA,1,ModuleB,3,MouldeC,ModuleE,ModuleF
            std::string m_name = "";
            int num            = 1;
            std::map<std::string, int> ret;
            for (auto v : name_and_nums) {
                v   = picker::trim(v);
                num = 1;
                if (picker::str_start_with_digit(v)) {
                    num = std::stoi(v);
                    if (!m_name.empty()) {
                        ret[m_name] = num;
                        m_name      = "";
                    } else {
                        PK_MESSAGE("Ignore num: %d, no matched Module find", num)
                    }
                } else {
                    vassert(!v.empty(), "Find empty Module name in --sname arg");
                    if (!m_name.empty()) { ret[m_name] = num; }
                    m_name = v;
                }
            }
            if (!m_name.empty()) ret[m_name] = 1;
            return ret;
        }

        int parse_sv(const picker::export_opts &opts, const std::map<std::string, int> &requested_module_counts,
                                std::vector<std::string> &resolved_module_names,
                                std::vector<picker::sv_module_define> &sv_module_result)
        {
            const bool has_explicit_requests = !requested_module_counts.empty();
            if (!has_explicit_requests && opts.file.size() != 1) {
                PK_FATAL("When module name not given (--sname), can only parse one .v/.sv file (%d find!)",
                         (int)opts.file.size());
            }

            Driver driver;
            prepare_driver(driver, opts, !opts.filelists.empty());
            if (has_explicit_requests) {
                for (const auto &[module_name, _] : requested_module_counts) {
                    resolved_module_names.push_back(module_name);
                }
            } else {
                resolved_module_names = {infer_last_module_name(driver, opts.file.front())};
            }

            driver.options.topModules = resolved_module_names;

            std::unique_ptr<Compilation> compilation;
            try {
                compilation = driver.createCompilation();
                (void)compilation->getRoot();
            } catch (const std::exception &e) {
                PK_FATAL("Failed to elaborate RTL with slang: %s", e.what());
            }

            auto diags = compilation->getAllDiagnostics();
            fatal_if_errors(driver.sourceManager, diags, "elaborate RTL design");

            std::unordered_map<std::string, const InstanceSymbol *> top_instances;
            for (const auto *instance : compilation->getRoot().topInstances) {
                top_instances.emplace(std::string(instance->name), instance);
            }

            for (const auto &module_name : resolved_module_names) {
                auto iter = top_instances.find(module_name);
                if (iter == top_instances.end()) {
                    PK_FATAL("Module: %s not find in input file", module_name.c_str());
                }

                const auto *instance = iter->second;
                const int module_nums = has_explicit_requests ? requested_module_counts.at(module_name) : 1;
                sv_module_result.push_back(convert_instance_to_module_define(*instance, module_nums));
            }

            return 0;
        }

    } // namespace

    // Collect .v/.sv files from --fs/--filelist options for module search
    void collect_verilog_from_filelists(const std::vector<std::string> &filelists,
                                        std::vector<std::string> &out)
    {
        namespace fs = std::filesystem;
        std::vector<std::string> entries;
        std::string last_list_base;

        for (const auto &fl : filelists) {
            if (fl.ends_with(".txt") || fl.ends_with(".f")) {
                // Lines inside are relative to the filelist file
                last_list_base = fs::absolute(fl).parent_path().string();
                std::ifstream ifs(fl);
                std::string line;
                while (std::getline(ifs, line)) {
                    entries.push_back(line);
                }
            } else {
                // Comma-separated entries directly from CLI
                std::stringstream ss(fl);
                std::string token;
                while (std::getline(ss, token, ',')) {
                    entries.push_back(token);
                }
            }
        }

        for (auto entry : entries) {
            entry = strip_comment_and_trim(entry);
            if (entry.empty()) continue;

            // Resolve relative path against the last seen filelist base if present
            std::string path = entry;
            if (!fs::exists(path) && !path.starts_with("/") && !last_list_base.empty()) {
                path = (fs::path(last_list_base) / path).string();
            }

            if (!fs::exists(path)) {
                // Not fatal here; filelist may contain non-Verilog or generated paths not yet present
                continue;
            }

            if (fs::is_directory(path)) {
                for (auto const &dir_entry : fs::recursive_directory_iterator(path)) {
                    if (!dir_entry.is_regular_file()) continue;
                    auto p = dir_entry.path().string();
                    if (is_verilog_src(p)) {
                        out.push_back(fs::absolute(p).string());
                    }
                }
            } else if (fs::is_regular_file(path)) {
                if (is_verilog_src(path)) {
                    out.push_back(fs::absolute(path).string());
                }
            }
        }
    }

    int sv(picker::export_opts &opts, std::vector<picker::sv_module_define> &sv_module_result)
    {
        std::vector<std::string> m_names;
        std::map<std::string, int> m_nums;

        if (opts.source_module_name_list.empty()) {
            parse_sv(opts, m_nums, m_names, sv_module_result);
        } else {
            m_nums  = parse_mname_and_numbers(opts.source_module_name_list);
            parse_sv(opts, m_nums, m_names, sv_module_result);
        }

        auto target_name = picker::join_str_vec(m_names, "_");
        opts.target_module_name = opts.target_module_name.size() == 0 ? target_name : opts.target_module_name;
        if (opts.target_dir.size() == 0 || opts.target_dir.back() == '/') {
            opts.target_dir += opts.target_module_name;
            PK_DEBUG("Set target dir to target module name: %s", opts.target_dir.c_str());
        }
        for (auto &m : sv_module_result) {
            PK_DEBUG("Module: %s, nums: %d", m.module_name.c_str(), m.module_nums);
            for (auto &p : m.pins) {
                PK_DEBUG("Pin: %s, type: %s, high: %d, low: %d", p.logic_pin.c_str(), p.logic_pin_type.c_str(),
                         p.logic_pin_hb, p.logic_pin_lb);
            }
        }
        PK_DEBUG("Target Module: %s", opts.target_module_name.c_str());

        return 0;
    }
}} // namespace picker::parser
