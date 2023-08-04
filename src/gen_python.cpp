#include <mcv/mcv.hpp>

std::string nake_args(std::string arg_str)
{
    std::string ret;
    for (auto &v : strsplit(arg_str, ","))
    {
        auto a = trim(strsplit(v, " ").back());
        if (ret.empty())
        {
            ret = a;
        }
        else
        {
            ret = ret + ", " + a;
        }
    }
    return ret;
}

std::map<std::string, std::string> func_wrapper(CMember &mb)
{
    vassert(mb.type == "fuc", "need function");
    // Get return type and args
    auto dfs = strsplit(mb.define, mb.name + "(");
    vassert(dfs.size() == 2, "parse funciton(" + mb.define + ") fail");
    auto ret = trim(streplace(dfs[0], "virtual"));
    auto argtxt = strsplit(dfs[1], ")");
    vassert(argtxt.size() == 2, "parse args from(" + mb.define + ") fail");
    auto arg = argtxt[0];
    return std::map<std::string, std::string>{
        {"ret", ret},
        {"name", mb.name},
        {"args", arg},
        {"nkargs", nake_args(arg)},
    };
}

void gen_python(inja::json &cfg, std::vector<CMember> &var_and_fucs, std::string target_dir)
{

    auto fname = cfg[CFG_MODEL_NAME].template get<std::string>();
    auto vrinc = cfg[CFG_VERILATOR_INCLUDE].template get<std::string>();
    auto pyccp = path_join({target_dir, fname + "_python.cpp"});
    auto pybsh = path_join({target_dir, fname + "_python.build"});

    auto data = std::map<std::string, std::string>{
        {"dut_name", fname},
        {"mode_name", "mcv"},
        {"mcv_dut_funcs", ""},
    };

    // Gen MCVWrapper functions
    std::string wp_indent = "    ";
    std::string ut_indent = "        ";
    std::string wp_funcs, wp_getset, ut_funcs, ut_getset;
    
    for (auto &v : var_and_fucs)
    {
        if (v.type == "fuc")
        {
            auto func = func_wrapper(v);
            auto func_wtxt = sfmt("%s %s(%s){return this->dut->%s(%s);}\n", 
                                   func["ret"].c_str(), func["name"].c_str(), func["args"].c_str(), func["name"].c_str(), func["nkargs"].c_str());
            auto func_utxt = sfmt(".def(\"%s\", &MCVWrapper::%s)\n", func["name"].c_str(), func["name"].c_str());

            if (wp_funcs.empty()){
                wp_funcs = func_wtxt;
                ut_funcs = func_utxt;
            }else{
                wp_funcs = wp_funcs + wp_indent + func_wtxt;
                ut_funcs = ut_funcs + ut_indent + func_utxt;
            }
        }else{
            auto itm = v.name.c_str();
            auto ret = trim(streplace(v.define, itm, ""));
            auto getset_wtxt = sfmt("void set_%s(%s){this->dut->%s=%s;}\n%s%s get_%s(){return this->dut->%s;}\n",
                                itm, v.define.c_str(), itm, itm, wp_indent.c_str(), ret.c_str(), itm, itm);
            
            auto getset_utxt = sfmt(".def_property(\"%s\", &MCVWrapper::get_%s, &MCVWrapper::set_%s)\n",
                                itm, itm, itm);
            
            if (wp_getset.empty()){
                wp_getset = getset_wtxt;
                ut_getset = getset_utxt;
            }else{
                wp_getset = wp_getset + wp_indent + getset_wtxt;
                ut_getset = ut_getset + ut_indent + getset_utxt;
            }
        }
    }
    data["mcv_wrapper_funcs"] = wp_funcs;
    data["mcv_wrapper_getset"] = wp_getset;
    data["mcv_dut_getset"] = trim(ut_getset, "\n");
    data["mcv_dut_funcs"] = trim(ut_funcs, "\n");

    auto pytmp = find_file(std::vector<std::string>{"template/python/pybind11.cpp",
                                                    "/etc/mcv/template/python/pybind11.cpp"});
    write_file(pyccp, template_rander(pytmp, data));

    auto build_file = fname + "__ALL.cpp " + fname + "_python.cpp";
    auto build_opts = std::string("-faligned-new -faligned-new -O3 -Wall -shared -std=c++17 -fPIC");
    auto build_python = "cd `dirname $0`\nc++ " + build_opts + " $(python3 -m pybind11 --includes) -I " + vrinc +
                        " -o mcv$(python3-config --extension-suffix) " + vrinc + "/verilated.cpp " + build_file + "\n";

    write_file(pybsh, build_python);
    MESSAGE("build python ...");
    exec_result("bash " + pybsh, 1);
}
