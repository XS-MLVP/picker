#include <unordered_set>
#include <sstream>
#include <iomanip>
#include <random>
#include "codegen/lib.hpp"
#include "codegen/render_tags.hpp"
#include "config_database.hpp"
#include "filelist.hpp"
#include "picker.hpp"
#include "codegen/sv.hpp"
#include "codegen/firrtl.hpp"

namespace picker { namespace codegen {

    void configure_array_bindings(const std::vector<picker::sv_signal_define> &pins, const picker::export_opts &opts,
                                  nlohmann::json &data)
    {
        auto arrays = nlohmann::json::array();
        std::set<std::string> seen;
        for (const auto &pin : pins) {
            if (pin.array_name.empty() || !seen.insert(pin.array_name).second) continue;
            nlohmann::json array;
            array["name"] = pin.array_name;
            array["width"] = pin.logic_pin_hb == -1 ? 0 : pin.logic_pin_hb - pin.logic_pin_lb + 1;
            std::string left = "{", right = "{", native = pin.array_name;
            auto strides = nlohmann::json::array();
            for (const auto &[l, r] : pin.unpacked_dimensions) {
                if (left.size() > 1) { left += ", "; right += ", "; }
                left += std::to_string(l);
                right += std::to_string(r);
                native += "[0]";
                strides.push_back(native);
            }
            array["left"] = left + "}";
            array["right"] = right + "}";
            array["native_first"] = native;
            array["native_strides"] = strides;
            if (!pin.binding_native.empty()) {
                array["native_first"] = pin.binding_native;
                array["native_strides"] = pin.binding_strides;
            }
            if (opts.sim == "verilator") {
                array["native_first"] = verilator_member_expression(array["native_first"].get<std::string>());
                for (auto &stride : array["native_strides"]) stride = verilator_member_expression(stride.get<std::string>());
            }
            arrays.push_back(array);
        }
        const bool enabled = !arrays.empty();
        const bool aggregates = std::any_of(pins.begin(), pins.end(),
            [](const auto &pin) { return bool(pin.type_layout); });
        data[tags::has_array_bindings]  = enabled;
        data[tags::has_aggregate_views] = aggregates;
        data[tags::has_vpi_views]       = opts.vpi;
        data[tags::config_model_id]     = opts.config_model_id;
        data[tags::array_ports]         = arrays;
        nlohmann::json layouts = nlohmann::json::object();
        for (const auto &pin : pins) {
            if (!pin.aggregate_name.empty() && pin.type_layout)
                layouts[pin.aggregate_name] = nlohmann::json::parse(*pin.type_layout);
        }
        if (opts.vpi && !opts.config_model_id.empty()) {
            // The language wrapper is rendered after the single config file.
            // Reuse its exact port references, including plain vector ranges,
            // rather than letting VPI infer normalized wrapper coordinates.
            config::db::Database input(opts.target_dir + "/" + opts.target_module_name + ".xscfg", opts.config_model_id);
            std::map<std::string, nlohmann::json> expanded;
            std::function<nlohmann::json(const std::string &)> expand = [&](const std::string &id) {
                if (const auto cached = expanded.find(id); cached != expanded.end()) return cached->second;
                const auto type = input.get('t', id);
                if (!type) throw std::invalid_argument("Missing configured port type: " + id);
                auto node = config::decode(*type);
                if (node.contains("element")) node["element"] = expand(node["element"].get<std::string>());
                if (node.contains("fields")) for (auto &field : node["fields"])
                    field["type"] = expand(field["type"].get<std::string>());
                expanded.emplace(id, node);
                return node;
            };
            if (auto ports = input.get('m', "port_types"))
                for (const auto &[name, id] : ports->object) {
                    auto type = expand(id.text);
                    // Ordinary VPI leaves have always normalized one bit to
                    // scalar XData. Keep that representation, not the wrapper's
                    // normalized declaration range, while associating its type.
                    if (type["kind"] == "bits" && type["width"] == 1) type["width"] = 0;
                    layouts[name] = std::move(type);
                }
        }
        data[tags::aggregate_layouts_json] = nlohmann::json(layouts.dump()).dump();
        data[tags::rw_type]                = opts.rw_type == picker::SignalAccessType::MEM_DIRECT ? "MEM_DIRECT" : "DPI";
        if ((enabled || aggregates) && !data.contains(tags::xspcomm_include)) {
            std::string error;
            auto include = picker::get_xcomm_lib("include", error);
            if (include.empty()) { PK_FATAL("%s\n", error.c_str()); }
            data[tags::xspcomm_include] = include;
        }
    }

