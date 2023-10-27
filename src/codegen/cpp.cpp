#include <bits/stdc++.h>
#include "mcv.hpp"
#include "cxxopts.hpp"
#include "codegen/json.hpp"
#include "codegen/inja.hpp"

namespace mcv
{
    namespace codegen
    {

        struct sv_pin_member
        {
            std::string logic_pin;
            std::string logic_pin_type;
            int logic_pin_length;
        };

        std::string capitalize_first_letter(const std::string &str)
        {
            if (str.empty())
                return str;
            std::string result = str;
            if (result[0] >= 'a' && result[0] <= 'z')
                result[0] = result[0] - 'a' + 'A';
            return result;
        }

        std::vector<sv_pin_member> parse_sv_pin(std::string filename, std::string &src_module_name)
        {
            // call verible-verilog-syntax
            std::string syntax_cmd = "verible-verilog-syntax --export_json --printtokens " + filename;
            std::string verible_result = exec(syntax_cmd.c_str());

            // nlohmann parse json
            nlohmann::json module_json = nlohmann::json::parse(verible_result);

            // filter json like 标点符号等
            auto module_token = module_json[filename]["tokens"];
            for (auto it = module_token.begin(); it != module_token.end();)
            {
                auto itemV = it.value();
                std::string a = itemV["tag"];
                (a.size() < 5) ? (it = module_token.erase(it)) : (it++);
            }
            // 不指明解析的module名，则默认解析文件中最后一个module
            if (src_module_name.length() == 0)
            {
                std::vector<std::string> module_list;
                for (int i = 0; i < module_token.size(); i++)
                    if (module_token[i]["tag"] == "module")
                        module_list.push_back(module_token[i + 1]["text"]);
                src_module_name = module_list.back();
            }

            // 解析module_token，将解析好的pin_name\pin_length\pin_type都push进pin数组并返回。
            std::vector<sv_pin_member> pin;
            for (int j = 0; j < module_token.size(); j++)
                if (module_token[j]["tag"] == "module" && module_token[j + 1]["text"] == src_module_name)
                {
                    for (int i = j; i < module_token.size(); i++)
                    {
                        if (module_token[i]["tag"] == "input" || module_token[i]["tag"] == "output")
                        {
                            sv_pin_member tmp_pin;
                            tmp_pin.logic_pin_type = capitalize_first_letter(module_token[i]["tag"]);
                            if (module_token[i + 1]["tag"] == "SymbolIdentifier")
                            {
                                tmp_pin.logic_pin = module_token[i + 1]["text"];
                                tmp_pin.logic_pin_length = 0;
                            }
                            else if (module_token[i + 1]["tag"] == "TK_DecNumber")
                            {
                                std::string pin_high = module_token[i + 1]["text"];
                                std::string pin_low = module_token[i + 2]["text"];
                                tmp_pin.logic_pin = module_token[i + 3]["text"];
                                tmp_pin.logic_pin_length = std::stoi(pin_high) - std::stoi(pin_low) + 1;
                            }
                            pin.push_back(tmp_pin);
                        }
                    }
                }
            return pin;
        }

        int gen_dpi(std::vector<sv_pin_member> pin, std::string &src_module_name, std::string &dst_module_name, std::string &src_dir, std::string &dst_dir)
        {
            inja::Environment env;

            // Codegen Segment Buffers
            std::string logic_pin_declaration, wire_pin_declaration, pin_connect, dpi_function_export, dpi_function_implement, hpp_logic_annotation, hpp_input_pins, hpp_output_pins, cpp_reinit_pins, cpp_bind_pins;

            // DPI Segment Templates
            std::string pin_connect_template = "    .{{logic_pin}}({{logic_pin}}),\n";
            std::string logic_pin_declaration_template = "  logic {{logic_pin_length}} {{logic_pin}};\n";
            std::string wire_pin_declaration_template = "  wire {{logic_pin_length}} {{logic_pin}};\n";
            std::string dpi_function_export_template = "  export \"DPI-C\" function get_{{logic_pin}};\n  export \"DPI-C\" function set_{{logic_pin}};\n";
            std::string dpi_function_implement_template = "  function get_{{logic_pin}};\n    output logic {{logic_pin_length}} value;\n    value={{logic_pin}};\n  endfunction\n\n  function set_{{logic_pin}};\n    input logic {{logic_pin_length}} value;\n    {{logic_pin}}=value;\n  endfunction\n\n";

            std::string hpp_logic_annotation_template = "    // {{logic_pin_type}} {{logic_pin_length}} {{logic_pin}}\n";
            std::string hpp_logic_pin_template = "    dut::XData {{logic_pin}};\n";
            std::string cpp_reinit_pins_template = "    this->{{logic_pin}}.ReInit({{logic_pin_length}}, dut::IOType::{{logic_pin_type}}, \"{{logic_pin}}\");\n";
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
                    if (src_filename.compare("top.sv") == 0 || src_filename.compare("top.v") == 0)
                        dst_filename = dst_dir + "/" + dst_module_name + "_" + src_filename;
                    else
                        dst_filename = dst_dir + "/" + src_filename;
                    MESSAGE("Render file: %s to %s\n", src_filename.c_str(), dst_filename.c_str());
                    dst_file_content = env.render_file(entry.path().string(), data);
                    write_file(dst_filename, dst_file_content);
                }
            }

            MESSAGE("Generate DPI files successfully!");
            return 0;
        }

        void gen_main(int argc, char **argv)
        {
            // config
            // std::string src_dir = "./xdut_template/";
            // std::string dst_dir = "./tmp/";
            // std::string src_module_name = "Cache";
            // std::string dst_module_name = "";
            // std::string filename = "cache.v";
            // assert file or dir exists

            std::string file;

            cxxopts::Options options("XDut Gen", "XDut Generate. \nConvert DUT(*.v) to C++ DUT libs.\n");

            options.add_options()("f,file", "DUT .v file", cxxopts::value<std::string>(file));

            options.add_options()("s,source_dir", "Template Files", cxxopts::value<std::string>());
            options.add_options()("t,target_dir", "Gen Files in the target dir", cxxopts::value<std::string>());

            options.add_options()("sdut,source_module_name", "Pick the module in DUT .v file", cxxopts::value<std::string>());
            options.add_options()("tdut,target_module_name", "Set the module name and file name of target DUT", cxxopts::value<std::string>());

            options.add_options()("v,vflag", "User defined verilator args, split by comma. Eg: -v -x-assign=fast,-Wall,--trace. Or a file contain params.",
                                  cxxopts::value<std::string>());
            options.add_options()("h,help", "Print usage");

            auto opts = options.parse(argc, argv);
            if (opts.count("help") || !opts.count("file") || !opts.count("source_dir") || !opts.count("target_dir"))
            {
                MESSAGE("%s", options.help().c_str());
                exit(0);
            }

            std::string filename = opts["file"].as<std::string>();
            std::string src_dir = opts["source_dir"].as<std::string>();
            std::string dst_dir = opts["target_dir"].as<std::string>();
            std::string src_module_name = opts["source_module_name"].as<std::string>();
            std::string dst_module_name = opts["target_module_name"].as<std::string>();

            // parse
            std::vector<sv_pin_member> pin = parse_sv_pin(filename, src_module_name);
            // gen
            gen_dpi(pin, src_module_name, dst_module_name, src_dir, dst_dir);
            // copy verilog source
            std::filesystem::copy_file(filename, dst_dir + "/" + dst_module_name + ".v", std::filesystem::copy_options::overwrite_existing);
        }
    }
}
