#include <bits/stdc++.h>
#include "mcv.hpp"
#include "codegen/cpp.hpp"
#include "codegen/sv.hpp"
#include "parser/sv.hpp"

namespace mcv { namespace codegen {

    void set_verilator(nlohmann::json &global_render_data,
                       std::string &cpp_flags,
                       const std::string &wave_file_name,
                       std::string &sv_dump_wave, std::string &verilator_trace)
    {
        inja::Environment env;

        cpp_flags += "-I /usr/local/share/verilator/include "
                     "-I /usr/local//share/verilator/include/vltstd/ "
                     "-DVL_DEBUG=1 "
                     "-DUSE_VERILATOR ";
        if (wave_file_name.length() > 0) {
            cpp_flags += "-DVL_TRACE ";
            if (wave_file_name.find(".vcd") != std::string::npos)
                verilator_trace = "--trace";
            else if (wave_file_name.find(".fst") != std::string::npos)
                verilator_trace = "--trace-fst";
            else
                FATAL("Verilator trace file must be .vcd or .fst format.\n");
            sv_dump_wave =
                env.render("initial begin\n"
                           "    $dumpfile(\"{{__WAVE_FILE_NAME__}}\");\n"
                           "    $dumpvars(0, {{__TOP_MODULE_NAME__}}_top);\n"
                           " end ",
                           global_render_data);
        }
    }

    void set_vcs(nlohmann::json &global_render_data, std::string &cpp_flags,
                 const std::string &wave_file_name, std::string &sv_dump_wave,
                 std::string &vcs_trace)
    {
        inja::Environment env;
        cpp_flags +=
            "-DUSE_VCS "
            "-I${VCS_HOME}/include -I${VCS_HOME}/linux64/lib/ "
            "-Wl,-rpath=${VCS_HOME}/linux64/lib -L${VCS_HOME}/linux64/lib "
            "";
        if (wave_file_name.length() > 0) {
            vcs_trace = "-debug_all ";
            if (wave_file_name.find(".fsdb") == std::string::npos)
                FATAL("VCS trace file must be .fsdb format.\n");
            sv_dump_wave = env.render(
                "initial begin\n"
                "    $fsdbDumpfile(\"{{__WAVE_FILE_NAME__}}\");\n"
                "    $fsdbDumpvars(0, {{__TOP_MODULE_NAME__}}_top);\n"
                " end ",
                global_render_data);
        }
    }

    void set_make_param(nlohmann::json &global_render_data,
                        const std::string &src_module_name,
                        std::string &dst_module_name,
                        const std::string &src_dir, const std::string &dst_dir,
                        const std::string &wave_file_name,
                        const std::string &simulator,
                        const std::string &simulator_flags)
    {
        inja::Environment env;
        // dst_module_name为生成模板的module name，未指定时默认为src_module_name
        if (dst_module_name.length() == 0) dst_module_name = src_module_name;

        global_render_data["__WAVE_FILE_NAME__"]     = wave_file_name;
        global_render_data["__SIMULATOR__"]          = simulator;
        global_render_data["__SIMULATOR_FLAGS__"]    = simulator_flags;
        global_render_data["__SOURCE_MODULE_NAME__"] = src_module_name;
        global_render_data["__TOP_MODULE_NAME__"]    = dst_module_name;

        // Render Simulator Related Files
        std::string cpp_flags, sv_dump_wave, verilator_trace, vcs_trace;

        if (simulator == "verilator") {
            set_verilator(global_render_data, cpp_flags, wave_file_name,
                          sv_dump_wave, verilator_trace);
        } else if (simulator == "vcs") {
            set_vcs(global_render_data, cpp_flags, wave_file_name, sv_dump_wave,
                    vcs_trace);
        } else {
            FATAL("Unsupported simulator: %s\n", simulator.c_str());
        }

        global_render_data["__CPP_FLAGS__"]       = cpp_flags;
        global_render_data["__SV_DUMP_WAVE__"]    = sv_dump_wave;
        global_render_data["__VERILATOR_TRACE__"] = verilator_trace;
        global_render_data["__VCS_TRACE__"]       = vcs_trace;
    };

    /// @brief render all files in src template dir to dst dir
    /// @param opts
    /// @param src_module_name
    /// @param external_pin
    /// @param internal_signal
    void render(const cxxopts::ParseResult &opts,
                const std::string &src_module_name,
                const std::vector<sv_signal_define> &external_pin,
                const std::vector<sv_signal_define> &internal_signal)
    {
        inja::Environment env;
        std::string filename = opts["file"].as<std::string>();
        std::string src_dir  = opts["source_dir"].as<std::string>();
        std::string dst_dir  = opts["target_dir"].as<std::string>();
        std::string dst_module_name =
            opts["target_module_name"].as<std::string>();
        std::string wave_file_name = opts["wave_file_name"].as<std::string>();
        std::string simulator      = opts["sim"].as<std::string>();
        std::string vflag          = opts["vflag"].as<std::string>();
        std::string frequency      = opts["frequency"].as<std::string>();

        nlohmann::json global_render_data;

        set_sv_param(global_render_data, external_pin, internal_signal);
        set_cpp_param(global_render_data, external_pin, internal_signal,
                      frequency);
        set_make_param(global_render_data, src_module_name, dst_module_name,
                       src_dir, dst_dir, wave_file_name, simulator, vflag);

        if (!std::filesystem::create_directory(dst_dir)) {
            FATAL("Failed to create directory: %s due to already exists ",
                  dst_dir.c_str());
        }
        for (const auto &entry : std::filesystem::directory_iterator(src_dir)) {
            if (entry.is_regular_file()) {
                std::string src_filename, dst_filename, dst_file_content;
                src_filename = entry.path().filename().string();
                // 如果是sv或者v文件，则自动补全dst_filename
                if (src_filename.compare("top.sv") == 0
                    || src_filename.compare("top.v") == 0)
                    dst_filename = dst_module_name + "_" + src_filename;
                else
                    dst_filename = src_filename;
                dst_filename = dst_dir + "/" + dst_filename;
                MESSAGE("Render file: %s to %s\n", src_filename.c_str(),
                        dst_filename.c_str());
                dst_file_content =
                    env.render_file(entry.path().string(), global_render_data);
                write_file(dst_filename, dst_file_content);
            }
        }

        MESSAGE("Generate DPI files successfully!");

        std::filesystem::copy_file(
            filename, dst_dir + "/" + dst_module_name + ".v",
            std::filesystem::copy_options::overwrite_existing);
    }
}} // namespace mcv::codegen
