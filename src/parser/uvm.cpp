#include "picker.hpp"
#include "parser/sv.hpp"
#include "parser/uvm.hpp"

#include "slang/ast/Compilation.h"
#include "slang/ast/SemanticFacts.h"
#include "slang/ast/symbols/ClassSymbols.h"
#include "slang/ast/symbols/CompilationUnitSymbols.h"
#include "slang/ast/symbols/VariableSymbols.h"
#include "slang/ast/types/AllTypes.h"
#include "slang/diagnostics/AllDiags.h"
#include "slang/diagnostics/DiagnosticEngine.h"
#include "slang/diagnostics/Diagnostics.h"
#include "slang/driver/Driver.h"
#include "slang/syntax/AllSyntax.h"
#include "slang/syntax/SyntaxTree.h"
#include "slang/syntax/SyntaxVisitor.h"
#include "slang/text/SourceManager.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <optional>
#include <sstream>
#include <unordered_set>

namespace picker { namespace parser {

    namespace {

        using slang::Diagnostic;
        using slang::DiagnosticEngine;
        using slang::Diagnostics;
        using slang::SourceManager;
        using slang::ast::ClassPropertySymbol;
        using slang::ast::ClassType;
        using slang::ast::Compilation;
        using slang::ast::PackedArrayType;
        using slang::ast::PredefinedIntegerType;
        using slang::ast::SymbolKind;
        using slang::ast::Type;
        using slang::ast::VariableFlags;
        using slang::ast::VariableLifetime;
        using slang::driver::Driver;
        using slang::syntax::ClassDeclarationSyntax;
        using slang::syntax::CompilationUnitSyntax;
        using slang::syntax::SyntaxTree;
        using slang::syntax::SyntaxVisitor;

        struct TransactionTargetFile {
            std::string display_path;
            std::string normalized_path;
        };

        std::string normalize_path(const std::filesystem::path &path);

        struct ClassSyntaxInfo {
            std::string name;
            std::string base_name;
        };

        struct ClassCollector : SyntaxVisitor<ClassCollector> {
            const SourceManager &source_manager;
            const std::string target_file;
            std::vector<ClassSyntaxInfo> classes;

            ClassCollector(const SourceManager &source_manager, std::string target_file)
                : source_manager(source_manager), target_file(std::move(target_file))
            {}

            void handle(const ClassDeclarationSyntax &node)
            {
                auto loc = source_manager.getFullyOriginalLoc(node.name.location());
                auto full_path = source_manager.getFullPath(loc.buffer());
                if (!full_path.empty() && normalize_path(full_path) == target_file) {
                    std::string base_name;
                    if (node.extendsClause) { base_name = node.extendsClause->baseName->toString(); }
                    classes.push_back({std::string(node.name.valueText()), std::move(base_name)});
                }
                visitDefault(node);
            }
        };

        bool is_verilog_src(const std::string &path)
        {
            return path.ends_with(".sv") || path.ends_with(".v");
        }

