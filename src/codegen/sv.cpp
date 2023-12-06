#include <bits/stdc++.h>
#include "codegen/sv.hpp"

namespace mcv { namespace codegen {

    namespace sv {
        static const std::string pin_connect_template =
            "    .{{logic_pin}}({{logic_pin}}),\n";
        static const std::string logic_pin_declaration_template =
            "  logic {{logic_pin_length}} {{logic_pin}};\n";
        static const std::string wire_pin_declaration_template =
            "  wire {{logic_pin_length}} {{logic_pin}};\n";

        static const std::string dpi_get_export_template =
            "  export \"DPI-C\" function get_{{pin_func_name}};\n";
        static const std::string dpi_set_export_template =
            "  export \"DPI-C\" function set_{{pin_func_name}};\n";

        static const std::string dpi_get_impl_template =
            "  function void get_{{pin_func_name}};\n"
            "    output logic {{logic_pin_length}} value;\n"
            "    value={{logic_pin}};\n"
            "  endfunction\n\n";

        static const std::string dpi_set_impl_template =
            "  function void set_{{pin_func_name}};\n"
            "    input logic {{logic_pin_length}} value;\n"
            "    {{logic_pin}}=value;\n"
            "  endfunction\n\n";

        /// @brief Export external pin for verilog render, contains pin connect,
        /// @param pin
        /// @param pin_connect
        /// @param logic
        /// @param wire
        /// @param dpi_export
        /// @param dpi_impl
        void render_external_pin(std::vector<sv_signal_define> pin,
                                 std::string &pin_connect, std::string &logic,
                                 std::string &wire, std::string &dpi_export,
                                 std::string &dpi_impl)
        {
            inja::Environment env;
            nlohmann::json data;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"] = replace_all(pin[i].logic_pin, ".", "_");

                // Set empty or [hb:lb] for verilog render
                data["logic_pin_length"] =
                    pin[i].logic_pin_hb == -1 ?
                        "" :
                        "[" + std::to_string(pin[i].logic_pin_hb) + ":"
                            + std::to_string(pin[i].logic_pin_lb) + "]";

                pin_connect =
                    pin_connect + env.render(pin_connect_template, data);
                logic =
                    logic + env.render(logic_pin_declaration_template, data);
                wire = wire + env.render(wire_pin_declaration_template, data);

                dpi_export = dpi_export
                             + env.render(dpi_get_export_template, data)
                             + env.render(dpi_set_export_template, data);
                dpi_impl = dpi_impl + env.render(dpi_get_impl_template, data)
                           + env.render(dpi_set_impl_template, data);
            }
            if (pin_connect.length() == 0)
                FATAL(
                    "No port information of src_module was found in the specified file. \nPlease check whether the file name or source module name is correct.");
            pin_connect.pop_back();
            pin_connect.pop_back();
        }

        /// @brief Export internal signal for verilog render, only contains dpi
        /// get function
        /// @param pin
        /// @param dpi_export
        /// @param dpi_impl
        void render_internal_signal(std::vector<sv_signal_define> pin,
                                    std::string &dpi_export,
                                    std::string &dpi_impl)
        {
            inja::Environment env;
            nlohmann::json data;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"] = replace_all(pin[i].logic_pin, ".", "_");

                // Set empty or [hb:lb] for verilog render
                data["logic_pin_length"] =
                    pin[i].logic_pin_hb == -1 ?
                        "" :
                        "[" + std::to_string(pin[i].logic_pin_hb) + ":"
                            + std::to_string(pin[i].logic_pin_lb) + "]";

                dpi_export =
                    dpi_export + env.render(dpi_get_export_template, data);
                dpi_impl = dpi_impl + env.render(dpi_get_impl_template, data);
            }
        };

        void render_sv_waveform(const std::string &simulator,
                                const std::string &wave_file_name,
                                nlohmann::json &data)
        {
            inja::Environment env;
            std::string sv_dump_wave, trace;
            data["__WAVE_FILE_NAME__"] = wave_file_name;
            if (simulator == "verilator") {
                if (wave_file_name.length() > 0) {
                    if (wave_file_name.ends_with(".vcd")
                        || wave_file_name.ends_with(".fst"))
                        sv_dump_wave = env.render(
                            "initial begin\n"
                            "    $dumpfile(\"{{__WAVE_FILE_NAME__}}\");\n"
                            "    $dumpvars(0, {{__TOP_MODULE_NAME__}}_top);\n"
                            " end ",
                            data);
                    else
                        FATAL(
                            "Verilator trace file must be .vcd or .fst format.\n");
                }
            } else if (simulator == "vcs") {
                if (wave_file_name.length() > 0) {
                    if (wave_file_name.ends_with(".fsdb") == false)
                        FATAL("VCS trace file must be .fsdb format.\n");
                    sv_dump_wave = env.render(
                        "initial begin\n"
                        "    $fsdbDumpfile(\"{{__WAVE_FILE_NAME__}}\");\n"
                        "    $fsdbDumpvars(0, {{__TOP_MODULE_NAME__}}_top);\n"
                        " end ",
                        data);
                }
            } else {
                FATAL("Unsupported simulator: %s\n", simulator.c_str());
            }

            data["__TRACE__"] =
                sv_dump_wave.length() > 0 ?
                    wave_file_name.substr(wave_file_name.find_last_of(".") + 1,
                                          wave_file_name.length()) :
                    "OFF";
            data["__SV_DUMP_WAVE__"] = sv_dump_wave;
        }
    } // namespace sv
    /// @brief generate system verilog wrapper file contains
    /// __PIN_CONNECT__ , __LOGIC_PIN_DECLARATION__ , __WIRE_PIN_DECLARATION__
    /// __DPI_FUNCTION_EXPORT__ and __DPI_FUNCTION_IMPLEMENT__
    /// @param global_render_data
    /// @param external_pin
    /// @param internal_signal
    void gen_sv_param(nlohmann::json &global_render_data,
                      const std::vector<sv_signal_define> &external_pin,
                      const std::vector<sv_signal_define> &internal_signal,
                      const std::string &wave_file_name,
                      const std::string &simulator)
    {
        std::string pin_connect, logic, wire, dpi_export, dpi_impl;

        sv::render_external_pin(external_pin, pin_connect, logic, wire,
                                dpi_export, dpi_impl);
        sv::render_internal_signal(internal_signal, dpi_export, dpi_impl);
        sv::render_sv_waveform(simulator, wave_file_name, global_render_data);

        global_render_data["__LOGIC_PIN_DECLARATION__"]  = logic;
        global_render_data["__WIRE_PIN_DECLARATION__"]   = wire;
        global_render_data["__PIN_CONNECT__"]            = pin_connect;
        global_render_data["__DPI_FUNCTION_EXPORT__"]    = dpi_export;
        global_render_data["__DPI_FUNCTION_IMPLEMENT__"] = dpi_impl;
    }

}} // namespace mcv::codegen