#include <iostream>
#include <mcv/mcv.hpp>
#include <mcv/cxxopts.hpp>
#include <mcv/cpp_parser.hpp>


void mcv_main(int argc, char **argv) {
    // arg-parser
    // define
    cxxopts::Options options("mcv", "Multi-language-based Chip Verification");
    options.add_options()
    ("files", "Cpp head files to parse", cxxopts::value<std::vector<std::string>>())
    ("l,lang", "Language to gen, select from [python, go, java], split by comma. Eg: --lang go,java. Default all", 
        cxxopts::value<std::string>())
    ("t,target", "Gen data to target dir, default cpp file dir", cxxopts::value<std::string>())
    ("h,help", "Print usage")
    ("d,debug", "Enable debuging");
    options.parse_positional("files");
    options.positional_help("<hpp_files_to_parse>");

    // check help & position args
    auto result = options.parse(argc, argv);
    if (result.count("help") || !result.count("files")){
        MESSAGE("%s",options.help().c_str());
        exit(0);
    }

    // check lang
    std::vector<std::string> langs_to_parse{"java", "go", "python"}; // all langs
    auto langs = langs_to_parse;
    if (result.count("lang")) {
        langs = {};
        for (std::string v : strsplit(result["lang"].as<std::string>(), ",")){
            v = trim(v);
            if(!contians<std::string>(langs_to_parse, v)){
                std::cout << "lang: "<< v <<" not support" << std::endl;
                std::cout << options.help() << std::endl;
                exit(-1);
            }
            if(contians(langs, v)){
                continue;
            }
            langs.push_back(v);
        };
    }

    // parse and gen
    std::cout << result.arguments_string();
}


int main(int argc, char **argv) {
    MESSAGE("13123123:%d\n", 123);
    mcv_main(argc, argv);
    return 0;
}