        bool is_command_file(const std::string &path)
        {
            return path.ends_with(".f") || path.ends_with(".txt");
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

        bool is_ignorable_uvm_stub_diagnostic(const Diagnostic &diag)
        {
            if (diag.code != slang::diag::UndeclaredIdentifier || diag.args.empty()) { return false; }

            const auto *name = std::get_if<std::string>(&diag.args[0]);
            return name != nullptr && *name == "uvm_sequence_item";
        }

        Diagnostics filter_ignorable_uvm_stub_diagnostics(const std::span<const Diagnostic> diags)
        {
            Diagnostics filtered;
            for (const auto &diag : diags) {
                if (!is_ignorable_uvm_stub_diagnostic(diag)) { filtered.push_back(diag); }
            }
            return filtered;
        }

        void fatal_if_errors(const SourceManager &source_manager, const std::span<const Diagnostic> diags,
                             const std::string &context)
        {
            if (!diagnostics_have_errors(diags)) { return; }
            auto rendered = DiagnosticEngine::reportAll(source_manager, diags);
            PK_FATAL("Failed to %s with slang:\n%s", context.c_str(), rendered.c_str());
        }

        void dedupe_targets(std::vector<TransactionTargetFile> &targets)
        {
            std::unordered_set<std::string> seen;
            std::vector<TransactionTargetFile> deduped;
            deduped.reserve(targets.size());

            for (auto &target : targets) {
                if (seen.insert(target.normalized_path).second) { deduped.push_back(std::move(target)); }
            }

            targets = std::move(deduped);
        }

        std::optional<std::filesystem::path> resolve_existing_entry(const std::filesystem::path &path)
        {
            if (std::filesystem::exists(path)) { return path; }
            if (!path.has_extension()) {
                for (const auto *ext : {".sv", ".v"}) {
                    auto candidate = std::filesystem::path(path.string() + ext);
                    if (std::filesystem::exists(candidate)) { return candidate; }
                }
            }
            return std::nullopt;
        }

        void append_target_file(const std::filesystem::path &path, std::vector<TransactionTargetFile> &targets)
        {
            if (std::filesystem::is_directory(path)) {
                std::vector<std::filesystem::path> files;
                for (const auto &entry : std::filesystem::recursive_directory_iterator(path)) {
                    if (!entry.is_regular_file()) { continue; }
                    if (is_verilog_src(entry.path().string())) { files.push_back(entry.path()); }
                }

                std::sort(files.begin(), files.end());
                for (const auto &file : files) {
                    targets.push_back({file.string(), normalize_path(file)});
                }
                return;
            }

            if (std::filesystem::is_regular_file(path) && is_verilog_src(path.string())) {
                targets.push_back({path.string(), normalize_path(path)});
            }
        }

        void append_direct_cli_entry_to_targets(const std::string &entry, std::vector<TransactionTargetFile> &targets)
        {
            auto trimmed = picker::trim(entry);
            if (trimmed.empty()) { return; }

            auto resolved = resolve_existing_entry(std::filesystem::path(trimmed));
            if (!resolved) { PK_FATAL("Transaction source not found: %s", trimmed.c_str()); }
            append_target_file(*resolved, targets);
        }

        void collect_targets_from_command_file(const std::filesystem::path &filelist_path,
                                               std::unordered_set<std::string> &visited_filelists,
                                               std::vector<TransactionTargetFile> &targets)
        {
            const auto normalized = normalize_path(filelist_path);
            if (!visited_filelists.insert(normalized).second) { return; }

            std::ifstream stream(filelist_path);
            if (!stream.is_open()) {
                PK_FATAL("Failed to open transaction filelist: %s", filelist_path.string().c_str());
            }

            const auto base_dir = std::filesystem::absolute(filelist_path).parent_path();
            std::string line;
            while (std::getline(stream, line)) {
                auto entry = strip_comment_and_trim(line);
                if (entry.empty()) { continue; }

                if (entry.rfind("-f ", 0) == 0 || entry.rfind("-F ", 0) == 0) {
                    auto nested = picker::trim(entry.substr(2));
                    if (nested.empty()) { continue; }
                    auto nested_path = base_dir / nested;
                    collect_targets_from_command_file(nested_path, visited_filelists, targets);
                    continue;
                }

                if (entry.starts_with("+incdir+") || entry.starts_with("+define+") || entry.rfind("-I", 0) == 0 ||
                    entry.rfind("-D", 0) == 0 || entry.rfind("--include-directory", 0) == 0 ||
                    entry.rfind("--define-macro", 0) == 0) {
                    continue;
                }

                auto resolved = resolve_existing_entry(base_dir / entry);
                if (!resolved) { continue; }
                append_target_file(*resolved, targets);
            }
        }

        std::vector<TransactionTargetFile> collect_transaction_target_files(const picker::pack_opts &opts)
        {
            std::vector<TransactionTargetFile> targets;

            for (const auto &file : opts.files) { append_direct_cli_entry_to_targets(file, targets); }

            std::unordered_set<std::string> visited_filelists;
            for (const auto &filelist : opts.filelist) {
                if (is_command_file(filelist)) {
                    auto resolved = resolve_existing_entry(std::filesystem::path(filelist));
                    if (!resolved) {
                        PK_FATAL("Transaction filelist not found: %s", filelist.c_str());
                    }
                    collect_targets_from_command_file(*resolved, visited_filelists, targets);
                    continue;
                }

                std::stringstream ss(filelist);
                std::string token;
                while (std::getline(ss, token, ',')) { append_direct_cli_entry_to_targets(token, targets); }
            }

            dedupe_targets(targets);
            return targets;
        }

        void append_sorted_recursive_verilog_files(const std::filesystem::path &dir, std::vector<std::string> &args)
        {
            std::vector<std::string> files;
            for (const auto &entry : std::filesystem::recursive_directory_iterator(dir)) {
                if (!entry.is_regular_file()) { continue; }
                auto path = entry.path().string();
                if (is_verilog_src(path)) { files.push_back(normalize_path(entry.path())); }
            }

            std::sort(files.begin(), files.end());
            args.insert(args.end(), files.begin(), files.end());
        }

        void append_direct_source_to_driver_args(const std::string &entry, std::vector<std::string> &args)
        {
            auto trimmed = picker::trim(entry);
            if (trimmed.empty()) { return; }

            auto resolved = resolve_existing_entry(std::filesystem::path(trimmed));
            if (!resolved) { PK_FATAL("Transaction source not found: %s", trimmed.c_str()); }

            if (std::filesystem::is_directory(*resolved)) {
                append_sorted_recursive_verilog_files(*resolved, args);
                return;
            }

            args.push_back(resolved->string());
        }

        void append_filelists_to_driver_args(const std::vector<std::string> &filelists, std::vector<std::string> &args)
        {
            for (const auto &filelist : filelists) {
                if (is_command_file(filelist)) {
                    auto resolved = resolve_existing_entry(std::filesystem::path(filelist));
                    if (!resolved) {
                        PK_FATAL("Transaction filelist not found: %s", filelist.c_str());
                    }
                    args.push_back("-F");
                    args.push_back(resolved->string());
                    continue;
                }

                std::stringstream ss(filelist);
                std::string token;
                while (std::getline(ss, token, ',')) { append_direct_source_to_driver_args(token, args); }
            }
        }

        std::vector<const char *> build_argv(const std::vector<std::string> &args)
        {
            std::vector<const char *> argv;
            argv.reserve(args.size());
            for (const auto &arg : args) { argv.push_back(arg.c_str()); }
            return argv;
        }

        void add_uvm_stub_macros(Driver &driver)
        {
            static const std::vector<std::string> macros = {
                "UVM_ALL_ON=0",
                "uvm_object_utils(x)=",
                "uvm_object_utils_begin(x)=",
                "uvm_object_utils_end=",
                "uvm_field_int(a,b)=",
                "uvm_field_enum(t,a,b)=",
                "uvm_field_object(a,b)=",
                "uvm_field_string(a,b)=",
            };

            driver.options.defines.insert(driver.options.defines.end(), macros.begin(), macros.end());
        }

        void add_uvm_stub_tree(Driver &driver)
        {
            static constexpr std::string_view uvm_stub = R"(
class uvm_sequence_item;
    function new(string name = "");
    endfunction
endclass
)";

            auto tree = SyntaxTree::fromText(uvm_stub, driver.sourceManager, "picker_uvm_stub", "<picker_uvm_stub>",
                                             driver.createOptionBag());
            fatal_if_errors(driver.sourceManager, tree->diagnostics(), "parse picker UVM stub");
            driver.syntaxTrees.push_back(std::move(tree));
        }

