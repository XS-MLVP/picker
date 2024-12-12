#include "picker.hpp"
#include "parser/sv.hpp"
#include "parser/exprtk.hpp"

namespace picker { namespace parser {

    void pre_process_sv(picker::pack_opts &opts, std::string filepath, std::string &filename)
    {
        // create file
        filename = extract_name(filepath, '/', 0);
        filename = extract_name(filename, '.', 1);
        std::filesystem::create_directory(filename + "_tmp");
        // parse sv to json
        std::string command = "verible-verilog-syntax --export_json --printtokens " + filepath + " > " + filename
                              + "_tmp/" + filename + ".json";
        int result = std::system(command.c_str());
        // createDirectoryRecursive(folderPath);
        if (result == 0) {
            std::cout << "parse " + filename + ".sv successfully." << std::endl;
        } else {
            std::cout << "failed to parse " + filename << std::endl;
            exit(0);
        }
    }

    std::string get_macro_define(std::string macro_path, std::string macro_name)
    {
        char current_path[PATH_MAX];
        if (getcwd(current_path, sizeof(current_path)) == nullptr) {     // 使用getcwd函数获取当前工作目录
            throw std::runtime_error("Error getting current directory"); // 如果获取失败，抛出异常
        }
        std::array<char, 128> buffer;
        std::string result;
        std::string script_path = picker::get_template_path() + "/uvm/get_macro_value.sh";
        std::string command     = script_path + " " + std::string(current_path) + "/" + macro_path + " " + macro_name;

        std::cout << command << std::endl;

        // 创建一个自定义的 deleter
        auto deleter = [](FILE *fp) { pclose(fp); };
        std::unique_ptr<FILE, decltype(deleter)> pipe(popen(command.c_str(), "r"), deleter);
        if (!pipe) { throw std::runtime_error("popen() failed!"); }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) { result += buffer.data(); }
        return result;
    }

    void handle_basic_type(std::string data_type, uvm_parameter &parameter)
    {
        if (data_type == "string") {
            // person_data["type"] = "str";
        } else if (data_type == "int") {
            parameter.byte_count = 4;
            parameter.bit_count  = 32;

        } else if (data_type == "shortint") {
            parameter.bit_count  = 16;
            parameter.byte_count = 2;

        } else if (data_type == "longint") {
            parameter.bit_count  = 64;
            parameter.byte_count = 8;

        } else if (data_type == "byte") {
            parameter.bit_count  = 8;
            parameter.byte_count = 1;
        }
    }

    void handle_array(int &j, auto &module_token, uvm_parameter &parameter, std::string macro_path)
    {
        std::string nbits = module_token[++j]["tag"], count;
        if (module_token[j + 1]["tag"] == "MacroIdentifier") {
            parameter.macro_name = extract_name(module_token[j + 1]["text"], '`', 0);
            if (module_token[j + 2]["tag"] == "-") {
                count               = get_macro_define(macro_path, extract_name(module_token[j + 1]["text"], '`', 0));
                parameter.bit_count = std::stod(count);
            } else {
                count               = get_macro_define(macro_path, module_token[j + 1]["text"]);
                parameter.bit_count = std::stod(count) + 1;
            }
        } else {
            parameter.macro_name = "0";
            count                = module_token[j + 1]["text"];
            parameter.bit_count  = std::stod(count) + 1;
        }
        parameter.byte_count = std::ceil(parameter.bit_count / 8.0);

        while (module_token[j + 1]["tag"] != "]") {
            if (module_token[j + 1]["tag"] == ":" || module_token[j + 1]["tag"] == "-")
                nbits += module_token[++j]["tag"];
            else
                nbits += module_token[++j]["text"];
        }
        nbits += module_token[++j]["tag"];
        parameter.name = module_token[++j]["text"];
        nbits          = nbits + parameter.name;
    }

    //解析sv文件
    uvm_transaction_define parse_sv(std::string filepath, std::string filename, std::string macro_path)
    {
        uvm_transaction_define transaction;
        auto json_result = read_file(filename + "_tmp/" + filename + ".json");

        // 删除tmp文件夹
        std::filesystem::path deletefolderPath = filename + "_tmp";
        std::filesystem::remove_all(deletefolderPath);
        nlohmann::json module_json = nlohmann::json::parse(json_result);
        std::string current_index  = "0";

        //指明json路径
        auto module_token = module_json[filepath]["tokens"];
        inja::json data;
        transaction.filepath = filepath;
        transaction.version  = version();
        transaction.data_now = fmtnow();
        for (int i = 0; i < module_token.size(); i++) {
            if (module_token[i]["tag"] == "class") {
                transaction.name  = module_token[i + 1]["text"];
                data["className"] = transaction.name;
                break;
            }
        }

        for (int j = 0; j < module_token.size(); j++) {
            std::string data_type, variableName, nbits, name1, numsstring, macro_name;
            int numsOrigin, nums = 1, macro = 0;
            uvm_parameter parameter;

            if (module_token[j]["tag"] == "rand") {
                data_type            = module_token[++j]["tag"];
                parameter.byte_count = 1;
                parameter.bit_count  = 1;

                handle_basic_type(data_type, parameter);

                if (module_token[j + 1]["tag"] == "[") {
                    handle_array(j, module_token, parameter, macro_name);
                } else {
                    parameter.name = module_token[++j]["text"];
                }
                parameter.is_marcro  = macro;
                parameter.macro_name = macro_name;

                if (macro == 1) {
                    current_index = current_index + " + " + macro_name;
                } else {
                    current_index = current_index + " + " + std::to_string(nums);
                }

                transaction.parameters.push_back(parameter);
            }
        }
        return transaction;
    }

    int uvm(picker::pack_opts &opts, std::string &path, std::string &filename, uvm_transaction_define &uvm_transaction)
    {
        std::string macro_define;
        pre_process_sv(opts, path, filename);
        uvm_transaction = parse_sv(path, filename, macro_define);
        return 0;
    }

}} // namespace picker::parser