    bool check_file_type(const std::string src, const std::vector<std::string> &types)
    {
        for (const auto &type : types) {
            if (src.ends_with(type)) { return true; }
        }
        return false;
    }

    void recursive_render(std::string &src_dir, std::string &dst_dir, nlohmann::json &data, inja::Environment &env)
    {
        if (!std::filesystem::create_directories(dst_dir)) {
            PK_FATAL("Create: %s fail, please check if it is already exists", dst_dir.c_str());
        };
        // Render all files in src_dir to dst_dir
        for (const auto &entry : std::filesystem::directory_iterator(src_dir)) {
            if (entry.is_regular_file()) {
                std::string src_filename, dst_filename, dst_file_content;
                src_filename = entry.path().filename().string();
                // In-place installs can leave templates removed by the new
                // package. Do not render obsolete sidecars or delete files
                // from the user's template installation.
                if (data.is_object() && data.value(tags::single_config_database, false) &&
                    (src_filename == "signals.json" || src_filename == "native_layouts.json")) continue;
                dst_filename = dst_dir + "/" + src_filename;
                PK_MESSAGE("Render file: %s to %s", src_filename.c_str(), dst_filename.c_str());
                dst_file_content = env.render_file(entry.path().string(), data);
                write_file(dst_filename, dst_file_content);
            } else if (entry.is_directory()) {
                std::string src_sub_dir, dst_sub_dir;
                src_sub_dir = entry.path().string();
                dst_sub_dir = dst_dir + "/" + entry.path().filename().string();
                recursive_render(src_sub_dir, dst_sub_dir, data, env);
            }
        }
    }

    static std::string quote_entry(const std::string &entry)
    {
        if (entry.find_first_of(" \t") == std::string::npos) { return entry; }
        return "\"" + entry + "\"";
    }

    struct exported_source {
        std::string source; // path as given on the command line
        std::string name;   // unique name inside the export directory
    };

    // The simulator only reads the export directory, so every positional RTL
    // source is copied there. Keep the original basename (the suffix selects
    // the language) and add a numeric suffix only for a collision. Names that
    // recursive_render writes must never be reused, or a user module could
    // replace picker's generated wrapper.
    static std::vector<exported_source> plan_exported_sources(const std::vector<std::string> &files,
                                                              const std::string &target_module_name,
                                                              const std::string &template_dir)
    {
        std::unordered_set<std::string> used;
        auto reserve = [&](const std::string &name) { used.insert(name); };
        reserve("top.v");
        reserve("top.sv");
        reserve(target_module_name + "_top.v");
        reserve(target_module_name + "_top.sv");
        reserve(target_module_name + ".xscfg");
        std::error_code error;
        std::filesystem::directory_iterator entries(template_dir, error);
        if (error) {
            PK_FATAL("Cannot read template directory '%s': %s", template_dir.c_str(), error.message().c_str());
        }
        for (const auto &entry : entries) {
            if (entry.is_regular_file()) { reserve(entry.path().filename().string()); }
        }

        std::unordered_set<std::string> seen;
        std::vector<exported_source> planned;
        for (const auto &file : files) {
            auto source = std::filesystem::path(file);
            // Identity follows symlinks, so `a.v` and a link to it are one
            // source; copying both would define every module twice.
            auto canonical = std::filesystem::weakly_canonical(source, error);
            const auto identity = (error ? std::filesystem::absolute(source, error) : canonical)
                                      .lexically_normal()
                                      .string();
            error.clear();
            if (!seen.insert(identity).second) { continue; } // the same file listed twice must not define a module twice
            auto filename = source.filename();
            if (filename.empty()) { PK_FATAL("RTL source '%s' does not name a file", file.c_str()); }
            std::string stem = filename.stem().string();
            std::string extension = filename.extension().string();
            // A missing suffix gives the simulators no language hint; Verilog
            // is the only sensible default for a picker source argument.
            if (extension.empty() || extension == ".") {
                if (stem.empty()) { stem = filename.string(); }
                extension = ".v";
            }
            std::string candidate = stem + extension;
            for (unsigned suffix = 1; used.count(candidate) != 0; ++suffix) {
                candidate = stem + "_" + std::to_string(suffix) + extension;
            }
            used.insert(candidate);
            planned.push_back({file, candidate});
        }
        return planned;
    }