        void prepare_driver(Driver &driver, const picker::pack_opts &opts)
        {
            driver.addStandardArgs();

            std::vector<std::string> args = {"picker-slang-uvm"};
            for (const auto &file : opts.files) { append_direct_source_to_driver_args(file, args); }
            append_filelists_to_driver_args(opts.filelist, args);

            auto argv = build_argv(args);
            if (!driver.parseCommandLine((int)argv.size(), argv.data())) {
                PK_FATAL("Failed to parse slang command line for transaction inputs");
            }
            if (!driver.processOptions()) {
                PK_FATAL("Failed to process slang options for transaction inputs");
            }

            add_uvm_stub_macros(driver);

            if (!driver.parseAllSources()) {
                PK_FATAL("Failed to load transaction inputs for slang parsing");
            }

            add_uvm_stub_tree(driver);

            Diagnostics diags;
            for (const auto &tree : driver.syntaxTrees) { diags.append_range(tree->diagnostics()); }
            auto filtered = filter_ignorable_uvm_stub_diagnostics(diags);
            fatal_if_errors(driver.sourceManager, filtered, "parse transaction sources");
        }

        bool is_supported_transaction_base_type(const Type &type)
        {
            const auto &canonical = type.getCanonicalType();
            if (canonical.kind == SymbolKind::ScalarType) { return true; }

            if (canonical.kind != SymbolKind::PredefinedIntegerType) { return false; }

            const auto &integer = canonical.as<PredefinedIntegerType>();
            switch (integer.integerKind) {
            case PredefinedIntegerType::Byte:
            case PredefinedIntegerType::ShortInt:
            case PredefinedIntegerType::Int:
            case PredefinedIntegerType::LongInt:
                return true;
            default:
                return false;
            }
        }

