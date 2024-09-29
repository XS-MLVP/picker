#include <bits/stdc++.h>
#include "codegen/sv.hpp"

namespace picker { namespace codegen {

    void gen_uvm_code(inja::json data,std::string templateName,std::string outputName){
    
        //std::cout << className << std::endl;
        std::ifstream template_file(templateName);
        std::string template_str((std::istreambuf_iterator<char>(template_file)), std::istreambuf_iterator<char>());
        template_file.close();

        inja::Environment env;
        std::string rendered = env.render(template_str, data);
        std::ofstream output_file(outputName);
        output_file << rendered;
        output_file.close();
    }

    // namespace sv
    void gen_uvm_param(picker::pack_opts &opts,uvm_transaction_define transaction,std::string filename){

        std::string folderPath = filename;
        if(std::filesystem::exists(filename) && !opts.force)
        {
            PK_MESSAGE("folder already exists");
            exit(0);
        }else {
            if(std::filesystem::exists(filename)){
                    std::filesystem::path folderPath = filename;
                    std::filesystem::remove_all(filename);
                }      
            std::filesystem::create_directory(filename);
            if (opts.example){
                std::filesystem::create_directory(filename+"/example");
            }
        }

        inja::json data;
        std::string erro_message;
        auto python_location =
            picker::get_xcomm_lib("python/xspcomm", erro_message);
        if (python_location.empty()) { PK_FATAL("%s\n", erro_message.c_str()); }
        data["__XSPCOMM_PYTHON__"] = python_location;
        auto xspcomm_include_location =
            picker::get_xcomm_lib("include", erro_message);
        if (python_location.empty()) { PK_FATAL("%s\n", erro_message.c_str()); }
        data["__XSPCOMM_INCLUDE__"] = xspcomm_include_location;
        data["variables"] = inja::json::array();
        data["useType"] = 1;
        data["filepath"] = transaction.filepath;
        data["version"] = transaction.version;
        data["datenow"] = transaction.data_now;
        data["className"] = transaction.name;
        int byte_stream_count= 0;
        for (int i = 0; i<transaction.parameters.size(); i++){
            inja::json parameter;
            parameter["nums"] = transaction.parameters[i].byte_count;
            parameter["bit_count"] = transaction.parameters[i].bit_count;
            parameter["macro"] = transaction.parameters[i].is_marcro;
            parameter["name"] = transaction.parameters[i].name;
            parameter["macro_name"] = transaction.parameters[i].macro_name;
            parameter["start_index"] = "0";
            parameter["end_index"] = "0";
            data["variables"].push_back(parameter);
            byte_stream_count += transaction.parameters[i].byte_count;
        }
        data["byte_stream_count"] = byte_stream_count;
        std::string template_path = picker::get_template_path();
        gen_uvm_code(data,template_path+"/uvm/xagent.py",filename+"/"+filename+"_xagent"+".py");
        gen_uvm_code(data,template_path+"/uvm/xagent.sv",filename+"/"+filename+"_xagent.sv");
        gen_uvm_code(data,template_path+"/uvm/example_python.py",filename+"/example/"+"example_python.py");
        gen_uvm_code(data,template_path+"/uvm/example_uvm.sv",filename+"/example/"+"example_uvm.sv");
        gen_uvm_code(data,template_path+"/uvm/Makefile",filename+"/example/"+"Makefile");
        std::cout << "generate " + filename + " code successfully." << std::endl;

    }

}} // namespace picker::codegen