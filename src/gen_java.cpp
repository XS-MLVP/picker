#include <mcv/mcv.hpp>

namespace mcv
{
    void gen_java(inja::json &cfg, std::vector<CMember> &var_and_fucs, std::string target_dir)
    {
        std::string sub_dir = "java";
        auto fname = cfg[CFG_MODEL_NAME].template get<std::string>();
        auto data = mcv::gen_cpp_so(cfg, var_and_fucs, target_dir, sub_dir);
        data["work_java_dir"] = path_join({current_path(), target_dir, sub_dir});

        // Generate memebers and functions from DUT
        // TBD

        // java wrapper
        auto javafile = path_join({target_dir, sub_dir, fname + ".java"});
        auto javatemp = mcv_file("template/java/javacpp.java");
        write_file(javafile, template_rander(javatemp, data));

        // build and test
        auto buildfile = path_join({target_dir, sub_dir, fname + ".build"});
        auto buildtemp = mcv_file("template/java/javacpp.build");
        write_file(buildfile, template_rander(buildtemp, data));
    }
}