        [[noreturn]] void fail_unsupported_member_type(const ClassType &class_type, const ClassPropertySymbol &property,
                                                       const std::string &reason)
        {
            auto type_text = property.getType().toString();
            PK_FATAL("Transaction class %s member %s uses unsupported type '%s' (%s)",
                     std::string(class_type.name).c_str(), std::string(property.name).c_str(), type_text.c_str(),
                     reason.c_str());
        }

        uvm_parameter convert_property_to_parameter(const ClassType &class_type, const ClassPropertySymbol &property)
        {
            if (property.lifetime == VariableLifetime::Static) {
                fail_unsupported_member_type(class_type, property, "static members are not supported");
            }
            if (property.flags.has(VariableFlags::Const)) {
                fail_unsupported_member_type(class_type, property, "const members are not supported");
            }

            const auto &type = property.getType();
            if (type.isError()) {
                PK_FATAL("Failed to resolve transaction class %s member %s type",
                         std::string(class_type.name).c_str(), std::string(property.name).c_str());
            }
            if (type.isUnpackedArray() || type.isQueue() || type.isAssociativeArray()) {
                fail_unsupported_member_type(class_type, property, "only scalar and single packed fields are supported");
            }
            if (type.isStruct() || type.isUnion() || type.isEnum() || type.isString() || type.isClass() ||
                type.isVirtualInterface()) {
                fail_unsupported_member_type(class_type, property, "complex types are not supported");
            }

            const auto &canonical = type.getCanonicalType();
            if (canonical.kind == SymbolKind::PackedArrayType) {
                const auto &packed = canonical.as<PackedArrayType>();
                if (packed.elementType.getCanonicalType().kind == SymbolKind::PackedArrayType) {
                    fail_unsupported_member_type(class_type, property, "multi-dimensional packed arrays are not supported");
                }
                if (!is_supported_transaction_base_type(packed.elementType)) {
                    fail_unsupported_member_type(class_type, property, "packed arrays must be based on bit/logic/byte/shortint/int/longint");
                }
            } else if (!is_supported_transaction_base_type(canonical)) {
                fail_unsupported_member_type(class_type, property, "only bit/logic/byte/shortint/int/longint members are supported");
            }

            const auto width = type.getBitWidth();
            if (width == 0) {
                fail_unsupported_member_type(class_type, property, "field width must be a constant");
            }
            if (width > std::numeric_limits<int>::max()) {
                PK_FATAL("Transaction class %s member %s width exceeds picker's supported range (%llu bits)",
                         std::string(class_type.name).c_str(), std::string(property.name).c_str(),
                         (unsigned long long)width);
            }

            uvm_parameter param;
            param.name = std::string(property.name);
            param.bit_count = (int)width;
            param.byte_count = (param.bit_count + 7) / 8;
            param.is_macro = 0;
            param.macro_name = "";
            param.current_index = "0";
            return param;
        }