    void gen_filelist(const std::vector<std::string> &source_file, const std::vector<std::string> &ifilelists,
                      std::string &ofilelist, std::vector<std::string> &incdirs)
    {
        const std::vector<std::string> allow_file_types = {
            ".sv", ".v", ".svh", ".vh", ".cpp", ".c", ".cc", ".cxx", ".so", ".a", ".o"};
        const std::vector<std::string> header_file_types = {".svh", ".vh"};
        std::unordered_set<std::string> incdir_set;
        auto resolve_input_path = [](const std::string &path, const std::string &base_dir) {
            auto resolved = std::filesystem::path(path);
            if (!base_dir.empty() && !resolved.is_absolute()) { resolved = std::filesystem::path(base_dir) / resolved; }
            resolved = resolved.lexically_normal();
            // lexically_normal() preserves a trailing separator for paths ending
            // in "/.". Keep generated filelists stable by removing it, except
            // when the resolved path is the filesystem root.
            if (resolved != resolved.root_path() && resolved.filename().empty()) {
                resolved = resolved.parent_path();
            }
            return resolved.string();
        };

        std::unordered_set<std::string> source_file_set;
        for (const auto &file : source_file) {
            auto normalized = std::filesystem::absolute(resolve_input_path(file, "")).lexically_normal().string();
            source_file_set.insert(normalized);
        }

        auto resolve_incdir = [&](const std::string &dir, const std::string &base_dir) {
            auto trimmed = picker::trim(dir);
            if (trimmed.empty()) { return std::string(); }
            return std::filesystem::absolute(resolve_input_path(trimmed, base_dir)).lexically_normal().string();
        };

        auto add_incdir = [&](const std::string &dir, const std::string &base_dir) {
            auto resolved = resolve_incdir(dir, base_dir);
            if (resolved.empty()) { return; }
            if (incdir_set.insert(resolved).second) { incdirs.push_back(resolved); }
        };

        auto add_file = [&](const std::string &path) {
            if (source_file_set.count(path) != 0) { return; } // skip source file
            if (check_file_type(path, header_file_types)) {
                add_incdir(std::filesystem::path(path).parent_path().string(), "");
                return;
            }
            ofilelist += quote_entry(path) + "\n";
        };

        std::vector<picker::filelist::token> tokens;
        picker::filelist::expand(ifilelists, tokens);

        for (const auto &token : tokens) {
            switch (token.kind) {
            case picker::filelist::token_kind::incdir: {
                std::string line;
                for (const auto &dir : token.argv) {
                    auto resolved = resolve_incdir(dir, token.base_dir);
                    if (resolved.empty()) { continue; }
                    if (token.name == "+incdir") {
                        line += (line.empty() ? "+incdir+" : "+") + resolved;
                    } else {
                        if (!line.empty()) { line += " "; }
                        line += "-I" + resolved;
                    }
                }
                if (!line.empty()) { ofilelist += quote_entry(line) + "\n"; }
                continue;
            }
            case picker::filelist::token_kind::arg: {
                std::string line;
                for (const auto &entry : token.argv) {
                    if (!line.empty()) { line += " "; }
                    line += quote_entry(entry);
                }
                ofilelist += line + "\n";
                continue;
            }
            case picker::filelist::token_kind::path:
                break;
            }

            auto path = token.argv.front();
            if (path.ends_with("/")) { // directory
                auto resolved_dir = resolve_input_path(path, token.base_dir);
                if (!std::filesystem::exists(resolved_dir)) PK_FATAL("Directory not found: %s\n", path.c_str());
                std::filesystem::recursive_directory_iterator iter(resolved_dir);
                for (const auto &entry : iter) {
                    if (entry.is_regular_file()) {
                        std::string filename = entry.path().filename().string();
                        if (check_file_type(filename, allow_file_types)) { add_file(entry.path().string()); }
                    }
                }
            } else {
                add_file(std::filesystem::absolute(resolve_input_path(path, token.base_dir))
                             .lexically_normal()
                             .string());
            }
        }
    }

