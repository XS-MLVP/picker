#include <bits/stdc++.h>
#include "mcv.hpp"
#include "codegen/cpp.hpp"
#include "parser/sv.hpp"

namespace mcv
{
    namespace codegen
    {
        int gen_dpi(std::vector<sv_pin_member> pin, std::string &src_module_name, std::string &dst_module_name, std::string &src_dir, std::string &dst_dir, std::string &wave_file_name, std::string &simulator, std::string &vflag)
        {
            inja::Environment env;

            // Codegen Segment Buffers
            std::string logic_pin_declaration, wire_pin_declaration, pin_connect, dpi_function_export, dpi_function_implement, hpp_logic_annotation, hpp_input_pins, hpp_output_pins, cpp_reinit_pins, cpp_bind_pins;

            // DPI Segment Templates
            std::string pin_connect_template = "    .{{logic_pin}}({{logic_pin}}),\n";
            std::string logic_pin_declaration_template = "  logic {{logic_pin_length}} {{logic_pin}};\n";
            std::string wire_pin_declaration_template = "  wire {{logic_pin_length}} {{logic_pin}};\n";

            std::string dpi_function_export_template = "  export \"DPI-C\" function get_{{logic_pin}};\n"
                                                       "  export \"DPI-C\" function set_{{logic_pin}};\n";

            std::string dpi_function_implement_template = "  function get_{{logic_pin}};\n"
                                                          "    output logic {{logic_pin_length}} value;\n"
                                                          "    value={{logic_pin}};\n"
                                                          "  endfunction\n\n"
                                                          "  function set_{{logic_pin}};\n"
                                                          "    input logic {{logic_pin_length}} value;\n"
                                                          "    {{logic_pin}}=value;\n"
                                                          "  endfunction\n\n";

            std::string hpp_logic_annotation_template = "    // {{logic_pin_type}} {{logic_pin_length}} {{logic_pin}}\n";
            std::string hpp_logic_pin_template = "    XData {{logic_pin}};\n";
            std::string cpp_reinit_pins_template = "    this->{{logic_pin}}.ReInit({{logic_pin_length}}, IOType::{{logic_pin_type}}, \"{{logic_pin}}\");\n";
            std::string cpp_bind_pins_template = "    this->{{logic_pin}}.BindDPIRW(get_{{logic_pin}}, set_{{logic_pin}});\n";

            // Fill Codegen Segment Buffers
            nlohmann::json data;
            for (int i = 0; i < pin.size(); i++)
            {
                data["logic_pin"] = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["logic_pin_length"] = pin[i].logic_pin_length;
                pin_connect = pin_connect + env.render(pin_connect_template, data);
                cpp_reinit_pins = cpp_reinit_pins + env.render(cpp_reinit_pins_template, data);
                cpp_bind_pins = cpp_bind_pins + env.render(cpp_bind_pins_template, data);

                if (pin[i].logic_pin_type == "Input")
                    hpp_input_pins = hpp_input_pins + env.render(hpp_logic_pin_template, data);
                else if (pin[i].logic_pin_type == "Output")
                    hpp_output_pins = hpp_output_pins + env.render(hpp_logic_pin_template, data);

                if (pin[i].logic_pin_length == 0)
                    data["logic_pin_length"] = "";
                else
                    data["logic_pin_length"] = "[" + std::to_string(pin[i].logic_pin_length - 1) + ":" + "0" + "]";
                hpp_logic_annotation = hpp_logic_annotation + env.render(hpp_logic_annotation_template, data);
                logic_pin_declaration = logic_pin_declaration + env.render(logic_pin_declaration_template, data);
                wire_pin_declaration = wire_pin_declaration + env.render(wire_pin_declaration_template, data);
                dpi_function_export = dpi_function_export + env.render(dpi_function_export_template, data);
                dpi_function_implement = dpi_function_implement + env.render(dpi_function_implement_template, data);
            }
            // remove extra ','
            if (pin_connect.length() == 0)
                FATAL("No port information of src_module was found in the specified file. \nPlease check whether the file name or source module name is correct.");
            pin_connect.pop_back();
            pin_connect.pop_back();

            // Render DPI Files
            // dst_module_name为生成模板的module name，未指定时默认为src_module_name
            if (dst_module_name.length() == 0)
                dst_module_name = src_module_name;

            data["__SOURCE_MOUDLE_NAME__"] = src_module_name;
            data["__TOP_MODULE_NAME__"] = dst_module_name;
            data["__LOGIC_PIN_DECLARATION__"] = logic_pin_declaration;
            data["__WIRE_PIN_DECLARATION__"] = wire_pin_declaration;
            data["__PIN_CONNECT__"] = pin_connect;
            data["__DPI_FUNCTION_EXPORT__"] = dpi_function_export;
            data["__DPI_FUNCTION_IMPLEMENT__"] = dpi_function_implement;

            data["__INPUT_PINS__"] = hpp_input_pins;
            data["__OUTPUT_PINS__"] = hpp_output_pins;
            data["__REINIT_PINS__"] = cpp_reinit_pins;
            data["__BIND_PINS__"] = cpp_bind_pins;
            data["__LOGIC_ANNOTATION__"] = hpp_logic_annotation;

            data["__WAVE_FILE_NAME__"] = wave_file_name;
            data["__SIMULATOR__"] = simulator;

            // Render Simulator Related Files
            std::string cpp_flags;
            std::string sv_dump_wave;
            std::string verilaotr_trace;
            std::string vcs_trace;
            if (simulator == "verilator")
            {
                cpp_flags += "-I /usr/local/share/verilator/include "
                             "-I /usr/local//share/verilator/include/vltstd/ "
                             "-DVL_DEBUG=1 "
                             "-DUSE_VERILATOR ";
                if (wave_file_name.length() > 0)
                {
                    verilaotr_trace = "--trace-fst";
                    cpp_flags += "-DVL_TRACE ";
                    if (wave_file_name.find(".fst") == std::string::npos)
                        FATAL("Verilator trace file must be .fst format.\n");
                    sv_dump_wave = env.render("initial begin\n"
                                              "    $dumpfile(\"{{__WAVE_FILE_NAME__}}\");\n"
                                              "    $dumpvars(0, {{__TOP_MODULE_NAME__}}_top);\n"
                                              " end ",
                                              data);
                }
            }
            else if (simulator == "vcs")
            {
                cpp_flags += "-DUSE_VCS "
                             "-I${VCS_HOME}/include -I${VCS_HOME}/linux64/lib/ "
                             "-Wl,-rpath=${VCS_HOME}/linux64/lib -L${VCS_HOME}/linux64/lib "
                             "";
                if (wave_file_name.length() > 0)
                {
                    vcs_trace = "-debug_all ";
                    if (wave_file_name.find(".fsdb") == std::string::npos)
                        FATAL("VCS trace file must be .fsdb format.\n");
                    sv_dump_wave = env.render("initial begin\n"
                                              "    $fsdbDumpfile(\"{{__WAVE_FILE_NAME__}}\");\n"
                                              "    $fsdbDumpvars(0, {{__TOP_MODULE_NAME__}}_top);\n"
                                              " end ",
                                              data);
                }
            }
            else
            {
                FATAL("Unsupported simulator: %s\n", simulator.c_str());
                return -1;
            }
            data["__CPP_FLAGS__"] = cpp_flags;
            data["__SV_DUMP_WAVE__"] = sv_dump_wave;
            data["__VERILAOTR_TRACE__"] = verilaotr_trace;
            data["__VCS_TRACE__"] = vcs_trace;

            // Render all files in src_dir to dst_dir
            if (!std::filesystem::create_directory(dst_dir))
            {
                FATAL("Failed to create directory: %s due to existing dir with the same name.\n", dst_dir.c_str());
                return -1;
            }
            for (const auto &entry : std::filesystem::directory_iterator(src_dir))
            {
                if (entry.is_regular_file())
                {
                    std::string src_filename, dst_filename, dst_file_content;
                    src_filename = entry.path().filename().string();
                    // 如果是sv或者v文件，则自动补全dst_filename
                    if (src_filename.compare("top.sv") == 0 || src_filename.compare("top.v") == 0)
                        dst_filename = dst_module_name + "_" + src_filename;
                    else
                        dst_filename = src_filename;
                    dst_filename = dst_dir + "/" + dst_filename;
                    MESSAGE("Render file: %s to %s\n", src_filename.c_str(), dst_filename.c_str());
                    dst_file_content = env.render_file(entry.path().string(), data);
                    write_file(dst_filename, dst_file_content);
                }
            }

            MESSAGE("Generate DPI files successfully!");
            return 0;
        }

        void cpp(cxxopts::ParseResult opts, std::vector<sv_pin_member> pin)
        {
            std::string filename = opts["file"].as<std::string>();
            std::string src_dir = opts["source_dir"].as<std::string>();
            std::string dst_dir = opts["target_dir"].as<std::string>();
            std::string src_module_name = opts["source_module_name"].as<std::string>();
            std::string dst_module_name = opts["target_module_name"].as<std::string>();
            std::string wave_file_name = opts["wave_file_name"].as<std::string>();
            std::string simulator = opts["sim"].as<std::string>();
            std::string vflag = opts["vflag"].as<std::string>();

            // gen
            gen_dpi(pin, src_module_name, dst_module_name, src_dir, dst_dir, wave_file_name, simulator, vflag);
            // copy verilog source
            std::filesystem::copy_file(filename, dst_dir + "/" + dst_module_name + ".v", std::filesystem::copy_options::overwrite_existing);
        }
    }
}