        const ClassType &select_transaction_class(const Compilation &compilation, const SourceManager &source_manager,
                                                  const std::string &target_file)
        {
            std::vector<const ClassType *> candidates;

            for (const auto &tree : compilation.getSyntaxTrees()) {
                if (tree->root().kind != slang::syntax::SyntaxKind::CompilationUnit) { continue; }

                ClassCollector collector(source_manager, target_file);
                tree->root().visit(collector);
                if (collector.classes.empty()) { continue; }

                auto *unit = compilation.getCompilationUnit(tree->root().as<CompilationUnitSyntax>());
                for (const auto &class_info : collector.classes) {
                    if (class_info.base_name.find("uvm_sequence_item") == std::string::npos) { continue; }

                    const auto *symbol = unit ? unit->find(class_info.name) : nullptr;
                    if (!symbol) { symbol = compilation.getRootNoFinalize().find(class_info.name); }
                    if (!symbol || symbol->kind != SymbolKind::ClassType) { continue; }

                    const auto &class_type = symbol->as<ClassType>();
                    if (class_type.isAbstract || class_type.isInterface || class_type.isUninstantiated) { continue; }
                    candidates.push_back(&class_type);
                }
            }

            if (candidates.empty()) {
                PK_FATAL("No concrete transaction class extending uvm_sequence_item found in %s", target_file.c_str());
            }
            if (candidates.size() > 1) {
                PK_FATAL("Multiple transaction classes extending uvm_sequence_item found in %s; only one is supported per file",
                         target_file.c_str());
            }

            return *candidates.front();
        }

        uvm_transaction_define convert_class_to_transaction(const ClassType &class_type, const std::string &filepath)
        {
            uvm_transaction_define transaction;
            transaction.name = std::string(class_type.name);
            transaction.filepath = filepath;
            transaction.version = picker::version();
            transaction.data_now = picker::fmtnow();

            for (const auto &property : class_type.membersOfType<ClassPropertySymbol>()) {
                transaction.parameters.push_back(convert_property_to_parameter(class_type, property));
            }

            return transaction;
        }

        void parse_sv_transactions_impl(const picker::pack_opts &opts,
                                        std::vector<picker::uvm_transaction_define> &transactions)
        {
            auto targets = collect_transaction_target_files(opts);
            if (targets.empty()) {
                PK_FATAL("No transaction files specified. Use 'file' argument or -f/--filelist option.");
            }

            Driver driver;
            prepare_driver(driver, opts);

            std::unique_ptr<Compilation> compilation;
            try {
                compilation = driver.createCompilation();
            } catch (const std::exception &e) {
                PK_FATAL("Failed to create slang compilation for transactions: %s", e.what());
            }

            auto diags = filter_ignorable_uvm_stub_diagnostics(compilation->getAllDiagnostics());
            fatal_if_errors(driver.sourceManager, diags, "analyze transaction sources");

            transactions.clear();
            transactions.reserve(targets.size());

            for (const auto &target : targets) {
                const auto &class_type = select_transaction_class(*compilation, driver.sourceManager, target.normalized_path);
                transactions.push_back(convert_class_to_transaction(class_type, target.display_path));
            }
        }

    } // namespace

    uvm_transaction_define parse_sv(
        const std::string& filepath,
        const std::string& macro_path)
    {
        (void)macro_path;

        picker::pack_opts opts {};
        opts.files.push_back(filepath);

        std::vector<uvm_transaction_define> transactions;
        parse_sv_transactions_impl(opts, transactions);
        if (transactions.size() != 1) {
            PK_FATAL("Expected exactly one transaction in %s", filepath.c_str());
        }
        return transactions.front();
    }

    void parse_sv_transactions(const pack_opts& opts, std::vector<uvm_transaction_define>& transactions)
    {
        parse_sv_transactions_impl(opts, transactions);
    }


