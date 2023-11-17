#include "mcv.hpp"
#include "codegen/json.hpp"
#include "parser/sv.hpp"

namespace mcv { namespace parser {

    std::vector<sv_signal_define> sv_pin(std::string &filename,
                                         std::string &src_module_name)
    {
        // call verible-verilog-syntax
        // get real filename without path
        std::filesystem::path filepath = filename;
        std::string raw_filename       = filepath.filename();

        MESSAGE("start call!");
        std::string syntax_cmd =
            "verible-verilog-syntax --export_json --printtokens " + filename
            + "> /tmp/" + raw_filename + ".json";
        exec(syntax_cmd.c_str());

        // read verible-verilog-syntax result
        auto verible_result = read_file("/tmp/" + raw_filename + ".json");

        MESSAGE("start parse!");

        // nlohmann parse json
        nlohmann::json module_json = nlohmann::json::parse(verible_result);

        // filter json like 标点符号等
        MESSAGE("start filter!");
        auto module_token = module_json[filename]["tokens"];
        nlohmann::json res_token;
        for (auto it = module_token.begin(); it != module_token.end();) {
            auto itemV    = it.value();
            std::string a = itemV["tag"];
            if (a.size() > 1) res_token.push_back(itemV);
            it++;
        }
        module_token = res_token;

        MESSAGE("want module: %s", src_module_name.c_str());

        // 不指明解析的module名，则默认解析文件中最后一个module
        if (src_module_name.length() == 0) {
            std::vector<std::string> module_list;
            for (int i = 0; i < module_token.size(); i++)
                if (module_token[i]["tag"] == "module")
                    module_list.push_back(module_token[i + 1]["text"]);
            src_module_name = module_list.back();
        }
        // 解析module_token，将解析好的pin_name\pin_length\pin_type都push进pin数组并返回。
        std::vector<sv_signal_define> pin;
        for (int j = 0; j < module_token.size(); j++)
            if (module_token[j]["tag"] == "module"
                && module_token[j + 1]["text"] == src_module_name) {
                std::string pin_type, pin_high, pin_low;
                for (int i = j + 2; i < module_token.size(); i++) {
                    // printf("%s\n", module_token[i]["tag"].dump().c_str());
                    // Record pin type

                    if (module_token[i]["tag"] == "input"
                        || module_token[i]["tag"] == "output"
                        || module_token[i]["tag"] == "inout") {
                        pin_type = module_token[i]["tag"];

                        if (module_token[i + 1]["tag"] == "TK_DecNumber") {
                            // Vector pin
                            pin_high = module_token[++i]["text"];
                            pin_low  = module_token[++i]["text"];
                        } else {
                            // Noraml pin
                            pin_high = "-1";
                        }
                        continue;
                    }

                    // If is pin
                    sv_signal_define tmp_pin;
                    tmp_pin.logic_pin_type = pin_type;

                    // printf("%s\n", module_token[i]["tag"].dump().c_str());
                    if (module_token[i]["tag"]
                        == "SymbolIdentifier") { // Noraml pin
                        tmp_pin.logic_pin = module_token[i]["text"];
                        if (pin_high != "-1") {
                            tmp_pin.logic_pin_hb = std::stoi(pin_high);
                            tmp_pin.logic_pin_lb = std::stoi(pin_low);
                        } else {
                            tmp_pin.logic_pin_hb = -1;
                        }
                    } else
                        goto module_out;
                    pin.push_back(tmp_pin);
                }
            }
    module_out:
        return pin;
    }

    int sv(cxxopts::ParseResult opts,
           std::vector<sv_signal_define> &external_pin,
           std::string &src_module_name)
    {
        std::string filename = opts["file"].as<std::string>();

        external_pin = sv_pin(filename, src_module_name);
        return 0;
    }
}} // namespace mcv::parser