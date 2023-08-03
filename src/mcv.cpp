#include <iostream>
#include <mcv/mcv.hpp>
#include <mcv/cxxopts.hpp>
#include <algorithm>
#include <filesystem>

#define PYTHON "python"
#define GOLANG "go"
#define JAVA "java"

bool is_debug = false;

void parse_and_gen(inja::json &cfg, std::unique_ptr<cppast::cpp_file> &cppfile, std::string lang_to_gen, std::string target_dir)
{
    MESSAGE("Gen(%s) with lang: %s", (*cppfile).name().c_str(), lang_to_gen.c_str());
    if (lang_to_gen == PYTHON)
    {
        gen_python(cfg, cppfile, target_dir);
    }
    else if (lang_to_gen == GOLANG)
    {
        // TBD
    }
    else if (lang_to_gen == JAVA)
    {
        // TBD
    }
    else
    {
        vassert(false, "lang(" + lang_to_gen + ") not spported");
    }
}

void mcv_main(int argc, char **argv)
{
    auto time_start = vtime();
    // arg-parser
    // define
    std::vector<std::string> files;
    cxxopts::Options options("mcv", "Multi-language-based Chip Verification");
    options.add_options()("files", "Cpp head files to parse", cxxopts::value<std::vector<std::string>>(files))
    ("l,lang", "Language to gen, select from [python, go, java], split by comma. Eg: --lang go,java. Default all",
    cxxopts::value<std::string>())
    ("t,target", "Gen data to target dir, default cpp file dir", cxxopts::value<std::string>())
    ("h,help", "Print usage")
    ("d,debug", "Enable debuging")
    ("o,overwrite", "re-write results");

    options.parse_positional("files");
    options.positional_help("<hpp_files_to_parse>");

    // check help & position args
    auto opts = options.parse(argc, argv);
    if (opts.count("help") || !opts.count("files"))
    {
        MESSAGE("%s", options.help().c_str());
        exit(0);
    }

    // check target dir
    std::string work_dir = "";
    if (opts.count("target"))
    {
        work_dir = opts["target"].as<std::string>();
        // TBD: check validate
    }

    // check lang
    std::vector<std::string> langs_to_parse{JAVA, GOLANG, PYTHON}; // all langs
    auto langs = langs_to_parse;
    if (opts.count("lang"))
    {
        langs = {};
        for (std::string v : strsplit(opts["lang"].as<std::string>(), ","))
        {
            v = trim(v);
            if (!contians<std::string>(langs_to_parse, v))
            {
                std::cout << "lang: " << v << " not support" << std::endl;
                std::cout << options.help() << std::endl;
                exit(-1);
            }
            if (contians(langs, v))
            {
                continue;
            }
            langs.push_back(v);
        };
    }

    auto last = std::unique(files.begin(), files.end());
    files.erase(last, files.end());

    // parse files
    cppast::compile_flags flags;
    cppast::libclang_compile_config config;
    cppast::stderr_diagnostic_logger logger;
    inja::json mcfg;

    if (opts["debug"].as<bool>())
    {
        is_debug = true;
        logger.set_verbose(true);
    }
    config.set_flags(cppast::cpp_standard::cpp_11, flags);
    config.set_flags(cppast::cpp_standard::cpp_14, flags);
    config.set_flags(cppast::cpp_standard::cpp_17, flags);

    // check verilator
    auto verilator_version = exec_result("verilator --version", 1);
    auto verilator_root = exec_result("verilator -V|grep ROOT|grep verilator|awk '{print $3}'", 0);

    DEBUG("find verilator with version: %s", verilator_version.c_str());
    config.add_include_dir(verilator_root + "/include");
    mcfg[CFG_VERILATOR_INCLUDE] = verilator_root + "/include";

    cppast::libclang_parser parser(type_safe::ref(logger));
    for (auto f : files)
    {
        cppast::cpp_entity_index idx;
        MESSAGE("Parse file: %s", f.c_str());
        vassert(file_exists(f), "File(" + f + ") not exits");
        auto file_sufx = lower_case(strsplit(f, ".").back());
        auto file_name = first_upercase(base_name(f, false));
        auto file_path = get_path(f);
        auto work_path = work_dir;
        if (work_path == ""){
            work_path = file_path;
        }
        if (file_sufx == "v"){
            // convert *.v file to *.h
            auto cmd = sfmt("verilator --cc %s --prefix %s --Mdir %s --build", f.c_str(), file_name.c_str(), work_path.c_str());
            auto header_f = path_join({work_path, file_name}) + ".h";
            if (!file_exists(header_f) || opts["overwrite"].as<bool>()){
                MESSAGE("Convert: %s => %s", f.c_str(), header_f.c_str());
                exec_result(cmd, 0);
            }
            f = header_f;
        }else if (file_sufx == "h"){
            // PASS
        }else {
            vassert(false, f + " not supported");
        }
        // pase *.h file
        auto parsed = parser.parse(idx, f, config);
        mcfg[CFG_MODEL_NAME] = file_name;
        for (auto lan : langs)
        {
            parse_and_gen(mcfg, parsed, lan, work_path);
        }
    }
    // parse and gen
    auto time_end = vtime();
    MESSAGE("Gen complete, time cost: %.2f s", (time_end - time_start) / 1000000.0);
}


int main(int argc, char **argv)
{
    mcv_main(argc, argv);
    return 0;
}
