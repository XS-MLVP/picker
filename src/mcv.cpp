#include <iostream>
#include <mcv/mcv.hpp>
#include <mcv/cxxopts.hpp>
#include <algorithm>
#include <filesystem>

namespace mcv{

    #define PYTHON "python"
    #define GOLANG "go"
    #define JAVA "java"
    #define CPP "cpp"

    void parse_and_gen(inja::json &cfg, std::string name, std::vector<mcv::CMember> &var_and_fucs, std::string lang_to_gen, std::string target_dir)
    {
        MESSAGE("Gen(%s) with lang: %s", name.c_str(), lang_to_gen.c_str());
        if (lang_to_gen == PYTHON)
        {
            gen_python(cfg, var_and_fucs, target_dir);
        }
        else if (lang_to_gen == GOLANG)
        {
            gen_golang(cfg, var_and_fucs, target_dir);
        }
        else if (lang_to_gen == JAVA)
        {
            gen_java(cfg, var_and_fucs, target_dir);
        }
        else if (lang_to_gen == CPP)
        {
            gen_cpp(cfg, var_and_fucs, target_dir);
        }
        else
        {
            vassert(false, "lang(" + lang_to_gen + ") not spported");
        }
    }

    void mcv_main(int argc, char **argv)
    {
        auto time_start = mcv::vtime();
        // arg-parser
        // define
        std::string file;
        cxxopts::Options options("mcv", "Multi-language-based Chip Verification. \nConvert DUT(*.v) to other language libs.\n");

        options.add_options()
        ("file", "DUT .v file or cpp head file to convert", cxxopts::value<std::string>(file))
        ("l,lang", "Language to gen, select from [python, go, java, cpp], split by comma. Eg: --lang go,java. Default all",
                                                 cxxopts::value<std::string>())
        ("t,target", "Gen data in the target dir, default in current dir like _mcv_<name>_Ymd_HMS", cxxopts::value<std::string>())
        ("n,name", "set mode name, default name is Lxx from lxx.v", cxxopts::value<std::string>())
        ("v,vflag", "User defined verilator args, split by comma. Eg: -v -x-assign=fast,-Wall,--trace. Or a file contain params.", 
                                                 cxxopts::value<std::string>())
        ("e,exfiles", "extend input files, split by comma. Eg: -e f1,f2.", 
                                                 cxxopts::value<std::string>())
        ("c,cppflag", "User defined CPPFLAGS, split by comma. Eg: -c -Wall,-DVL_DEBUG. Or a file contain params.", 
                                                 cxxopts::value<std::string>())
        ("h,help", "Print usage")
        ("d,debug", "Enable debuging")
        ("k,keep", "keep intermediate files")
        ("o,overwrite", "Force generate .cpp from .v");

        options.parse_positional("file");
        options.positional_help("<dut_file_to_convert>");

        // check help & position args
        auto opts = options.parse(argc, argv);
        if (opts.count("help") || !opts.count("file"))
        {
            MESSAGE("%s", options.help().c_str());
            exit(0);
        }

        // extend input files
        std::string efiles;
        if (opts.count("exfiles"))
        {
            auto v_params = opts["exfiles"].as<std::string>();
            efiles += streplace(v_params, ",", " ");
            MESSAGE("exfiles: %s", efiles.c_str())
        }

        // parse cpp flags
        std::string cflags;
        if (opts.count("cppflag"))
        {
            auto v_params = opts["cppflag"].as<std::string>();
            if (file_exists(v_params))
            {
                cflags += read_params(v_params);
            }
            else
            {
                cflags += streplace(v_params, ",", " ");
            }
            MESSAGE("cflags: %s", cflags.c_str())
        }

        // parse verilator args
        std::string vflags = "-CFLAGS -fPIC ";
        if (opts.count("vflag"))
        {
            auto v_params = opts["vflag"].as<std::string>();
            if (file_exists(v_params))
            {
                vflags += read_params(v_params);
            }
            else
            {
                vflags += streplace(v_params, ",", " ");
            }
            MESSAGE("vflags: %s", vflags.c_str())
        }

        // parse module name
        std::string module_name = "";
        if (opts.count("name"))
        {
            module_name = opts["name"].as<std::string>();
        }

        // check target dir
        std::string work_dir = "";
        if (opts.count("target"))
        {
            work_dir = opts["target"].as<std::string>();
            // TBD: check validate
        }

        // check lang
        std::vector<std::string> langs_to_parse{JAVA, GOLANG, PYTHON, CPP}; // all langs
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
        auto verilator_root = trim(exec_result("verilator -V|grep ROOT|grep verilator|awk '{print $3}'", 0));

        DEBUG("find verilator with version: %s", verilator_version.c_str());
        vassert(!verilator_version.empty(), "Get verilator fail, check verilaor is installed.");
        vassert(!verilator_root.empty(), "Get verilator root fail, check verilaor -V.");

        auto verilator_include = verilator_root + "/include";
        MESSAGE("verilator include: %s", verilator_include.c_str());
        config.add_include_dir(verilator_include);
        config.add_include_dir(verilator_include + "/vltstd");
        
        mcfg[CFG_VERILATOR_INCLUDE] = verilator_include;
        mcfg[CFG_CPP_FLAGS] = cflags;
        mcfg[CFG_VERILATOR_VERSION] = verilator_version;

        cppast::libclang_parser parser(type_safe::ref(logger));

        cppast::cpp_entity_index idx;
        MESSAGE("Parse file: %s", file.c_str());
        vassert(file_exists(file), "File(" + file + ") not exits");
        auto file_sufx = lower_case(strsplit(file, ".").back());
        auto file_name = base_name(file, false);
        auto m_name = first_upercase(file_name);
        if (!module_name.empty())
        {
            m_name = first_upercase(module_name);
        }
        auto file_path = get_path(file);
        auto work_path = work_dir;
        if (work_path == "")
        {
            work_path = "./_mcv_"+m_name+"_"+fmtnow("%Y%m%d_%H%M%S");
        }
        if (file_sufx == "v")
        {
            // convert *.v file to *.h
            auto cmd = sfmt("verilator --build --cc --prefix %s --Mdir %s %s %s %s", 
                                    m_name.c_str(), work_path.c_str(),  vflags.c_str(), file.c_str(), efiles.c_str());
            
            auto header_f = path_join({work_path, m_name}) + ".h";
            if (!file_exists(header_f) || opts["overwrite"].as<bool>())
            {
                MESSAGE("Convert: %s => %s:\n%s", file.c_str(), header_f.c_str(), cmd.c_str());
                exec_result(cmd, 0);
            }
            mcfg[CFG_VERILOG_IO_VARS] = get_verilog_inoutput_type(file);
            file = header_f;
        }
        else if (file_sufx == "h")
        {
            // PASS
        }
        else
        {
            vassert(false, file + " not supported");
        }
        // pase *.h file
        auto cppparsed = parser.parse(idx, file, config);
        auto parsed = parse_cpp_public_items(cppparsed, m_name);
        if (is_debug)
        {
            print_cpp(cppparsed);
        }
        mcfg[CFG_MODEL_NAME] = m_name;
        for (auto lan : langs)
        {
            parse_and_gen(mcfg, m_name, parsed, lan, work_path);
        }

        // clean up intermediate files
        if(!opts.count("keep")){
            for (auto &e: std::vector<std::string>{"cpp", "h", "a", "o", "d", "mk", "build", "dat"}){
                auto cmd_clean = std::string("rm -f "+work_path+"/*."+e);
                exec_result(cmd_clean, 0);
            }
        }
        
        // parse and gen
        auto time_end = vtime();
        MESSAGE("Gen complete, time cost: %.2f s", (time_end - time_start) / 1000000.0);
    }
}

int main(int argc, char **argv)
{
    mcv::mcv_main(argc, argv);
    return 0;
}