    void append_incdirs_to_vflag(const std::string &simulator, const std::vector<std::string> &incdirs,
                                 std::string &vflag)
    {
        if (incdirs.empty()) { return; }
        std::string extra;
        for (const auto &dir : incdirs) {
            if (!extra.empty()) { extra += " "; }
            // vflag reaches CMake through the environment and is split with
            // separate_arguments(NATIVE_COMMAND), so a directory containing a
            // space must remain a single quoted argument.
            if (simulator == "verilator") {
                extra += quote_entry("-I" + dir);
            } else if (simulator == "vcs") {
                extra += quote_entry("+incdir+" + dir);
            } else if (simulator == "uvs") {
                extra += quote_entry("-include " + dir);
            }
        }
        if (extra.empty()) { return; }
        if (!vflag.empty()) { vflag += " "; }
        vflag += extra;
    }

    void get_clock_period(std::string &vcs_clock_period_h, std::string &vcs_clock_period_l,
                          const std::string &frequency)
    {
        // h,l with ps unit
        uint64_t freq, period;
        if (frequency.ends_with("KHz")) {
            freq   = std::stoull(frequency.substr(0, frequency.length() - 3));
            period = 1000000000 / freq;
        } else if (frequency.ends_with("MHz")) {
            freq   = std::stoull(frequency.substr(0, frequency.length() - 3));
            period = 1000000 / freq;
        } else if (frequency.ends_with("GHz")) {
            freq   = std::stoull(frequency.substr(0, frequency.length() - 3));
            period = 1000 / freq;
        } else if (frequency.ends_with("Hz")) {
            freq   = std::stoull(frequency.substr(0, frequency.length() - 2));
            period = 1000000000000 / freq;
        } else {
            PK_FATAL("Unsupported frequency unit: %s\n", frequency.c_str());
        } // end if
        vcs_clock_period_h = std::to_string((period >> 1) + (period & 1));
        vcs_clock_period_l = std::to_string(period >> 1);
    }

    static void apply_build_render_data(std::string &simulator, std::string &vflag, std::string &cflag,
                                        nlohmann::json &data)
    {
        data[tags::simulator] = simulator;
        data[tags::vflag]     = vflag;
        data[tags::cflag]     = cflag;
    }

    int vcs_coverage_metric_bit(const std::string &metric)
    {
        if (metric == "line") return 1 << 0;
        if (metric == "cond") return 1 << 1;
        if (metric == "fsm") return 1 << 2;
        if (metric == "tgl" || metric == "toggle") return 1 << 3;
        if (metric == "branch") return 1 << 4;
        if (metric == "assert" || metric == "assertion") return 1 << 5;
        if (metric == "all") return 0b111111;
        return 0;
    }

