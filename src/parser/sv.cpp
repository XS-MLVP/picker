#include "picker.hpp"
#include "parser/sv.hpp"
#include "parser/exprtk.hpp"

#define token_res(i)                                                           \
    (std::string(module_token[i]["tag"])).size() > 1 ?                         \
        (std::string)module_token[i]["text"] :                                 \
        (std::string)module_token[i]["tag"]

#define param_res(var) parameter_var.count(var) ? parameter_var[var] : var

namespace picker { namespace parser {

    std::vector<picker::sv_signal_define> sv_pin(std::string &filename,
                                         std::string &src_module_name)
    {
        std::string raw_filename;
        try
        {
            std::filesystem::path filepath = filename;
            raw_filename = filepath.filename();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return std::vector<picker::sv_signal_define>();
        }

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
        // nlohmann::json res_token;
        // for (auto it = module_token.begin(); it != module_token.end();) {
        //     auto itemV    = it.value();
        //     std::string a = itemV["tag"];
        //     if (a.size() > 1 ) res_token.push_back(itemV);
        //     if (a == "+" || a=="-" || a=="*" || a=="/" || a=="=" || a=="<" ||
        //     a==">" || a=="&" || a=="|" || a=="!" || a=="~" || a=="^" ||
        //     a=="%" || a=="?" || a==":" || a=="," || a==";" || a=="." ||
        //     a=="(" || a==")" || a=="[" || a=="]" || a=="{" || a=="}" ||
        //     a=="@" || a=="#" || a=="$") {
        //         it = module_token.erase(it);
        //     } else
        //     it++;
        // }
        // module_token = res_token;

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
        std::map<std::string, std::string> parameter_var;
        std::vector<picker::sv_signal_define> pin;
        for (int j = 0; j < module_token.size(); j++)
            if (module_token[j]["tag"] == "module"
                && module_token[j + 1]["text"] == src_module_name) {
                std::string pin_type, pin_high, pin_low;
                for (int i = j + 2; i < module_token.size(); i++) {
                    printf("%s\n", module_token[i]["tag"].dump().c_str());

                    // Check if is parameter
                    if (module_token[i]["tag"] == "parameter") {
                        pin_type = "parameter";
                        continue;
                    }

                    // Record pin type
                    if (module_token[i]["tag"] == "input"
                        || module_token[i]["tag"] == "output"
                        || module_token[i]["tag"] == "inout") {
                        pin_type = module_token[i]["tag"];
                        pin_high.clear();
                        pin_low.clear();
                        if (module_token[++i]["tag"] == "[") {
                            while (module_token[++i]["tag"] != ":")
                                pin_high += param_res(token_res(i));
                            while (module_token[++i]["tag"] != "]")
                                pin_low += param_res(token_res(i));
                        } else {
                            i--;
                            pin_high = "-1";
                        }

                        continue;
                    }

                    if (pin_type == "parameter") {
                        if (module_token[i]["tag"] == "SymbolIdentifier") {
                            std::string parameter_name =
                                module_token[i++]["text"];
                            std::string parameter_value;
                            while (module_token[++i]["tag"] != ","
                                   && module_token[i]["tag"] != ")") {
                                parameter_value += param_res(token_res(i));
                            }
                            parameter_var[parameter_name] = parameter_value;
                            printf("parameter_name: %s, parameter_value: %s\n",
                                   parameter_name.c_str(),
                                   parameter_value.c_str());
                        }
                        continue;
                    }

                    // printf("%s\n", module_token[i]["tag"].dump().c_str());

                    // lambda function to parse and calc pin length wich exprtk
                    exprtk::parser<double> parser;
                    exprtk::expression<double> expression;
                    auto calc_pin_length = [&](const std::string &exp) {
                        if (parser.compile(exp, expression)) {
                            printf("pin length expression: %s as %f\n",
                                   exp.c_str(), expression.value());
                            return expression.value();
                        } else {
                            FATAL(
                                "Failed to parse pin length expression: %s .\n",
                                exp.c_str());
                        }
                    };

                    // If is pin
                    sv_signal_define tmp_pin;
                    tmp_pin.logic_pin_type = pin_type;

                    if (module_token[i]["tag"]
                        == "SymbolIdentifier") { // Noraml pin
                        tmp_pin.logic_pin = module_token[i]["text"];
                        if (pin_high != "-1") {
                            tmp_pin.logic_pin_hb = calc_pin_length(pin_high);
                            tmp_pin.logic_pin_lb = calc_pin_length(pin_low);
                        } else {
                            tmp_pin.logic_pin_hb = -1;
                        }
                        pin.push_back(tmp_pin);
                    } else if (module_token[i]["tag"] == ")") { // Escaped pin
                        goto module_out;
                    }
                }
            module_out:
                return pin;
            }
        pin.clear();
        return pin;
    }

    int sv(picker::export_opts &opts, std::vector<picker::sv_signal_define> &external_pin)
    {
        std::string dst_module_name = opts.target_module_name;

        external_pin = sv_pin(opts.file, opts.source_module_name);

        opts.target_module_name = opts.target_module_name.size() == 0 ?
                                      opts.source_module_name :
                                      opts.target_module_name;
        return 0;
    }
}} // namespace picker::parser