    /// Convert transaction definition to enriched JSON with metadata
    std::pair<inja::json, int> transaction_to_json(
        const uvm_transaction_define& trans,
        const std::string& trans_filename,
        bool is_multi_transaction)
    {
        bool is_rtl = trans.filepath.empty();

        inja::json trans_data;
        trans_data["name"] = trans.name;
        trans_data["filename"] = trans_filename;
        trans_data[TemplateVars::FILEPATH] = is_rtl ? trans.name + "_trans.sv" : trans.filepath;
        trans_data[TemplateVars::CLASS_NAME] = is_rtl ? trans.name + "_trans" : trans.name;
        trans_data[TemplateVars::FROM_RTL] = is_rtl;
        trans_data[TemplateVars::VARIABLES] = inja::json::array();

        int byte_offset = 0;
        int total_bytes = 0;

        // Convert each parameter to enriched field data
        for (const auto& param : trans.parameters) {
            inja::json field;
            field["name"] = param.name;
            field["byte_count"] = param.byte_count;
            field["bit_count"] = param.bit_count;
            field["macro"] = param.is_macro;
            field["macro_name"] = param.macro_name;

            // Add transaction qualifier for multi-transaction mode
            if (is_multi_transaction) {
                field["transaction_name"] = trans.name;
            }

            // Compute serialization metadata
            field["byte_offset"] = byte_offset;
            switch(param.byte_count) {
                case 1: field["struct_fmt"] = "B"; field["is_standard_aligned"] = true; break;
                case 2: field["struct_fmt"] = "H"; field["is_standard_aligned"] = true; break;
                case 4: field["struct_fmt"] = "I"; field["is_standard_aligned"] = true; break;
                case 8: field["struct_fmt"] = "Q"; field["is_standard_aligned"] = true; break;
                default: field["struct_fmt"] = ""; field["is_standard_aligned"] = false; break;
            }
            byte_offset += param.byte_count;

            trans_data[TemplateVars::VARIABLES].push_back(field);
            total_bytes += param.byte_count;
        }

        trans_data[TemplateVars::BYTE_STREAM_COUNT] = total_bytes;
        return {trans_data, total_bytes};
    }

    /// Enrich template data with shortcuts for single-transaction mode
    static void enrich_single_transaction_template_data(inja::json& data, const uvm_transaction_define& trans)
    {
        if (trans.filepath.empty()) {
            // RTL-generated transaction
            data[TemplateVars::FILEPATH] = trans.name + "_trans.sv";
            data[TemplateVars::MODULE_NAME] = trans.name;
            data[TemplateVars::TRANS_CLASS_NAME] = trans.name + "_trans";
        } else {
            // User-provided transaction
            data[TemplateVars::FILEPATH] = trans.filepath;
            data[TemplateVars::MODULE_NAME] = trans.name;
            data[TemplateVars::TRANS_CLASS_NAME] = trans.name;
        }

        // Add template shortcuts (so templates can use both data.variables and data.transactions[0].variables)
        data["trans"] = data[TemplateVars::TRANSACTIONS][0];
        data["variables"] = data[TemplateVars::TRANSACTIONS][0]["variables"];
        data["trans_class_name"] = data[TemplateVars::TRANS_CLASS_NAME];
        data["byte_stream_count"] = data[TemplateVars::BYTE_STREAM_COUNT];
    }

    /// Prepare complete UVM package template data
    inja::json prepare_uvm_package_data(
        const std::vector<uvm_transaction_define>& transactions,
        const std::vector<std::string>& filenames,
        const std::string& package_name,
        bool generate_dut,
        const std::string& rtl_file_path)
    {
        inja::json data;
        // Basic package info
        data[TemplateVars::PACKAGE_NAME] = package_name;
        data[TemplateVars::VERSION] = transactions[0].version;
        data[TemplateVars::DATE_NOW] = transactions[0].data_now;
        data[TemplateVars::USE_TYPE] = 1;
        data[TemplateVars::GENERATE_DUT] = generate_dut;

        // Setup RTL file path
        if (!rtl_file_path.empty()) {
            std::filesystem::path rtl_abs = std::filesystem::absolute(rtl_file_path);
            std::filesystem::path uvmpy_abs = std::filesystem::absolute("uvmpy");
            std::filesystem::path rtl_relative = std::filesystem::relative(rtl_abs, uvmpy_abs);
            data[TemplateVars::RTL_FILE_PATH] = rtl_relative.string();
        } else {
            data[TemplateVars::RTL_FILE_PATH] = "";
        }

        // Initialize arrays
        data[TemplateVars::TRANSACTIONS] = inja::json::array();
        data[TemplateVars::VARIABLES] = inja::json::array();

        // Process all transactions (Unified Loop)
        int total_byte_count = 0;
        for (size_t i = 0; i < transactions.size(); i++) {
            // Always treat as part of a list for the codegen logic
            auto [trans_data, trans_bytes] = transaction_to_json(
                transactions[i],
                filenames[i],
                true // Always use multi-transaction style metadata
            );

            for (const auto& var : trans_data[TemplateVars::VARIABLES]) {
                data[TemplateVars::VARIABLES].push_back(var);
            }

            data[TemplateVars::TRANSACTIONS].push_back(trans_data);
            total_byte_count += trans_bytes;
        }

        data[TemplateVars::BYTE_STREAM_COUNT] = total_byte_count;
        data[TemplateVars::TRANSACTION_COUNT] = transactions.size();

        // Add template shortcuts for single-transaction mode
        if (transactions.size() == 1) {
            enrich_single_transaction_template_data(data, transactions[0]);
        }

        return data;
    }

