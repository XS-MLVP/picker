#include "picker.hpp"
#include "parser/sv.hpp"
#include "parser/exprtk.hpp"

#define token_res(i)                                                           \
    (std::string(module_token[i]["tag"])).size() > 1 ?                         \
        (std::string)module_token[i]["text"] :                                 \
        (std::string)module_token[i]["tag"]

#define param_res(var) parameter_var.count(var) ? parameter_var[var] : var

namespace picker { namespace parser {



    std::vector<picker::sv_signal_define> sv_pin(nlohmann::json &module_token,
                                                 std::string &src_module_name)
    {
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

        PK_MESSAGE("want module: %s", src_module_name.c_str());

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
                        // Skip logic, wire, reg type. We export pins with logic type, these types are not needed yet.
                        i++;
                        if( module_token[i]["tag"] == "logic" || module_token[i]["tag"] == "wire" || module_token[i]["tag"] == "reg" ) {
                            i++;
                        }
                        // Parse pin length
                        if (module_token[i]["tag"] == "[") {
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
                            PK_FATAL(
                                "Failed to parse pin length expression: %s .\n",
                                exp.c_str());
                        }
                    };

                    // If is pin
                    sv_signal_define tmp_pin;
                    tmp_pin.module_name = src_module_name;
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

    std::map<std::string, int> parse_mname_and_numbers(std::vector<std::string> &name_and_nums){
        // eg: MouduleA,1,ModuleB,3,MouldeC,ModuleE,ModuleF
        std::string m_name = "";
        int num = 1;
        std::map<std::string, int> ret;
        for(auto v : name_and_nums){
            v = picker::trim(v);
            num = 1;
            if(picker::str_start_with_digit(v)){
                num = std::stoi(v);
                if(!m_name.empty()){
                    ret[m_name] = num;
                    m_name = "";
                }else{
                    PK_MESSAGE("Ignore num: %d, no matched Module find", num)
                }
            }else{
                vassert(!v.empty(), "Find empty Module name in --sname arg");
                if(!m_name.empty()){
                    ret[m_name] = num;
                }
                m_name = v;
            }
        }
        if(!m_name.empty())ret[m_name] = 1;
        return ret;
    }

    std::map<std::string, nlohmann::json> match_module_with_file(
        std::vector<std::string> files, std::vector<std::string> m_names
    ){
        std::map<std::string, nlohmann::json> ret;
        for(auto f: files){
            std::string rf;
            std::filesystem::path fpath = f;
            rf = fpath.filename();
            std::string syntax_cmd =
            "verible-verilog-syntax --export_json --printtokens " + f
            + "> /tmp/" + rf + ".json";
            exec(syntax_cmd.c_str());
            auto mjson = nlohmann::json::parse(read_file("/tmp/" + rf + ".json"));
            std::vector<std::string> module_list;
            auto module_token = mjson[f]["tokens"];
            for (int i = 0; i < module_token.size(); i++){
                if (module_token[i]["tag"] == "module")
                    module_list.push_back(module_token[i + 1]["text"]);
            }
            if(m_names.empty()){
                ret[module_list.back()] = mjson;
                return ret;
            }
            for(auto m: m_names){
                if(picker::contians(module_list, m)){
                    PK_MESSAGE("find module: %s in file: %s", m.c_str(), f.c_str())
                    ret[m] = mjson;
                    break;
                }
            }
        }
        for(auto m: m_names){
            vassert(ret.count(m), "Module: " + m + " not find in input file");
        }
        return ret;
    }

    int sv(picker::export_opts &opts, std::vector<picker::sv_module_define> &module_external_pin)
    {
        std::string dst_module_name = opts.target_module_name;
        std::vector<std::string> m_names;
        std::map<std::string, nlohmann::json> m_json;
        std::map<std::string, int> m_nums;

        if(opts.source_module_name.empty()){
            if(opts.file.size()!=1)
            PK_FATAL("When module name not given (--sname),"
                     " can only parse one .v/.sv file (%d find!)", (int)opts.file.size())
            m_json = match_module_with_file(opts.file, opts.source_module_name);
            m_names = picker::key_as_vector(m_json);
        }else{
            m_nums = parse_mname_and_numbers(opts.source_module_name);
            m_names = picker::key_as_vector(m_nums);
            m_json = match_module_with_file(opts.file, m_names);
        }
        auto target_name = picker::join_str_vec(m_names, "_");
        for(auto &v: m_json){
            picker::sv_module_define sv_module;
            sv_module.module_name = v.first;
            sv_module.module_index = m_nums[sv_module.module_name];
            sv_module.pins = sv_pin(v.second, sv_module.module_name);
            module_external_pin.push_back(sv_module);
        }

        opts.target_module_name = opts.target_module_name.size() == 0 ?
                                      target_name:
                                      opts.target_module_name;
        return 0;
    }
}} // namespace picker::parser