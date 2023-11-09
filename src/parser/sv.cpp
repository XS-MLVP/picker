#include "mcv.hpp"
#include "codegen/json.hpp"
#include "parser/sv.hpp"

namespace mcv { namespace parser {

    std::string capitalize_first_letter(const std::string &str)
    {
        if (str.empty()) return str;
        std::string result = str;
        if (result[0] >= 'a' && result[0] <= 'z')
            result[0] = result[0] - 'a' + 'A';
        return result;
    };

    std::vector<sv_pin_member> sv_pin(std::string &filename,
                                      std::string &src_module_name)
    {
        // call verible-verilog-syntax
        std::string syntax_cmd =
            "verible-verilog-syntax --export_json --printtokens " + filename;
        std::string verible_result = exec(syntax_cmd.c_str());

        // nlohmann parse json
        nlohmann::json module_json = nlohmann::json::parse(verible_result);

        // filter json like 标点符号等
        auto module_token = module_json[filename]["tokens"];
        for (auto it = module_token.begin(); it != module_token.end();) {
            auto itemV    = it.value();
            std::string a = itemV["tag"];
            (a.size() < 5) ? (it = module_token.erase(it)) : (it++);
        }
        // 不指明解析的module名，则默认解析文件中最后一个module
        if (src_module_name.length() == 0) {
            std::vector<std::string> module_list;
            for (int i = 0; i < module_token.size(); i++)
                if (module_token[i]["tag"] == "module")
                    module_list.push_back(module_token[i + 1]["text"]);
            src_module_name = module_list.back();
        }
        // 解析module_token，将解析好的pin_name\pin_length\pin_type都push进pin数组并返回。
        std::vector<sv_pin_member> pin;
        for (int j = 0; j < module_token.size(); j++)
            if (module_token[j]["tag"] == "module"
                && module_token[j + 1]["text"] == src_module_name) {
                for (int i = j; i < module_token.size(); i++) {
                    if (module_token[i]["tag"] == "input"
                        || module_token[i]["tag"] == "output") {
                        sv_pin_member tmp_pin;
                        tmp_pin.logic_pin_type =
                            capitalize_first_letter(module_token[i]["tag"]);
                        if (module_token[i + 1]["tag"] == "SymbolIdentifier") {
                            tmp_pin.logic_pin = module_token[i + 1]["text"];
                            tmp_pin.logic_pin_length = 0;
                        } else if (module_token[i + 1]["tag"]
                                   == "TK_DecNumber") {
                            std::string pin_high = module_token[i + 1]["text"];
                            std::string pin_low  = module_token[i + 2]["text"];
                            tmp_pin.logic_pin    = module_token[i + 3]["text"];
                            tmp_pin.logic_pin_length =
                                std::stoi(pin_high) - std::stoi(pin_low) + 1;
                        }
                        pin.push_back(tmp_pin);
                    } else if (module_token[i]["tag"] == "endmodule")
                        break;
                }
            }
        return pin;
    }

    std::vector<sv_pin_member> sv(cxxopts::ParseResult opts)
    {
        std::string filename = opts["file"].as<std::string>();
        std::string src_module_name =
            opts["source_module_name"].as<std::string>();

        return sv_pin(filename, src_module_name);
    }
}} // namespace mcv::parser