    int parse_vcs_coverage_metrics(const std::string &vflag, bool &has_cm)
    {
        std::string normalized = vflag;
        for (auto &ch : normalized) {
            if (ch == '"' || ch == '\'' || ch == ',' || ch == '+') ch = ' ';
        }

        std::vector<std::string> tokens;
        std::stringstream ss(normalized);
        for (std::string token; ss >> token;) tokens.push_back(token);

        int metrics = 0;
        for (size_t i = 0; i < tokens.size(); ++i) {
            const auto &token = tokens[i];
            if (token == "-cm") {
                has_cm = true;
                if (i + 1 < tokens.size()) metrics |= vcs_coverage_metric_bit(tokens[++i]);
                continue;
            }
            if (token.starts_with("-cm=")) {
                has_cm = true;
                metrics |= vcs_coverage_metric_bit(token.substr(token.find('=') + 1));
                continue;
            }
            if (has_cm) metrics |= vcs_coverage_metric_bit(token);
        }
        return metrics;
    }

    int uvs_coverage_metric_bit(const std::string &metric)
    {
        if (metric == "stmt") return 1 << 0;
        if (metric == "cond") return 1 << 1;
        if (metric == "fsm") return 1 << 2;
        if (metric == "tgl") return 1 << 3;
        if (metric == "assert") return 1 << 5;
        if (metric == "all") return 0b101111;
        return 0;
    }

    int parse_uvs_coverage_metrics(const std::string &vflag, bool &has_cov)
    {
        std::string normalized = vflag;
        for (auto &ch : normalized) {
            if (ch == '"' || ch == '\'' || ch == ',') ch = ' ';
        }

        int metrics = 0;
        bool in_cov = false;
        std::stringstream ss(normalized);
        for (std::string token; ss >> token;) {
            if (token == "-cov") {
                has_cov = in_cov = true;
            } else if (token[0] == '-') {
                in_cov = false;
            } else if (in_cov) {
                metrics |= uvs_coverage_metric_bit(token);
            }
        }
        return metrics;
    }

    void gen_coverage_metrics(std::string &simulator, picker::export_opts &opts, std::string &vflag,
                              nlohmann::json &data)
    {
        const bool &coverage = opts.coverage;
        // Bitmask for collected coverage metrics.
        //
        // Bit | Metric
        // ----|---------
        //  0  | line
        //  1  | cond
        //  2  | fsm
        //  3  | toggle
        //  4  | branch
        //  5  | assert
        int metrics = 0;
        if (coverage && simulator == "verilator") {
            // Verilator doesn't support fsm coverage
            metrics = 0b111011;
        } else if (coverage && simulator == "vcs") {
            bool has_cm = false;
            metrics     = parse_vcs_coverage_metrics(vflag, has_cm);
            if (!has_cm) {
                if (!vflag.empty()) vflag += " ";
                vflag += "-cm line+cond+fsm+tgl+branch+assert";
                metrics = 0b111111;
            }
        } else if (coverage && simulator == "uvs") {
            bool has_cov = false;
            metrics      = parse_uvs_coverage_metrics(vflag, has_cov);
            if (!has_cov) {
                if (!vflag.empty()) vflag += " ";
                // UVS has no branch metric. cont_assign counts assign statements,
                // which are most of the statements in Chisel-generated RTL.
                vflag += "-cov stmt,cond,tgl,fsm,assert -covopt cont_assign";
                metrics = 0b101111;
            }
        }

        data[tags::coverage]         = coverage ? "ON" : "OFF";
        data[tags::coverage_metrics] = metrics;
    }

    void gen_expins(nlohmann::json &expins, picker::export_opts &opts,
                    const std::vector<picker::sv_signal_define> &external_pins)
    {
        if (opts.rw_type != picker::SignalAccessType::MEM_DIRECT) { return; }
        for (const auto &pin : external_pins) {
            if (!pin.array_name.empty()) continue; // Arrays register one base/stride layout, not N element addresses.
            nlohmann::json expin;
            expin["name"] = pin.logic_pin;
            expin["native_name"] = verilator_identifier(pin.logic_pin);
            expin["type"] = pin_direction(pin.logic_pin_type);
            expin["hb"]   = pin.logic_pin_hb;
            expin["lb"]   = pin.logic_pin_lb;
            expin["size"] = pin.logic_pin_hb - pin.logic_pin_lb + 1;
            expins.push_back(expin);
        }
    }

