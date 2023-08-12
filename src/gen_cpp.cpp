#include <mcv/mcv.hpp>

namespace mcv
{
    void gen_cpp(inja::json &cfg, std::vector<mcv::CMember> &var_and_fucs, std::string target_dir)
    {
        auto fname = cfg[CFG_MODEL_NAME].template get<std::string>();
        auto vrinc = cfg[CFG_VERILATOR_INCLUDE].template get<std::string>();
        auto cpp_dir = path_join({target_dir, "cpp"});
        auto cpp_inc = path_join({target_dir, fname + ".h"});

        exec_result("mkdir " + cpp_dir, 1);
        exec_result("cp "+ cpp_inc + " " + cpp_dir + "/", 0);

        auto data = std::map<std::string, std::string>{
            {"dut_name", fname},
            {"cpplib_filename", "lib" + fname + ".so"},
            {"cpp_flages", cfg[CFG_CPP_FLAGS].template get<std::string>()},
            {"verilator_version", cfg[CFG_VERILATOR_VERSION].template get<std::string>()},
        };
        data["verilator_include"] = vrinc;

        // makefile
        auto mcvmk = path_join({target_dir, fname + "_cpp.mk"});
        auto libtmp = mcv_file("template/cpp/cpp.mk");
        write_file(mcvmk, template_rander(libtmp, data));
        auto tstmk = path_join({target_dir, "cpp", "Makefile"});
        auto tstmp = mcv_file("template/cpp/test.mk");
        write_file(tstmk, template_rander(tstmp, data));

        // test example
        auto example = path_join({target_dir, "cpp", "Test_"+fname+".cpp"});
        auto example_tmp = mcv_file("template/cpp/test.cpp");
        write_file(example, template_rander(example_tmp, data));

        // build script
        auto build_bash = path_join({target_dir, fname + "_cpp.build"});
        auto build_cmd = "cd `dirname $0`\nrm *.o\nmake -f " + fname + "_cpp.mk CPPLIB\n";
        write_file(build_bash, build_cmd);
        exec_result("bash " + build_bash, 1);
    }
}
