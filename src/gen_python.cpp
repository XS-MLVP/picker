#include <mcv/mcv.hpp>

namespace mcv
{
    std::string nake_args(std::string arg_str)
    {
        std::string ret;
        for (auto &v : strsplit(arg_str, ","))
        {
            auto a = trim(strsplit(strsplit(v, " ").back(), "=").front());
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

    std::string conver_pytype(std::string t)
    {
        if (strconatins(t, "char"))
        {
            return "str";
        }
        return streplace(streplace(t, {"<", ":"}, "_"), ">");
    }

    std::string py_args(std::string args, std::set<std::string> &types_collection)
    {
        std::string ret;
        if (args.empty())
        {
            return ret;
        }
        // int a, int b -> a:int, b:int
        for (auto &a : strsplit(args, ","))
        {
            auto v = strsplit(trim(streplace(a, {"*", "&", "const", "unsigned"})));
            auto name = v.back();
            if (strconatins(name, "=")){
                name = strsplit(name, "=").front();
            }
            auto type = conver_pytype(v.front());
            if (ret.empty())
            {
                ret = name + ":" + type;
            }
            else
            {
                ret = ret + ", " + name + ":" + type;
            }
            types_collection.insert(type);
        }
        return ret;
    }

    std::string py_ret(std::string str, std::set<std::string> &types_collection)
    {
        std::string ret;
        if (str.empty() || strconatins(str, "void"))
        {
            return ret;
        }
        auto type = conver_pytype(trim(streplace(str, {"&", "const", "unsigned"})));
        types_collection.insert(type);
        return "->" + type;
    }

    std::string py_emb(std::string str, std::set<std::string> &types_collection)
    {
        if (str.empty())
        {
            return "";
        }
        auto v = strsplit(trim(streplace(str, {"&", "const"})));
        auto name = v.back();
        auto type = conver_pytype(v.front());
        types_collection.insert(type);
        return name + ": " + type + " = None";
    }

    std::string py_types(std::set<std::string> &types_collection)
    {
        std::string ret = "_INNER_TYPE_ = str\n";
        std::vector<std::string> ignore{"int", "float", "duble", "str"};
        for (auto v : types_collection)
        {
            if (contians(ignore, v))
            {
                continue;
            }
            ret += v + " = _INNER_TYPE_\n";
        }
        return ret;
    }

    void gen_python(inja::json &cfg, std::vector<CMember> &var_and_fucs, std::string target_dir)
    {
        exec_result("mkdir " + path_join({target_dir, "python"}), 1);
        auto fname = cfg[CFG_MODEL_NAME].template get<std::string>();
        auto vrinc = cfg[CFG_VERILATOR_INCLUDE].template get<std::string>();
        auto pyccp = path_join({target_dir, fname + "_python.cpp"});
        auto pybsh = path_join({target_dir, fname + "_python.build"});
        auto pymak = path_join({target_dir, fname + "_python.mk"});
        auto pyinc = path_join({target_dir, "vall.h"});
        auto pymcv = path_join({target_dir, "python", fname + ".py"});

        auto data = std::map<std::string, std::string>{
            {"dut_name", fname},
            {"mode_name", "_" + fname},
            {"mcv_dut_funcs", ""},
            {"cpp_flages", cfg["CFG_CPP_FLAGS"].template get<std::string>()}
        };

        // Gen MCVWrapper functions
        std::set<std::string> cpp_types;
        std::string wp_indent = "    ";
        std::string ut_indent = "        ";
        std::string wp_funcs, wp_getset, ut_funcs, ut_getset;
        std::string py_indent = "    ";
        std::string py_dut_members, py_dut_funcs;
        std::string template_data_define;

        for (auto &v : var_and_fucs)
        {
            if (v.type == "fuc")
            {
                auto func = func_wrapper(v);
                auto func_wtxt = sfmt("%s %s(%s){return this->dut->%s(%s);}\n",
                                      func["ret"].c_str(), func["name"].c_str(), func["args"].c_str(), func["name"].c_str(), func["nkargs"].c_str());
                auto func_utxt = sfmt(".def(\"%s\", &MCVWrapper::%s)\n", func["name"].c_str(), func["name"].c_str());

                auto pyargs = py_args(func["args"], cpp_types);
                auto pyret = py_ret(func["ret"], cpp_types);
                auto func_ptxt = sfmt("def %s(self%s)%s:\n        pass\n", func["name"].c_str(), pyargs.c_str(), pyret.c_str());

                if (wp_funcs.empty())
                {
                    wp_funcs = func_wtxt;
                    ut_funcs = func_utxt;
                    py_dut_funcs = func_ptxt;
                }
                else
                {
                    wp_funcs = wp_funcs + wp_indent + func_wtxt;
                    ut_funcs = ut_funcs + ut_indent + func_utxt;
                    py_dut_funcs = py_dut_funcs + py_indent + func_ptxt;
                }
            }
            else if (v.type == "var")
            {
                auto itm = v.name.c_str();
                auto ret = trim(streplace(v.define, itm, ""));
                auto getset_wtxt = sfmt("void set_%s(%s){this->dut->%s=%s;}\n%s%s get_%s(){return this->dut->%s;}\n",
                                        itm, v.define.c_str(), itm, itm, wp_indent.c_str(), ret.c_str(), itm, itm);

                auto getset_utxt = sfmt(".def_property(\"%s\", &MCVWrapper::get_%s, &MCVWrapper::set_%s)\n",
                                        itm, itm, itm);
                auto pymemb = py_emb(v.define, cpp_types) + "\n";

                if (wp_getset.empty())
                {
                    wp_getset = getset_wtxt;
                    ut_getset = getset_utxt;
                    py_dut_members = pymemb;
                }
                else
                {
                    wp_getset = wp_getset + wp_indent + getset_wtxt;
                    ut_getset = ut_getset + ut_indent + getset_utxt;
                    py_dut_members = py_dut_members + py_indent + pymemb;
                }
            }
            else if (v.type == "inc")
            {
                // ignore include
            }
            else if (v.type == "class"){
                auto df_str = streplace(streplace(v.name, "<", "("), ">", ")");
                if (!strconatins(template_data_define, df_str)){
                    df_str = "MCV_DEF_" + df_str + "\n";
                    if(!template_data_define.empty()){
                        df_str = "    " + df_str;
                    }
                    template_data_define += df_str;
                }
            }
        }
        
        data["mcv_wrapper_funcs"] = wp_funcs;
        data["mcv_wrapper_getset"] = wp_getset;
        data["mcv_dut_getset"] = trim(ut_getset, "\n");
        data["mcv_dut_funcs"] = trim(ut_funcs, "\n");
        data["python_dut_members"] = py_dut_members;
        data["python_dut_funcs"] = py_dut_funcs;
        data["python_dut_types"] = py_types(cpp_types);
        data["verilator_include"] = vrinc;
        data["template_data_define"] =  trim(template_data_define, "\n");

        auto pytmp = mcv_file("template/python/pybind11.cpp");
        auto mctmp = mcv_file("template/python/mcv.py");
        auto vatmp = mcv_file("template/python/vall.h");
        auto mktmp = mcv_file("template/python/python.mk");

        write_file(pyccp, template_rander(pytmp, data));
        write_file(pymcv, template_rander(mctmp, data));
        write_file(pyinc, template_rander(vatmp, data));
        write_file(pymak, template_rander(mktmp, data));

        auto build_cmd = "cd `dirname $0`\nrm *.o\nmake -f " + fname + "_python.mk PYTHONLIB\n";
        write_file(pybsh, build_cmd);
        // MESSAGE("build python with cmd:\n%s", build_cmd.c_str());
        exec_result("bash " + pybsh, 1);
    }
}
