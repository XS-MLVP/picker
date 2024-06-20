#include <bits/stdc++.h>
#include "codegen/lib.hpp"

namespace picker { namespace codegen {

    void recursive_render(std::string &src_dir, std::string &dst_dir,
                          nlohmann::json &data, inja::Environment &env)
    {
        // Create dst_dir if not exist
        std::filesystem::create_directory(dst_dir);
        // Render all files in src_dir to dst_dir
        for (const auto &entry : std::filesystem::directory_iterator(src_dir)) {
            if (entry.is_regular_file()) {
                std::string src_filename, dst_filename, dst_file_content;
                src_filename = entry.path().filename().string();
                dst_filename = dst_dir + "/" + src_filename;
                MESSAGE("Render file: %s to %s", src_filename.c_str(),
                        dst_filename.c_str());
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

    void gen_filelist(const std::string &source_file, const std::string &ifilelist, std::string &ofilelist)
    {
        std::vector<std::string> path_list;
        if (ifilelist.ends_with(".txt")) { // read from file
            std::ifstream ifs(ifilelist);
            std::string line;
            while (std::getline(ifs, line)) { path_list.push_back(line); }
        } else { // split by comma
            std::string line;
            std::stringstream ss(ifilelist);
            while (std::getline(ss, line, ',')) { path_list.push_back(line); }
        }
        for (auto &path : path_list) {
            if (path.starts_with("#")) { continue; } // skip comment line
            path.substr(0, path.find_first_of("#")); // remove comment part
            if (path == source_file)  { continue; } // skip source file

            if (path.ends_with(".sv") || path.ends_with(".v")) { // single file
                path = std::filesystem::absolute(path).string();
                ofilelist += path + "\n";
            } else if (path.ends_with("/")) { // directory
                std::filesystem::recursive_directory_iterator iter(path);
                for (const auto &entry : iter) {
                    if (entry.is_regular_file()) {
                        std::string filename = entry.path().filename().string();
                        if (filename.ends_with(".sv")
                            || filename.ends_with(".v")) {
                            ofilelist += entry.path().string() + "\n";
                        }
                    }
                }
            } else {
                FATAL("Unsupported file type: %s\n", path.c_str());
            }
        }
    }

    void get_clock_period(std::string &vcs_clock_period_h,
                          std::string &vcs_clock_period_l,
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
            FATAL("Unsupported frequency unit: %s\n", frequency.c_str());
        } // end if
        vcs_clock_period_h = std::to_string((period >> 1) + (period & 1));
        vcs_clock_period_l = std::to_string(period >> 1);
    }

    void gen_cmake(std::string &src_dir, std::string &dst_dir,
                   std::string &wave_file_name, std::string &simulator,
                   std::string &vflag, std::string &cflag,
                   inja::Environment &env, nlohmann::json &data)
    {
        data["__SIMULATOR__"] = simulator;
        data["__VFLAG__"]     = vflag;
        data["__CFLAG__"]     = cflag;

        std::string verilaotr_coverage, vcs_coverage;
    }

    void lib(picker::exports_opts &opts,
             const std::vector<picker::sv_signal_define> &external_pin,
             const std::vector<picker::sv_signal_define> &internal_pin)
    {
        // Parse Options
        std::string filename = opts.file,
                    src_dir  = opts.source_dir + "/lib",
                    dst_dir  = opts.target_dir,
                    src_module_name = opts.source_module_name,
                    dst_module_name = opts.target_module_name,
                    wave_file_name  = opts.wave_file_name,
                    simulator       = opts.sim,
                    vflag           = opts.vflag,
                    cflag           = opts.cflag,
                    ifilelist = opts.filelist, ofilelist,
                    vcs_clock_period_h, vcs_clock_period_l;

        // Build environment
        inja::Environment env;
        nlohmann::json data;

        data["__SOURCE_MODULE_NAME__"] = src_module_name;
        data["__TOP_MODULE_NAME__"]    = dst_module_name;

        gen_sv_param(data, external_pin, internal_pin, wave_file_name,
                     simulator);
        gen_cmake(src_dir, dst_dir, wave_file_name, simulator, vflag, cflag,
                  env, data);

        // Set clock period
        printf("Frequency: %s\n", opts.frequency.c_str());
        get_clock_period(vcs_clock_period_h, vcs_clock_period_l,
                            opts.frequency);    

        // Render lib filelist
        gen_filelist(filename, ifilelist, ofilelist);

        data["__VCS_CLOCK_PERIOD_HIGH__"] = vcs_clock_period_h;
        data["__VCS_CLOCK_PERIOD_LOW__"]  = vcs_clock_period_l;
        data["__VERBOSE__"]         = opts.verbose ? "ON" : "OFF";
        data["__EXAMPLE__"]         = opts.example ? "ON" : "OFF";
        data["__COVERAGE__"]        = opts.coverage ? "ON" : "OFF";
        data["__TARGET_LANGUAGE__"] = opts.language;
        data["__FILELIST__"]        = ofilelist;

        // Render lib files
        recursive_render(src_dir, dst_dir, data, env);

        // Copy verilog files
        for (const auto &entry : std::filesystem::directory_iterator(dst_dir)) {
            if (entry.is_regular_file()) {
                std::string src_filename, dst_filename;
                src_filename = entry.path().filename().string();
                if (src_filename.ends_with(".sv")
                    || src_filename.ends_with(".v")) {
                    std::filesystem::rename(
                        entry.path(),
                        entry.path().parent_path()
                            / (dst_module_name + "_"
                               + entry.path().filename().string()));
                }
            }
        }
        std::filesystem::copy_file(
            filename, dst_dir + "/" + dst_module_name + ".v",
            std::filesystem::copy_options::overwrite_existing);

        MESSAGE("Generate DPI files successfully!");
    }

}} // namespace picker::codegen
