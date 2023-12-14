#include <bits/stdc++.h>
#include "codegen/lib.hpp"

namespace mcv { namespace codegen {

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

    void lib(cxxopts::ParseResult opts, nlohmann::json &sync_opts,
             const std::vector<sv_signal_define> &external_pin,
             const std::vector<sv_signal_define> &internal_pin)
    {
        // Parse Options
        std::string filename = opts["file"].as<std::string>(),
                    src_dir  = opts["source_dir"].as<std::string>() + "/lib",
                    dst_dir  = opts["target_dir"].as<std::string>(),
                    src_module_name = sync_opts["src_module_name"],
                    dst_module_name = sync_opts["dst_module_name"],
                    wave_file_name  = opts["wave_file_name"].as<std::string>(),
                    simulator       = opts["sim"].as<std::string>(),
                    vflag           = opts["vflag"].as<std::string>(),
                    cflag = opts["cflag"].as<std::string>(), vcs_clock_period_h,
                    vcs_clock_period_l;

        // Build environment
        inja::Environment env;
        nlohmann::json data;

        data["__SOURCE_MODULE_NAME__"] = sync_opts["src_module_name"];
        data["__TOP_MODULE_NAME__"]    = sync_opts["dst_module_name"];

        gen_sv_param(data, external_pin, internal_pin, wave_file_name,
                     simulator);
        gen_cmake(src_dir, dst_dir, wave_file_name, simulator, vflag, cflag,
                  env, data);

        // Set clock period
        get_clock_period(vcs_clock_period_h, vcs_clock_period_l,
                         opts["frequency"].as<std::string>());

        data["__VCS_CLOCK_PERIOD_HIGH__"] = vcs_clock_period_h;
        data["__VCS_CLOCK_PERIOD_LOW__"]  = vcs_clock_period_l;
        data["__VERBOSE__"] = opts["verbose"].as<bool>() ? "ON" : "OFF";
        data["__EXAMPLE__"] = opts["example"].as<bool>() ? "ON" : "OFF";
        data["__TARGET_LANGUAGE__"] = opts["language"].as<std::string>();
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

}} // namespace mcv::codegen
