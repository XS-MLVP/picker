#include "codegen/sv/waveform.hpp"
#include "picker.hpp"

namespace picker::codegen::sv {
namespace {

const std::string vcs_fsdb_control_sv_template =
    "  export \"DPI-C\" function vcs_fsdb_set_waveform_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "  function void vcs_fsdb_set_waveform_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    input string filename;\n"
    "    picker_fsdb_configured = 1;\n"
    "    $fsdbDumpFinish;\n"
    "    $fsdbDumpfile(filename);\n"
    "    $fsdbDumpvars(0, {{__TOP_MODULE_NAME__}}_top{{__DUMP_VAR_OPTIONS__}});\n"
    "  endfunction\n\n"
    "  export \"DPI-C\" function vcs_fsdb_finish_waveform_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "  function void vcs_fsdb_finish_waveform_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    $fsdbDumpFinish;\n"
    "  endfunction\n\n"
    "  export \"DPI-C\" function vcs_fsdb_flush_waveform_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "  function void vcs_fsdb_flush_waveform_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    $fsdbDumpflush;\n"
    "  endfunction\n\n"
    "  export \"DPI-C\" function vcs_fsdb_waveform_enable_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "  function void vcs_fsdb_waveform_enable_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    input byte unsigned enable;\n"
    "    if (enable) $fsdbDumpon;\n"
    "    else $fsdbDumpoff;\n"
    "  endfunction\n";

const std::string uvs_usdb_control_sv_template =
    "  export \"DPI-C\" function uvs_usdb_set_waveform_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "  function void uvs_usdb_set_waveform_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    input string filename;\n"
    "    picker_usdb_configured = 1;\n"
    "    $usdbDumpFinish;\n"
    "    $usdbDumpfile(filename);\n"
    "    $usdbDumpvars(0, {{__TOP_MODULE_NAME__}}_top{{__DUMP_VAR_OPTIONS__}});\n"
    "  endfunction\n\n"
    "  export \"DPI-C\" function uvs_usdb_finish_waveform_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "  function void uvs_usdb_finish_waveform_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    $usdbDumpFinish;\n"
    "  endfunction\n\n"
    "  export \"DPI-C\" function uvs_usdb_flush_waveform_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "  function void uvs_usdb_flush_waveform_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    $usdbDumpflush;\n"
    "  endfunction\n\n"
    "  export \"DPI-C\" function uvs_usdb_waveform_enable_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "  function void uvs_usdb_waveform_enable_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    input byte unsigned enable;\n"
    "    if (enable) $usdbDumpon;\n"
    "    else $usdbDumpoff;\n"
    "  endfunction\n";

const std::string vcs_coverage_sv_template =
    "  export \"DPI-C\" task vcs_coverage_start_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "  task vcs_coverage_start_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    $cg_coverage_control(1);\n"
    "  endtask\n\n"
    "  export \"DPI-C\" task vcs_coverage_stop_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "  task vcs_coverage_stop_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    $cg_coverage_control(0);\n"
    "  endtask\n\n"
    "  export \"DPI-C\" task vcs_coverage_reset_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "  task vcs_coverage_reset_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    $coverage_reset();\n"
    "  endtask\n\n"
    "  export \"DPI-C\" task vcs_coverage_dump_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "  task vcs_coverage_dump_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    input string name;\n"
    "    $coverage_dump(name);\n"
    "  endtask\n";

} // namespace

void render_sv_waveform(const std::string &simulator, const std::string &wave_file_name,
                        nlohmann::json &data)
{
    inja::Environment env;
    std::string sv_dump_wave, sv_waveform_control;
    auto dump_var_options = picker::get_env("DUMPVARS_OPTION", "");
    if (!dump_var_options.empty()) {
        PK_DEBUG("Find DUMPVARS_OPTION=%s", dump_var_options.c_str());
        dump_var_options = ", \"" + dump_var_options + "\"";
    }
    data["__WAVE_FILE_NAME__"] = wave_file_name;
    data["__DUMP_VAR_OPTIONS__"] = dump_var_options;
    data["__LIB_DPI_FUNC_NAME_HASH__"] = std::string(lib_random_hash);
    if (simulator == "verilator") {
        if (!wave_file_name.empty()) {
            if (wave_file_name.ends_with(".vcd") || wave_file_name.ends_with(".fst")) {
                sv_dump_wave = env.render(
                    "  initial begin\n"
                    "    $dumpfile(\"{{__WAVE_FILE_NAME__}}\");\n"
                    "    $dumpvars(0, {{__TOP_MODULE_NAME__}}_top{{__DUMP_VAR_OPTIONS__}});\n"
                    "  end", data);
            } else {
                PK_FATAL("Verilator trace file must be .vcd or .fst format.\n");
            }
        }
    } else if (simulator == "vcs") {
        sv_waveform_control = env.render(vcs_fsdb_control_sv_template, data)
                            + env.render(vcs_coverage_sv_template, data);
        if (!wave_file_name.empty()) {
            if (!wave_file_name.ends_with(".fsdb")) PK_FATAL("VCS trace file must be .fsdb format.\n");
            sv_dump_wave = env.render(
                "  initial begin\n"
                "    if (!picker_fsdb_configured) begin\n"
                "    $fsdbDumpfile(\"{{__WAVE_FILE_NAME__}}\");\n"
                "    $fsdbDumpvars(0, {{__TOP_MODULE_NAME__}}_top{{__DUMP_VAR_OPTIONS__}});\n"
                "    end\n"
                "  end\n\n", data);
        }
    } else if (simulator == "uvs") {
        sv_waveform_control = env.render(uvs_usdb_control_sv_template, data);
        if (!wave_file_name.empty()) {
            if (!wave_file_name.ends_with(".usdb")) PK_FATAL("USDB trace file must be .usdb format.\n");
            sv_dump_wave = env.render(
                "  initial begin\n"
                "    if (!picker_usdb_configured) begin\n"
                "    $usdbDumpfile(\"{{__WAVE_FILE_NAME__}}\");\n"
                "    $usdbDumpvars(0, {{__TOP_MODULE_NAME__}}_top{{__DUMP_VAR_OPTIONS__}});\n"
                "    end\n"
                "  end\n\n", data);
        }
    } else {
        PK_FATAL("Unsupported simulator: %s\n", simulator.c_str());
    }

    data["__TRACE__"] = !sv_dump_wave.empty() ? wave_file_name.substr(wave_file_name.find_last_of(".") + 1,
                                                                  wave_file_name.length()) : "OFF";
    // Declare the runtime guard before its initial-block use without changing
    // the waveform/control order or trace enablement above.
    if (simulator == "vcs") sv_dump_wave = "  bit picker_fsdb_configured = 0;\n" + sv_dump_wave;
    if (simulator == "uvs") sv_dump_wave = "  bit picker_usdb_configured = 0;\n" + sv_dump_wave;
    data["__SV_DUMP_WAVE__"] = sv_dump_wave + sv_waveform_control;
}

} // namespace picker::codegen::sv
