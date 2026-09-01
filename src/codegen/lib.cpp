#include <unordered_set>
#include <sstream>
#include "codegen/lib.hpp"
#include "filelist.hpp"
#include "picker.hpp"
#include "codegen/sv.hpp"
#include "codegen/firrtl.hpp"

namespace picker { namespace codegen {

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
            return resolved.lexically_normal().string();
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
            if (simulator == "verilator") {
                extra += "-I" + dir;
            } else if (simulator == "vcs") {
                extra += "+incdir+" + dir;
            } else if (simulator == "uvs") {
                extra += "-include " + dir;
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

    void gen_cmake(std::string &src_dir, std::string &dst_dir, std::string &wave_file_name, std::string &simulator,
                   std::string &vflag, std::string &cflag, inja::Environment &env, nlohmann::json &data)
    {
        data["__SIMULATOR__"] = simulator;
        data["__VFLAG__"]     = vflag;
        data["__CFLAG__"]     = cflag;

        std::string verilaotr_coverage, vcs_coverage;
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
        }

        data["__COVERAGE__"]         = coverage ? "ON" : "OFF";
        data["__COVERAGE_METRICS__"] = metrics;
    }

    void gen_expins(nlohmann::json &expins, picker::export_opts &opts,
                    const std::vector<picker::sv_signal_define> &external_pins)
    {
        if (opts.rw_type != picker::SignalAccessType::MEM_DIRECT) { return; }
        for (const auto &pin : external_pins) {
            nlohmann::json expin;
            expin["name"] = pin.logic_pin;
            expin["type"] = (pin.logic_pin_type[0] == 'i') ? "In" : "Out";
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

        data["__TOP_MODULE_NAME__"] = dst_module_name;
        data["__SHARED_LIB_SUFFIX__"] = get_shared_lib_suffix();

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
        append_incdirs_to_vflag(simulator, incdirs, vflag);

        // Get coverage metrics before rendering Makefile/CMake, because VCS coverage may add -cm flags.
        gen_coverage_metrics(simulator, opts, vflag, data);
        gen_cmake(src_dir, dst_dir, wave_file_name, simulator, vflag, cflag, env, data);

        // Set clock period
        printf("Frequency: %s\n", opts.frequency.c_str());
        get_clock_period(vcs_clock_period_h, vcs_clock_period_l, opts.frequency);

        // Render expins info
        auto expins = nlohmann::json::array();
        gen_expins(expins, opts, ret);

        data["__MODULE_EXTERNAL_PINS__"]  = expins;
        data["__VCS_CLOCK_PERIOD_HIGH__"] = vcs_clock_period_h;
        data["__VCS_CLOCK_PERIOD_LOW__"]  = vcs_clock_period_l;
        data["__VERBOSE__"]               = opts.verbose ? "ON" : "OFF";
        data["__EXAMPLE__"]               = opts.example ? "ON" : "OFF";
        data["__CHECKPOINTS__"]           = opts.checkpoints ? "ON" : "OFF";
        data["__VPI__"]                   = opts.vpi ? "ON" : "OFF";
        data["__VERDI_MODE__"]            = opts.verdi_mode;
        data["__RW_TYPE__"]               = opts.rw_type == picker::SignalAccessType::MEM_DIRECT ? "MEM_DIRECT" : "DPI";
        data["__TARGET_LANGUAGE__"]       = opts.language;
        data["__BUILD_THREADS__"] =
            opts.build_threads > 0 ? std::to_string(opts.build_threads) :
                                     "$(shell (nproc 2>/dev/null || sysctl -n hw.ncpu) 2>/dev/null)";
        data["__FILELIST__"]              = ofilelist;
        data["__LIB_DPI_FUNC_NAME_HASH__"] = std::string(lib_random_hash);
        data["__GENERATOR_PICKER_PATH__"] =
            appimage::is_running_as_appimage() ?
                (getenv("APPIMAGE") != nullptr ? std::string(getenv("APPIMAGE")) : std::string()) :
                get_executable_path();
        data["__GENERATOR_TEMPLATE_PATH__"] = std::filesystem::absolute(opts.source_dir);

        // Render lib files
        recursive_render(src_dir, dst_dir, data, env);

        // Copy verilog files
        for (const auto &entry : std::filesystem::directory_iterator(dst_dir)) {
            if (entry.is_regular_file()) {
                std::string src_filename, dst_filename;
                src_filename = entry.path().filename().string();
                if (src_filename.ends_with(".sv") || src_filename.ends_with(".v")) {
                    std::filesystem::rename(entry.path(),
                                            entry.path().parent_path()
                                                / (dst_module_name + "_" + entry.path().filename().string()));
                }
            }
        }
        for (auto &f : files) {
            auto ext = f.substr(f.find_last_of('.'));
            if (ext == ".sv") ext = ".v";
            std::filesystem::copy_file(f, dst_dir + "/" + dst_module_name + ext, // reserve the original extension
                                       std::filesystem::copy_options::overwrite_existing);
        }
        if (files.empty()) {
            // create an empty verilog file to avoid simulator error
            write_file(dst_dir + "/" + dst_module_name + ".v", "// empty file generated by picker\n");
        }

        PK_MESSAGE("Generate DPI files successfully!");
        return ret;
    }

}} // namespace picker::codegen