    /// Convert a single RTL port definition to UVM transaction parameter
    uvm_parameter sv_signal_to_uvm_parameter(const sv_signal_define &signal)
    {
        uvm_parameter param;
        param.name = signal.logic_pin;
        param.is_macro = 0;
        param.macro_name = "";
        param.current_index = "0";

        // Calculate bit width
        if (signal.logic_pin_hb == -1) {
            // Single-bit signal
            param.bit_count = 1;
            param.byte_count = 1;
        } else {
            // Multi-bit signal: [hb:lb]
            param.bit_count = signal.logic_pin_hb - signal.logic_pin_lb + 1;
            param.byte_count = bits_to_bytes(param.bit_count);
        }

        return param;
    }

    /// Convert RTL module definition to UVM transaction definition
    uvm_transaction_define sv_module_to_uvm_transaction(const sv_module_define &module)
    {
        uvm_transaction_define transaction;

        // Module name becomes transaction name prefix
        // For RTL-generated transactions: Adder -> Adder_trans
        transaction.name = module.module_name;
        transaction.version = picker::version();
        transaction.data_now = picker::fmtnow();

        // Empty filepath indicates RTL-generated transaction
        // Will be set to {module_name}_trans.sv by codegen
        transaction.filepath = "";

        // Convert all module ports to transaction parameters
        transaction.parameters.reserve(module.pins.size());
        for (const auto &signal : module.pins) {
            transaction.parameters.push_back(sv_signal_to_uvm_parameter(signal));
        }

        return transaction;
    }

    /// Parse RTL file and convert to UVM transaction
    uvm_transaction_define parse_rtl_file(
        const std::string& rtl_file_path, 
        std::string& module_name,
        const std::string& target_module_name)
    {
        PK_MESSAGE("RTL mode: generating transaction from %s", rtl_file_path.c_str());

        // Prepare export_opts for RTL parsing (reuse export's parser::sv)
        picker::export_opts rtl_opts;
        rtl_opts.file.push_back(rtl_file_path);
        
        // If target_module_name is provided, use it to filter during RTL parsing
        if (!target_module_name.empty()) {
            rtl_opts.source_module_name_list.push_back(target_module_name);
        }

        // Parse RTL file using export's sv parser (it will handle module search and errors)
        std::vector<picker::sv_module_define> sv_module_result;
        try {
            sv(rtl_opts, sv_module_result);
        } catch (const std::exception &e) {
            PK_FATAL("Failed to parse RTL file: %s", e.what());
        }

        // sv() function ensures sv_module_result is not empty if target was specified
        const auto &target_module = sv_module_result[0];
        module_name = target_module.module_name;

        PK_MESSAGE("Using module: %s with %d ports",
                   target_module.module_name.c_str(), (int)target_module.pins.size());

        if (target_module.pins.empty()) {
            PK_MESSAGE("Warning: Module %s has no ports", target_module.module_name.c_str());
        }

        // Convert RTL to transaction definition
        return sv_module_to_uvm_transaction(target_module);
    }

}} // namespace picker::parser