    std::vector<picker::sv_signal_define> lib(picker::export_opts &opts,
                                              const std::vector<picker::sv_module_define> sv_module_result,
                                              const std::vector<picker::sv_signal_define> &internal_pin,
                                              nlohmann::json &signal_tree)
    {
        std::vector<picker::sv_signal_define> ret;
        // Parse Options
        std::string src_dir = opts.source_dir + "/lib", dst_dir = opts.target_dir,
                    dst_module_name = opts.target_module_name, wave_file_name = opts.wave_file_name,
                    simulator = opts.sim, vflag = opts.vflag, cflag = opts.cflag, ofilelist, vcs_clock_period_h,
                    vcs_clock_period_l;
        std::vector<std::string> files      = opts.file;
        std::vector<std::string> ifilelists = opts.filelists;

        // Build environment
        inja::Environment env;
        nlohmann::json data;

        data[tags::top_module_name]   = dst_module_name;
        data[tags::shared_lib_suffix] = get_shared_lib_suffix();

        // firrtl base simulators
        std::unordered_set<std::string> firrtl_simulators = {"gsim"};
        if (firrtl_simulators.count(opts.sim)) {
            ret = gen_firrtl_param(data, sv_module_result, internal_pin, signal_tree, wave_file_name, simulator,
                                   opts.rw_type);
            // verilog based simulators
        } else {
            ret = gen_sv_param(data, sv_module_result, internal_pin, signal_tree, wave_file_name, simulator,
                               opts.rw_type);
        }
        std::vector<std::string> incdirs;
        gen_filelist(files, ifilelists, ofilelist, incdirs);
        const bool firrtl_input = firrtl_simulators.count(opts.sim) != 0;
        std::vector<exported_source> exported_sources;
        if (!firrtl_input) {
            exported_sources = plan_exported_sources(files, dst_module_name, src_dir);
            // Positional sources used to be passed to the simulator as a
            // single <top>.v and were therefore skipped by gen_filelist. They
            // are now copied under unique names, so the simulator only sees
            // them through the generated filelist.
            const std::vector<std::string> header_types = {".svh", ".vh"};
            for (const auto &entry : exported_sources) {
                if (check_file_type(entry.name, header_types)) {
                    const auto dir = std::filesystem::absolute(dst_dir).lexically_normal().string();
                    if (std::find(incdirs.begin(), incdirs.end(), dir) == incdirs.end()) {
                        incdirs.push_back(dir);
                    }
                    continue;
                }
                const auto path = std::filesystem::absolute(std::filesystem::path(dst_dir) / entry.name)
                                      .lexically_normal()
                                      .string();
                ofilelist += quote_entry(path) + "\n";
            }
        }
        append_incdirs_to_vflag(simulator, incdirs, vflag);

        // Get coverage metrics before rendering Makefile/CMake, because VCS coverage may add -cm flags.
        gen_coverage_metrics(simulator, opts, vflag, data);
        apply_build_render_data(simulator, vflag, cflag, data);

        // Set clock period
        printf("Frequency: %s\n", opts.frequency.c_str());
        get_clock_period(vcs_clock_period_h, vcs_clock_period_l, opts.frequency);

        // Render expins info
        auto expins = nlohmann::json::array();
        gen_expins(expins, opts, ret);

        data[tags::module_external_pins]  = expins;
        data[tags::vcs_clock_period_high] = vcs_clock_period_h;
        data[tags::vcs_clock_period_low]  = vcs_clock_period_l;
        data[tags::verbose]               = opts.verbose ? "ON" : "OFF";
        data[tags::example]               = opts.example ? "ON" : "OFF";
        data[tags::checkpoints]           = opts.checkpoints ? "ON" : "OFF";
        data[tags::verdi_mode]            = opts.verdi_mode;
        data[tags::rw_type]               = opts.rw_type == picker::SignalAccessType::MEM_DIRECT ? "MEM_DIRECT" : "DPI";
        data[tags::target_language]       = opts.language;
        data[tags::build_threads] =
            opts.build_threads > 0 ? std::to_string(opts.build_threads) :
                                     "$(shell (nproc 2>/dev/null || sysctl -n hw.ncpu) 2>/dev/null)";
        data[tags::filelist]               = ofilelist;
        data[tags::lib_dpi_func_name_hash] = std::string(lib_random_hash);
        data[tags::generator_picker_path] =
            appimage::is_running_as_appimage() ?
                (getenv("APPIMAGE") != nullptr ? std::string(getenv("APPIMAGE")) : std::string()) :
                get_executable_path();
        data[tags::generator_template_path] = std::filesystem::absolute(opts.source_dir);

        // Render lib files
        configure_array_bindings(ret, opts, data);
        auto native_layouts = nlohmann::json{{"version", 1}, {"types", nlohmann::json::object()},
                                             {"signals", nlohmann::json::object()}};
        std::map<std::string, std::string> native_interned;
        for (const auto &module : sv_module_result) {
            std::map<std::string, std::string> remap;
            std::map<std::string, std::string> module_layouts(module.native_layouts.begin(), module.native_layouts.end());
            // Children are interned before parents by the RTL collector.
            for (const auto &[id, text] : module.native_types) {
                auto node = nlohmann::json::parse(text);
                if (node.contains("element")) node["element"] = remap.at(node["element"].get<std::string>());
                if (node.contains("fields")) for (auto &field : node["fields"])
                    field["type"] = remap.at(field["type"].get<std::string>());
                const auto [found, inserted] = native_interned.emplace(node.dump(), "t" + std::to_string(native_interned.size()));
                if (inserted) native_layouts["types"][found->second] = node;
                remap[id] = found->second;
            }
            for (int index = 0; index < module.module_nums; ++index) {
                const auto instance = module.module_name +
                    (module.module_nums == 1 ? "" : "_" + std::to_string(index));
                for (const auto &[path, layout] : module.native_layouts) {
                    if (!path.starts_with(module.module_name + ".")) continue;
                    const auto name = dst_module_name + "_top." + instance + path.substr(module.module_name.size());
                    native_layouts["signals"][name] = remap.at(layout);
                }
                // These wrapper connections are generated by us. Record both
                // sides explicitly; do not guess aliases by matching widths.
                auto prefix = (sv_module_result.size() == 1 && module.module_nums == 1) ? "" : module.module_name + "_";
                if (module.module_nums != 1) prefix += std::to_string(index) + "_";
                for (const auto &pin : module.pins) {
                    const auto found = module_layouts.find(module.module_name + "." + pin.logic_pin);
                    if (found != module_layouts.end())
                        native_layouts["signals"][dst_module_name + "_top." + prefix + pin.logic_pin] = remap.at(found->second);
                }
            }
        }
        // Port names are exact logical XPort keys, not host attributes or paths
        // reconstructed from the presentation-oriented metadata tree. Reuse the
        // same interned type section as native signals, including DPI projects.
        nlohmann::json port_types = nlohmann::json::object();
        std::function<std::string(nlohmann::json)> intern_port_type = [&](nlohmann::json node) {
            if (node.contains("element")) node["element"] = intern_port_type(node["element"]);
            if (node.contains("fields")) for (auto &field : node["fields"])
                field["type"] = intern_port_type(field["type"]);
            const auto [found, inserted] = native_interned.emplace(node.dump(), "t" + std::to_string(native_interned.size()));
            if (inserted) native_layouts["types"][found->second] = std::move(node);
            return found->second;
        };
        for (const auto &pin : ret) {
            if (!pin.aggregate_name.empty() && pin.type_layout) {
                if (!port_types.contains(pin.aggregate_name))
                    port_types[pin.aggregate_name] = intern_port_type(nlohmann::json::parse(*pin.type_layout));
            } else if (!pin.array_name.empty() && !port_types.contains(pin.array_name)) {
                const auto alias = dst_module_name + "_top." + pin.array_name;
                if (native_layouts["signals"].contains(alias)) {
                    port_types[pin.array_name] = native_layouts["signals"][alias];
                    continue;
                }
                nlohmann::json layout{{"kind", "bits"}, {"packed", true},
                    {"width", pin.logic_pin_hb == -1 ? 0 : pin.logic_pin_hb - pin.logic_pin_lb + 1},
                    {"signed", pin.unpacked_element_type.find("signed") != std::string::npos},
                    {"four_state", !pin.unpacked_element_type.starts_with("bit")}};
                for (auto it = pin.unpacked_dimensions.rbegin(); it != pin.unpacked_dimensions.rend(); ++it)
                    layout = {{"kind", "array"}, {"packed", false}, {"width", 0},
                        {"left", it->first}, {"right", it->second}, {"element", std::move(layout)}};
                port_types[pin.array_name] = intern_port_type(std::move(layout));
            } else if (pin.aggregate_name.empty() && pin.array_name.empty()) {
                const auto alias = dst_module_name + "_top." + pin.logic_pin;
                if (native_layouts["signals"].contains(alias))
                    port_types[signal_name(pin)] = native_layouts["signals"][alias];
            }
        }
        data[tags::single_config_database] = true;
        // The historical DPI hash is seeded by seconds. It is not sufficient
        // to distinguish two configuration exports made within the same tick.
        std::random_device entropy;
        std::ostringstream identity;
        identity << std::hex << std::setfill('0') << std::setw(16)
                  << config::db::hash(data.dump() + native_layouts.dump() + port_types.dump()) << '-';
        for (int i = 0; i < 4; ++i) identity << std::setw(8) << uint32_t(entropy());
        const auto model_id = identity.str();
        opts.config_model_id = model_id;
        data[tags::config_model_id] = model_id;
        recursive_render(src_dir, dst_dir, data, env);
        config::db::Writer database{model_id};
        auto ports = data[tags::signal_tree];
        if (ports.is_string()) {
            const auto parsed = nlohmann::json::parse(ports.get<std::string>(), nullptr, false);
            if (!parsed.is_discarded()) ports = parsed;
        }
        database.add('m', "ports", config::encode(ports));
        database.add('m', "port_types", config::encode(port_types));
        database.add('m', "generator", config::encode({{"simulator", opts.sim},
            {"rw_type", data[tags::rw_type]}, {"native_abi", PICKER_NATIVE_ABI}}));
        for (const auto &[id, type] : native_layouts["types"].items()) database.add('t', id, config::encode(type));
        for (const auto &[name, id] : native_layouts["signals"].items()) database.add('c', name, config::encode(id));
        database.write(dst_dir + "/" + dst_module_name + ".xscfg");

        // Copy verilog files. Each source keeps its own name so multiple
        // modules cannot overwrite one another; gsim still needs the FIRRTL
        // input named after the target module.
        for (const auto &entry : std::filesystem::directory_iterator(dst_dir)) {
            if (entry.is_regular_file()) {
                std::string src_filename;
                src_filename = entry.path().filename().string();
                if (src_filename.ends_with(".sv") || src_filename.ends_with(".v")) {
                    std::filesystem::rename(entry.path(),
                                            entry.path().parent_path()
                                                / (dst_module_name + "_" + entry.path().filename().string()));
                }
            }
        }
        if (firrtl_input) {
            for (auto &f : files) {
                auto ext = std::filesystem::path(f).extension().string();
                if (ext.empty() || ext == ".") { ext = ".fir"; }
                if (ext == ".sv") { ext = ".v"; }
                std::filesystem::copy_file(f, dst_dir + "/" + dst_module_name + ext,
                                           std::filesystem::copy_options::overwrite_existing);
            }
        } else {
            for (const auto &entry : exported_sources) {
                const auto destination = dst_dir + "/" + entry.name;
                std::error_code error;
                std::filesystem::copy_file(entry.source, destination, error);
                if (error) {
                    PK_FATAL("Failed to copy RTL source '%s' to '%s': %s", entry.source.c_str(),
                             destination.c_str(), error.message().c_str());
                }
            }
        }
        PK_MESSAGE("Generate DPI files successfully!");
        return ret;
    }

}} // namespace picker::codegen
