#include <mcv/mcv.hpp>

namespace mcv
{
    void gen_cpp(inja::json &cfg, std::vector<mcv::CMember> &var_and_fucs, std::string target_dir)
    {
        std::string sub_dir = "cpp";
        auto fname = cfg[CFG_MODEL_NAME].template get<std::string>();
        auto data = mcv::gen_cpp_so(cfg, var_and_fucs, target_dir, sub_dir);

        // test makefile
        auto tstmk = path_join({target_dir, sub_dir, "Makefile"});
        auto tstmp = mcv_file("template/cpp/test.mk");
        write_file(tstmk, template_rander(tstmp, data));

        // test example
        auto example = path_join({target_dir, sub_dir, "Test_"+fname+".cpp"});
        auto example_tmp = mcv_file("template/cpp/test.cpp");
        write_file(example, template_rander(example_tmp, data));
    }
}
