#include "picker.hpp"
#include "filelist.hpp"
#include "parser/sv.hpp"

#include "rtl_config.hpp"

#include "slang/ast/Compilation.h"
#include "slang/ast/ASTVisitor.h"
#include "slang/ast/SemanticFacts.h"
#include "slang/ast/symbols/CompilationUnitSymbols.h"
#include "slang/ast/symbols/BlockSymbols.h"
#include "slang/ast/symbols/SubroutineSymbols.h"
#include "slang/ast/symbols/InstanceSymbols.h"
#include "slang/ast/symbols/ParameterSymbols.h"
#include "slang/ast/symbols/PortSymbols.h"
#include "slang/ast/symbols/VariableSymbols.h"
#include "slang/ast/types/AllTypes.h"
#include "slang/diagnostics/DiagnosticEngine.h"
#include "slang/diagnostics/Diagnostics.h"
#include "slang/driver/Driver.h"
#include "slang/numeric/ConstantValue.h"
#include "slang/syntax/SyntaxTree.h"
#include "slang/syntax/SyntaxVisitor.h"
#include "slang/text/SourceManager.h"
#include "slang/util/CommandLine.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <set>
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
        using slang::ast::FixedSizeUnpackedArrayType;
        using slang::ast::InstanceSymbol;
        using slang::ast::PackedArrayType;
        using slang::ast::ParameterSymbol;
        using slang::ast::PortSymbol;
        using slang::ast::SymbolKind;
        using slang::ast::Type;
        using slang::CommandLine;
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

        // Single dash names are asked as '--name': slang looks both up in the same table,
        // but only the single dash form falls back to prefix matching, which would
        // silently turn '-full64' into '-f ull64'.
        bool slang_knows_option(Driver &probe, const std::string &name)
        {
            if (name.empty()) { return false; }

            std::string arg = name;
            if (arg.starts_with("-") && !arg.starts_with("--")) { arg = "-" + arg; }

            const size_t before = probe.cmdLine.getErrors().size();
            CommandLine::ParseOptions parse_opts;
            parse_opts.ignoreProgramName = true;
            parse_opts.ignoreDuplicates  = true;
            probe.cmdLine.parse(arg, parse_opts);

            for (const auto &error : probe.cmdLine.getErrors().subspan(before)) {
                if (error.message.find("unknown command line argument") != std::string::npos) { return false; }
            }
            return true;
        }

        // '--cmd-ignore <name>,<N>' rules for every filelist arg slang does not know.
        std::vector<std::string> collect_cmd_ignore_rules(const std::vector<std::string> &filelists)
        {
            std::vector<picker::filelist::token> tokens;
            picker::filelist::expand(filelists, tokens);

            Driver probe;
            probe.addStandardArgs();

            std::map<std::string, int> rules;
            std::set<std::string> probed;
            for (const auto &token : tokens) {
                if (token.kind != picker::filelist::token_kind::arg || token.name.empty()) { continue; }
                if (!probed.insert(token.name).second) { continue; }
                if (slang_knows_option(probe, token.name)) {
                    PK_DEBUG("slang knows filelist arg: %s", token.name.c_str());
                    continue;
                }
                rules[token.name] = token.values;
            }

            std::vector<std::string> ret;
            for (const auto &[name, values] : rules) {
                PK_DEBUG("Hide filelist arg from slang: %s,%d", name.c_str(), values);
                ret.push_back(name + "," + std::to_string(values));
            }
            return ret;
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

        nlohmann::json describe_port_type(const Type &source, const std::string &path, unsigned depth = 0)
        {
            if (depth >= 128) { PK_FATAL("Port type nesting exceeds 128 at %s", path.c_str()); }
            const auto &type = source.getCanonicalType();
            using J = nlohmann::json;
            J result{{"kind", "bits"}, {"packed", true}, {"width", 0},
                     {"signed", type.isSigned()}, {"four_state", type.isFourState()}};
            const auto kind = type.kind;
            if (kind == SymbolKind::FixedSizeUnpackedArrayType || kind == SymbolKind::PackedArrayType) {
                const bool packed = kind == SymbolKind::PackedArrayType;
                const auto &element = packed ? type.as<PackedArrayType>().elementType :
                                               type.as<FixedSizeUnpackedArrayType>().elementType;
                const auto range = packed ? type.as<PackedArrayType>().range :
                                             type.as<FixedSizeUnpackedArrayType>().range;
                result["kind"] = "array";
                result["packed"] = packed;
                result["left"] = range.left;
                result["right"] = range.right;
                result["element"] = describe_port_type(element, path + "[]", depth + 1);
            } else if (kind == SymbolKind::PackedStructType || kind == SymbolKind::UnpackedStructType) {
                const bool packed = kind == SymbolKind::PackedStructType;
                result["kind"] = "struct";
                result["packed"] = packed;
                result["fields"] = J::array();
                auto append = [&](const slang::ast::FieldSymbol &field) {
                    J member{{"name", std::string(field.name)},
                             {"type", describe_port_type(field.getType(), path + "." + std::string(field.name), depth + 1)}};
                    if (packed) member["offset"] = field.bitOffset;
                    result["fields"].push_back(std::move(member));
                };
                if (packed) {
                    for (const auto &field : type.as<slang::ast::PackedStructType>().membersOfType<slang::ast::FieldSymbol>())
                        append(field);
                } else {
                    for (const auto *field : type.as<slang::ast::UnpackedStructType>().fields) append(*field);
                }
            } else if (kind != SymbolKind::ScalarType && kind != SymbolKind::PredefinedIntegerType &&
                       kind != SymbolKind::EnumType) {
                PK_FATAL("Port %s uses unsupported type '%s'; supported leaves are fixed integral values, "
                         "packed structs/arrays, and fixed unpacked structs/arrays.",
                         path.c_str(), type.toString().c_str());
            }
            if (result["packed"].get<bool>()) {
                const auto width = type.getBitWidth();
                if (!width || width > uint64_t(INT32_MAX)) {
                    PK_FATAL("Port %s packed width exceeds the supported int range", path.c_str());
                }
                result["width"] = kind == SymbolKind::ScalarType ? 0 : width;
            }
            return result;
        }

        std::pair<int, int> extract_port_range(const Type &type)
        {
            const auto &canonical = type.getCanonicalType();
            if (canonical.kind == SymbolKind::ScalarType) { return {-1, 0}; }

            const auto &packed = canonical.as<PackedArrayType>();
            return {packed.range.left, packed.range.right};
        }

        // No storage assumptions here: unpacked members are resolved against
        // the generated C++ types later, with compiler-computed offsets.
        bool supports_native_layout(const Type &source, unsigned depth = 0)
        {
            if (depth >= 120) return false;
            const auto &type = source.getCanonicalType();
            switch (type.kind) {
            case SymbolKind::ScalarType:
            case SymbolKind::PredefinedIntegerType:
            case SymbolKind::EnumType:
                return type.getBitWidth() > 0 && type.getBitWidth() <= uint64_t(INT32_MAX);
            case SymbolKind::PackedArrayType:
                return type.getBitWidth() <= uint64_t(INT32_MAX) &&
                       supports_native_layout(type.as<PackedArrayType>().elementType, depth + 1);
            case SymbolKind::FixedSizeUnpackedArrayType:
                return supports_native_layout(type.as<FixedSizeUnpackedArrayType>().elementType, depth + 1);
            case SymbolKind::PackedStructType:
                if (type.getBitWidth() > uint64_t(INT32_MAX)) return false;
                for (const auto &field : type.as<slang::ast::PackedStructType>().membersOfType<slang::ast::FieldSymbol>())
                    if (!supports_native_layout(field.getType(), depth + 1)) return false;
                return true;
            case SymbolKind::UnpackedStructType:
                for (const auto *field : type.as<slang::ast::UnpackedStructType>().fields)
                    if (!supports_native_layout(field->getType(), depth + 1)) return false;
                return true;
            default:
                return false;
            }
        }

        struct NativeLayoutCollector : slang::ast::ASTVisitor<NativeLayoutCollector> {
            std::vector<std::pair<std::string, std::string>> &layouts;
            std::vector<std::pair<std::string, std::string>> &types;
            std::map<const Type *, std::string> cache;
            std::map<std::string, std::string> interned;
            explicit NativeLayoutCollector(picker::sv_module_define &module)
                : layouts(module.native_layouts), types(module.native_types) {}
            std::string describe(const Type &source) {
                const auto &type = source.getCanonicalType();
                if (auto found = cache.find(&type); found != cache.end()) return found->second;
                using J = nlohmann::json;
                J node{{"kind", "bits"}, {"packed", true},
                       {"width", type.kind == SymbolKind::ScalarType ? 0 : type.getBitWidth()},
                       {"signed", type.isSigned()}, {"four_state", type.isFourState()}};
                if (type.kind == SymbolKind::FixedSizeUnpackedArrayType ||
                    (type.kind == SymbolKind::PackedArrayType && !is_supported_flat_port_type(type))) {
                    const bool packed = type.kind == SymbolKind::PackedArrayType;
                    const auto range = packed ? type.as<PackedArrayType>().range : type.as<FixedSizeUnpackedArrayType>().range;
                    const auto &element = packed ? type.as<PackedArrayType>().elementType : type.as<FixedSizeUnpackedArrayType>().elementType;
                    node["kind"] = "array"; node["packed"] = packed;
                    if (!packed) node["width"] = 0;
                    node["left"] = range.left; node["right"] = range.right;
                    node["element"] = describe(element);
                } else if (type.kind == SymbolKind::PackedStructType || type.kind == SymbolKind::UnpackedStructType) {
                    const bool packed = type.kind == SymbolKind::PackedStructType;
                    node["kind"] = "struct"; node["packed"] = packed;
                    if (!packed) node["width"] = 0;
                    node["fields"] = J::array();
                    auto append = [&](const slang::ast::FieldSymbol &field) {
                        J member{{"name", std::string(field.name)}, {"type", describe(field.getType())}};
                        if (packed) member["offset"] = field.bitOffset;
                        node["fields"].push_back(std::move(member));
                    };
                    if (packed) for (const auto &field : type.as<slang::ast::PackedStructType>().membersOfType<slang::ast::FieldSymbol>()) append(field);
                    else for (const auto *field : type.as<slang::ast::UnpackedStructType>().fields) append(*field);
                } else if (type.kind == SymbolKind::PackedArrayType) {
                    // A bit vector is one value, but retain its declared range
                    // in the semantic type table rather than inventing [W-1:0].
                    const auto range = type.as<PackedArrayType>().range;
                    node["left"] = range.left; node["right"] = range.right;
                }
                const auto text = node.dump();
                auto [found, inserted] = interned.emplace(text, "t" + std::to_string(types.size()));
                if (inserted) types.emplace_back(found->second, text);
                cache.emplace(&type, found->second);
                return found->second;
            }
            void collect(const slang::ast::ValueSymbol &symbol) {
                const auto &type = symbol.getType();
                if (!supports_native_layout(type)) return;
                auto path = symbol.getHierarchicalPath();
                // Root scalar/vector metadata is useful too: signedness is not
                // recoverable from Verilator's unsigned CData/IData aliases.
                layouts.emplace_back(path, describe(type));
            }
            void handle(const slang::ast::VariableSymbol &symbol) { collect(symbol); }
            void handle(const slang::ast::NetSymbol &symbol) { collect(symbol); }
            void handle(const slang::ast::SubroutineSymbol &) {} // no automatic locals
            void handle(const slang::ast::GenerateBlockSymbol &block) {
                if (!block.isUninstantiated) visitDefault(block);
            }
        };

        void append_simple_port(std::vector<picker::sv_signal_define> &pins, const PortSymbol &port,
                                const std::string &module_name)
        {
            auto pin_type = port_direction_to_logic_pin_type(port.direction, module_name, std::string(port.name));
            const auto &type = port.getType();
            const Type *element_type = &type.getCanonicalType();
            std::vector<std::pair<int, int>> dimensions;
            while (element_type->kind == SymbolKind::FixedSizeUnpackedArrayType) {
                const auto &array = element_type->as<FixedSizeUnpackedArrayType>();
                dimensions.emplace_back(array.range.left, array.range.right);
                element_type = &array.elementType.getCanonicalType();
            }
            if (!is_supported_flat_port_type(*element_type)) {
                const auto layout = describe_port_type(type, module_name + "." + std::string(port.name));
                if (pin_type == "inout" && !layout["packed"].get<bool>()) {
                    PK_FATAL("Module %s port %s uses an unsupported inout unpacked aggregate",
                             module_name.c_str(), std::string(port.name).c_str());
                }
                const bool packed = layout["packed"].get<bool>();
                const int width = layout["width"].get<int>();
                picker::sv_signal_define pin{std::string(port.name), pin_type, packed ? width - 1 : -1, 0};
                pin.type_layout = std::make_shared<const std::string>(layout.dump());
                pin.unpacked_element_type = std::string(type.isFourState() ? "logic" : "bit") +
                                            (type.isSigned() ? " signed" : "");
                pins.push_back(std::move(pin));
                return;
            }
            if (!dimensions.empty() && pin_type == "inout") {
                PK_FATAL("Module %s port %s uses an unsupported inout unpacked array; "
                         "only input/output unpacked arrays are supported.",
                         module_name.c_str(), std::string(port.name).c_str());
            }

            auto [high_bit, low_bit] = extract_port_range(*element_type);
            // Packed element types are equivalent by width, signing and state kind.
            // Normalize their range for XData (also avoids the scalar -1 sentinel).
            if (!dimensions.empty() && element_type->kind == SymbolKind::PackedArrayType) {
                high_bit = int(element_type->getBitWidth()) - 1;
                low_bit = 0;
            }
            auto element_name = std::string(element_type->isFourState() ? "logic" : "bit");
            if (element_type->isSigned()) { element_name += " signed"; }
            pins.push_back({std::string(port.name), pin_type, high_bit, low_bit,
                            std::move(dimensions), element_name});
        }

        // Render an elaborated parameter as a literal that stands on its own in
        // the generated wrapper: unknown bits are kept exact and aggregate
        // values use assignment patterns, so the wrapper does not need the
        // original scope (packages, typedefs) to be visible.
        std::string render_parameter_value(const ParameterSymbol &parameter, const std::string &module_name)
        {
            const auto &value = parameter.getValue();
            if (value.bad()) {
                PK_FATAL("Parameter '%s' of module %s has no elaborated constant value",
                         std::string(parameter.name).c_str(), module_name.c_str());
            }
            return value.toString(slang::SVInt::DefaultStringAbbreviationThresholdBits,
                                  /*exactUnknowns=*/true, /*useAssignmentPatterns=*/true);
        }

        bool definition_parameter_has_default(const slang::ast::InstanceBodySymbol &body,
                                              std::string_view name)
        {
            for (const auto &declaration : body.getDefinition().parameters) {
                if (declaration.name == name) { return declaration.hasDefault(); }
            }
            // Unknown declaration metadata: prefer an explicit, equivalent value.
            return false;
        }

        // ParameterSymbol::isOverridden() only tracks instantiation-time
        // initializers. Command-line -G overrides and defparams are applied
        // through the hierarchy override tree, so check that as well.
        bool parameter_is_overridden(const slang::ast::InstanceBodySymbol &body,
                                     const ParameterSymbol &parameter)
        {
            if (parameter.isOverridden()) { return true; }
            const auto *overrides = body.hierarchyOverrideNode;
            const auto *syntax = parameter.getSyntax();
            return overrides && syntax &&
                   overrides->paramOverrides.find(syntax) != overrides->paramOverrides.end();
        }

        void append_module_parameters(std::vector<picker::sv_parameter_define> &parameters,
                                      const slang::ast::InstanceBodySymbol &body,
                                      const std::string &module_name)
        {
            std::vector<const ParameterSymbol *> port_parameters;
            bool needs_explicit_assignments = false;
            for (const auto *parameter : body.getParameters()) {
                // Type parameters cannot be expressed by a value assignment, and
                // slang never applies -G to them. Their instantiation default is
                // therefore already the elaborated value.
                if (parameter->symbol.kind == SymbolKind::TypeParameter) { continue; }
                // Localparams are not overridable; naming them in #() is illegal.
                if (parameter->isLocalParam()) { continue; }

                const auto &symbol = parameter->symbol.as<ParameterSymbol>();
                if (!parameter->isPortParam()) {
                    // Body parameters cannot be overridden by a named parameter
                    // assignment. If slang applied an override (CLI, defparam or
                    // config), the generated model would silently diverge from
                    // the elaboration used to derive the interface.
                    if (parameter_is_overridden(body, symbol)) {
                        PK_FATAL("Parameter '%s' of module %s is declared in the module body. "
                                 "Picker cannot carry an override of a body parameter into the "
                                 "generated wrapper; move it into the parameter port list.",
                                 std::string(parameter->symbol.name).c_str(), module_name.c_str());
                    }
                    continue;
                }
                port_parameters.push_back(&symbol);
                if (parameter_is_overridden(body, symbol) ||
                    !definition_parameter_has_default(body, parameter->symbol.name)) {
                    // A required parameter cannot elaborate without an explicit
                    // value, and an override must be reproduced in the wrapper.
                    needs_explicit_assignments = true;
                }
            }

            // Keep the generated text byte-for-byte identical when Verilator
            // would evaluate exactly the same defaults on the copied RTL.
            if (!needs_explicit_assignments) { return; }

            // Once the wrapper needs a parameter list, pin every port parameter
            // so the instantiation is fully equivalent to the elaboration that
            // produced the port signatures.
            for (const auto *parameter : port_parameters) {
                parameters.push_back({std::string(parameter->name),
                                      render_parameter_value(*parameter, module_name)});
            }
        }

        // Every requested --rtl-param must map to a parameter this wrapper can
        // actually pin. Anything else would elaborate differently from the
        // generated model, so it is rejected instead of silently ignored.
        void validate_requested_parameters(const std::vector<std::string> &requested_raw,
                                           const std::vector<const InstanceSymbol *> &instances)
        {
            const auto requested = picker::rtl::parse_params(requested_raw, "--rtl-param");
            if (requested.empty()) { return; }

            for (const auto &[name, value] : requested) {
                (void)value;
                bool assignable = false;
                std::vector<std::string> available;
                for (const auto *instance : instances) {
                    const auto module_name = std::string(instance->name);
                    for (const auto *parameter : instance->body.getParameters()) {
                        const auto parameter_name = std::string(parameter->symbol.name);
                        if (parameter->symbol.kind == SymbolKind::TypeParameter) {
                            if (parameter_name == name) {
                                PK_FATAL("--rtl-param %s: '%s' is a type parameter of module %s. Type "
                                         "parameters cannot be set by a value; give it a default in the RTL.",
                                         name.c_str(), name.c_str(), module_name.c_str());
                            }
                            continue;
                        }
                        if (parameter->isLocalParam()) {
                            if (parameter_name == name) {
                                PK_FATAL("--rtl-param %s: '%s' is a localparam of module %s and cannot be "
                                         "overridden.", name.c_str(), name.c_str(), module_name.c_str());
                            }
                            continue;
                        }
                        if (!parameter->isPortParam()) {
                            if (parameter_name == name) {
                                PK_FATAL("--rtl-param %s: '%s' is declared in the body of module %s. A body "
                                         "parameter cannot be set by an instance parameter assignment and "
                                         "Verilator rejects overriding it; move it into the parameter port "
                                         "list, or use --rtl-define for a macro.",
                                         name.c_str(), name.c_str(), module_name.c_str());
                            }
                            continue;
                        }
                        available.push_back(module_name + "." + parameter_name);
                        if (parameter_name == name) { assignable = true; }
                    }
                }
                if (assignable) { continue; }

                std::string candidates;
                for (const auto &candidate : available) {
                    if (!candidates.empty()) { candidates += ", "; }
                    candidates += candidate;
                }
                if (candidates.empty()) {
                    PK_FATAL("--rtl-param %s: the selected top module has no assignable parameters.",
                             name.c_str());
                }
                PK_FATAL("--rtl-param %s: no parameter with this name. Assignable parameters: %s",
                         name.c_str(), candidates.c_str());
            }
        }

        picker::sv_module_define convert_instance_to_module_define(const InstanceSymbol &instance, int module_nums)
        {
            picker::sv_module_define result;
            result.module_name = std::string(instance.name);
            result.module_nums = module_nums;
            append_module_parameters(result.parameters, instance.body, result.module_name);

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

            std::vector<std::string> args = {"picker-slang", "--single-unit", "--ignore-unknown-modules",
                                             "--exclude-ext", "so,a,o,cpp,c,cc,cxx"};
            if (opts.sim == "vcs") {
                args.push_back("--compat");
                args.push_back("vcs");
            }

            // Ignore rules must come before the filelist that contains the args.
            if (use_filelists) {
                for (const auto &rule : collect_cmd_ignore_rules(opts.filelists)) {
                    args.push_back("--cmd-ignore");
                    args.push_back(rule);
                }
            }
            // Structured RTL configuration goes first: the frontend keeps the
            // first definition of a macro (and the first -G), so an explicit
            // --rtl-define/--rtl-param wins over a raw -S passthrough, and the
            // generated filelist drops the macros overridden here.
            for (const auto &define : picker::rtl::parse_defines(opts.rtl_define, "--rtl-define")) {
                args.push_back(picker::rtl::slang_define_argument(define));
            }
            for (const auto &[name, value] : picker::rtl::parse_params(opts.rtl_param, "--rtl-param")) {
                args.push_back("-G" + name + "=" + value);
            }
            for (const auto &flag : opts.sflag) { args.push_back(flag); }

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

            std::vector<const InstanceSymbol *> selected_instances;
            for (const auto &module_name : resolved_module_names) {
                auto iter = top_instances.find(module_name);
                if (iter == top_instances.end()) {
                    PK_FATAL("Module: %s not find in input file", module_name.c_str());
                }

                const auto *instance = iter->second;
                selected_instances.push_back(instance);
                const int module_nums = has_explicit_requests ? requested_module_counts.at(module_name) : 1;
                sv_module_result.push_back(convert_instance_to_module_define(*instance, module_nums));
                if (opts.rw_type == picker::SignalAccessType::MEM_DIRECT && opts.sim == "verilator") {
                    NativeLayoutCollector collector(sv_module_result.back());
                    instance->visit(collector);
                }
            }

            validate_requested_parameters(opts.rtl_param, selected_instances);

            return 0;
        }

    } // namespace

    // Collect .v/.sv files from --fs/--filelist options for module search
    void collect_verilog_from_filelists(const std::vector<std::string> &filelists,
                                        std::vector<std::string> &out)
    {
        namespace fs = std::filesystem;

        std::vector<picker::filelist::token> tokens;
        picker::filelist::expand(filelists, tokens);

        for (const auto &token : tokens) {
            // compile args and their values are not sources
            if (token.kind != picker::filelist::token_kind::path) { continue; }

            auto path = token.argv.front();
            if (!fs::exists(path) && !token.base_dir.empty()) { path = (fs::path(token.base_dir) / path).string(); }
            if (!fs::exists(path)) {
                // Not fatal here; filelist may contain generated paths not yet present
                continue;
            }

            if (fs::is_directory(path)) {
                for (auto const &dir_entry : fs::recursive_directory_iterator(path)) {
                    if (!dir_entry.is_regular_file()) continue;
                    auto p = dir_entry.path().string();
                    if (is_verilog_src(p)) { out.push_back(fs::absolute(p).string()); }
                }
            } else if (fs::is_regular_file(path) && is_verilog_src(path)) {
                out.push_back(fs::absolute(path).string());
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
