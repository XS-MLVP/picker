#include <bits/stdc++.h>
#include "codegen/lib.hpp"

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

    void gen_filelist(const std::vector<std::string> &source_file, const std::vector<std::string> &ifilelists,
                      std::string &ofilelist)
    {
        std::vector<std::string> path_list;
        const std::vector<std::string> allow_file_types = {".sv", ".v", ".cpp", ".c", ".cc", ".cxx", ".so", ".a", ".o"};
        std::string fs_path                             = "";
        for (auto ifilelist : ifilelists) {
            if (check_file_type(ifilelist, {".txt", ".f"})) { // file
                std::ifstream ifs(ifilelist);
                std::string line;
                fs_path = std::filesystem::absolute(ifilelist).parent_path().string();
                while (std::getline(ifs, line)) { path_list.push_back(line); }
            } else { // split by comma
                std::string line;
                std::stringstream ss(ifilelist);
                while (std::getline(ss, line, ',')) { path_list.push_back(line); }
            }
        }
        for (auto &path : path_list) {
            path = picker::trim(path);
            if (path.starts_with("#")) { continue; }                      // skip comment line
            path = picker::trim(path.substr(0, path.find_first_of("#"))); // remove comment part
            if (path.empty()) { continue; }                               // skip empty line
            if (picker::contians(source_file, path)) { continue; }        // skip source file

            if (check_file_type(path, allow_file_types)) { // file
                auto target_file = path;
                if (!std::filesystem::exists(path) && !path.starts_with("/") && !fs_path.empty()) {
                    PK_ERROR("Cannot find file: %s, try search in path: %s", path.c_str(), fs_path.c_str());
                    path = (std::filesystem::path(fs_path) / path).string();
                }
                if (!std::filesystem::exists(path)) PK_FATAL("File not found: %s\n", target_file.c_str());
                path = std::filesystem::absolute(path).string();
                ofilelist += path + "\n";
            } else if (path.ends_with("/")) { // directory
                auto target_dir = path;
                if (!std::filesystem::exists(path) && !path.starts_with("/") && !fs_path.empty()) {
                    PK_ERROR("Cannot find directory: %s, try search in path: %s", path.c_str(), fs_path.c_str());
                    path = (std::filesystem::path(fs_path) / path).string();
                }
                if (!std::filesystem::exists(path)) PK_FATAL("Directory not found: %s\n", target_dir.c_str());
                std::filesystem::recursive_directory_iterator iter(path);
                for (const auto &entry : iter) {
                    if (entry.is_regular_file()) {
                        std::string filename = entry.path().filename().string();
                        if (check_file_type(filename, allow_file_types)) { ofilelist += entry.path().string() + "\n"; }
                    }
                }
            } else {
                PK_FATAL("Unsupported file type: %s\n", path.c_str());
            }
        }
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

    void gen_expins(nlohmann::json &expins, picker::export_opts &opts,
                    const std::vector<picker::sv_signal_define> &external_pins)
    {
        if (opts.rw_type != picker::SignalAccessType::MEM_DIRECT) {
            return;
        }
        for (const auto &pin : external_pins) {
            nlohmann::json expin;
            expin["name"] = pin.logic_pin;
            expin["type"] = (pin.logic_pin_type[0] == 'i') ? "In" : "Out";
            expin["hb"] = pin.logic_pin_hb;
            expin["lb"] = pin.logic_pin_lb;
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

        // firrtl base simulators
        std::unordered_set<std::string> firrtl_simulators = {"gsim"};
        if (firrtl_simulators.count(opts.sim)) {
            ret = gen_firrtl_param(data, sv_module_result, internal_pin, signal_tree, wave_file_name, simulator, opts.rw_type);
        // verilog based simulators
        } else {
            ret = gen_sv_param(data, sv_module_result, internal_pin, signal_tree, wave_file_name, simulator, opts.rw_type);
        }
        gen_cmake(src_dir, dst_dir, wave_file_name, simulator, vflag, cflag, env, data);

        // Set clock period
        printf("Frequency: %s\n", opts.frequency.c_str());
        get_clock_period(vcs_clock_period_h, vcs_clock_period_l, opts.frequency);

        // Render lib filelist
        gen_filelist(files, ifilelists, ofilelist);

        // Render expins info
        auto expins = nlohmann::json::array();
        gen_expins(expins, opts, ret);

        data["__MODULE_EXTERNAL_PINS__"]  = expins;
        data["__VCS_CLOCK_PERIOD_HIGH__"] = vcs_clock_period_h;
        data["__VCS_CLOCK_PERIOD_LOW__"]  = vcs_clock_period_l;
        data["__VERBOSE__"]               = opts.verbose ? "ON" : "OFF";
        data["__EXAMPLE__"]               = opts.example ? "ON" : "OFF";
        data["__COVERAGE__"]              = opts.coverage ? "ON" : "OFF";
        data["__CHECKPOINTS__"]           = opts.checkpoints ? "ON" : "OFF";
        data["__VPI__"]                   = opts.vpi ? "ON" : "OFF";
        data["__RW_TYPE__"]               = opts.rw_type == picker::SignalAccessType::MEM_DIRECT ? "MEM_DIRECT" : "DPI";
        data["__TARGET_LANGUAGE__"]       = opts.language;
        data["__FILELIST__"]              = ofilelist;
        data["__LIB_DPI_FUNC_NAME_HASH__"] = std::string(lib_random_hash);
        data["__GENERATOR_PICKER_PATH__"] =
            appimage::is_running_as_appimage() ?
                (getenv("APPIMAGE") != nullptr ? std::string(getenv("APPIMAGE")) : std::string()) :
                std::filesystem::read_symlink("/proc/self/exe").string();
        data["__GENERATOR_TEMPLATE_PATH__"] = std::filesystem::path(opts.source_dir).string();

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
            std::filesystem::copy_file(f, dst_dir + "/" + dst_module_name + ext, //reserve the original extension
                                       std::filesystem::copy_options::overwrite_existing);
        }
        PK_MESSAGE("Generate DPI files successfully!");
        return ret;
    }

}} // namespace picker::codegen
