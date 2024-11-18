#include <iostream>
#include <algorithm>
#include <filesystem>
#include "picker.hpp"

picker::main_opts main_opts;
picker::export_opts export_opts;
picker::pack_opts pack_opts;
char *picker::lib_random_hash;

namespace picker {
bool is_debug = false;
}

void check_debug()
{
    if (std::getenv("PICKER_DEBUG") != NULL) {
        picker::is_debug = true;
    }
}

std::string to_base62(uint64_t num)
{
    const std::string base62_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string result;
    do {
        result = base62_chars[num % 62] + result;
        num >>= 6;
    } while (num > 0);
    return result;
}

int set_options_export_rtl(CLI::App &top_app)
{
    auto app = top_app.add_subcommand(
        "export",
        "Export RTL Projects Sources as Software libraries such as C++/Python");

    // Set DUT RTL Source File, Required
    app->add_option("file", export_opts.file,
                    "DUT .v/.sv source file, contain the top module")->delimiter(',')
        ->required();

    // Set DUT RTL Extra Source File List, Optional
    app->add_option(
        "--fs,--filelist", export_opts.filelists,
        "DUT .v/.sv source files, contain the top module, split by comma.\n"
        "Or use '*.txt' file  with one RTL file path per line to specify the file list")->delimiter(',');

    // Set DUT RTL Simulator, Optional, default is verilator
    app->add_option("--sim", export_opts.sim,
                    "vcs or verilator as simulator, default is verilator")
        ->default_val("verilator");

    // Set DUT RTL Language, Optional, default is python
    std::vector<std::string> select_languages = {"python", "cpp", "java",
                                                 "scala", "golang"};
    app->add_option(
           "--lang,--language", export_opts.language,
           "Build example project, default is python, choose cpp, java or python")
        ->default_val("python")
        ->check(CLI::IsMember(select_languages));

    // Set DUT RTL Source Dir, Optional, default is
    // ${picker_install_path}/../picker/template
    app->add_option(
           "--sdir,--source_dir", export_opts.source_dir,
           "Template Files Dir, default is ${picker_install_path}/../picker/template")
        ->default_val(picker::get_template_path());

    // Set DUT RTL Source Module Name, Optional, default is the last module in
    app->add_option(
        "--sname,--source_module_name", export_opts.source_module_name_list,
        "Pick the module in DUT .v file, default is the last module in the -f marked file")->delimiter(',');

    // Set Generated Software library module name, Optional, default is the same
    app->add_option(
        "--tname,--target_module_name", export_opts.target_module_name,
        "Set the module name and file name of target DUT, default is the same as source. \nFor example, -T top, will generate UTtop.cpp and UTtop.hpp with UTtop class");

    // Set DUT RTL Target Dir, Optional, default is TNAME
    app->add_option(
           "--tdir,--target_dir", export_opts.target_dir,
           "Target directory to store all the results. If it ends with '/' or is empty, \nthe directory name will be the same as the target module name")
        ->default_val("");

    // Set DUT RTL Internal Signal Config File, Optional, default is empty
    app->add_option(
        "--internal", export_opts.internal,
        "Exported internal signal config file, default is empty, means no internal pin");

    // Simulate frequncy while using VCS simulator, Optional, default is 100MHz
    app->add_option(
           "-F,--frequency", export_opts.frequency,
           "Set the frequency of the **only VCS** DUT, default is 100MHz, use Hz, KHz, MHz, GHz as unit")
        ->default_val("100MHz");

    // Dump wave file name, Optional, default is empty means don't dump wave
    app->add_option("-w,--wave_file_name", export_opts.wave_file_name,
                    "Wave file name, emtpy mean don't dump wave");

    // Enable coverage, Optional, default is OFF
    app->add_flag("-c,--coverage", export_opts.coverage,
                  "Enable coverage, default is not selected as OFF");

    // Enable copy xspcomm lib to DUT location
    app->add_option("--cp_lib, --copy_xspcomm_lib", export_opts.cp_lib,
                    "Copy xspcomm lib to generated DUT dir, default is true")->default_val(true);

    // User defined simulator compile args, passthrough. Eg: '-v -x-assign=fast
    // -Wall --trace' || '-C vcs -cc -f filelist.f'
    app->add_option(
        "-V,--vflag", export_opts.vflag,
        "User defined simulator compile args, passthrough. \nEg: '-v -x-assign=fast -Wall --trace' || '-C vcs -cc -f filelist.f'");

    // User defined gcc/clang compile command, passthrough. Eg:'-O3 -std=c++17
    // -I./include'
    app->add_option(
        "-C,--cflag", export_opts.cflag,
        "User defined gcc/clang compile command, passthrough. Eg:'-O3 -std=c++17 -I./include'");

    // app->add_option(
    //     "-r,--rename", export_opts.rename,
    //     "User defined gcc/clang compile command, passthrough. Eg:'-O3
    //     -std=c++17 -I./include'");

    app->add_flag("--verbose", export_opts.verbose, "Verbose mode");
    app->add_flag("-e,--example", export_opts.example,
                  "Build example project, default is OFF");
    app->add_option("--autobuild", export_opts.autobuild,
                    "Auto build the generated project, default is true")
        ->default_val(true);

    return 0;
}

int set_options_pack_message(CLI::App &top_app)
{
    auto app = top_app.add_subcommand(
        "pack", "Pack UVM transaction as a UVM agent and Python class");

    // Set DUT RTL Source File, Required
    app->add_flag(
        "-e,--example", pack_opts.example,
        "Generate example project based on transaction, default is OFF");

    app->add_flag(
        "-c,--force", pack_opts.force,
        "Force delete folder when the code has already generated by picker");

    app->add_option("file", pack_opts.files,
                    "Sv source file, contain the transaction define")
        ->required();

    app->add_option("-r,--rename", pack_opts.rename,
                    "Rename transaction name in picker generate code");

    return 0;
}

int set_options_main(CLI::App &app)
{
    app.add_flag("-v, --version", main_opts.version, "Print version");
    app.add_flag("--show_default_template_path",
                 main_opts.show_default_template_path,
                 "Print default template path");
    app.add_flag("--show_xcom_lib_location_cpp",
                 main_opts.show_xcom_lib_location_cpp,
                 "Print xspcomm lib and include location");
    app.add_flag("--show_xcom_lib_location_java",
                 main_opts.show_xcom_lib_location_java,
                 "Print xspcomm-java.jar location");
    app.add_flag("--show_xcom_lib_location_scala",
                 main_opts.show_xcom_lib_location_scala,
                 "Print xspcomm-scala.jar location");
    app.add_flag("--show_xcom_lib_location_python",
                 main_opts.show_xcom_lib_location_python,
                 "Print python module xspcomm location");
    app.add_flag("--show_xcom_lib_location_golang",
                 main_opts.show_xcom_lib_location_golang,
                 "Print golang module xspcomm location");
    app.add_flag("--check", main_opts.check,
                 "check install location and supproted languages");
    return 0;
}

int main(int argc, char **argv)
{
    check_debug();
    CLI::App app{"XDut Generate. \n"
                 "Convert DUT(*.v/*.sv) to C++ DUT libs.\n"};
    set_options_main(app);
    set_options_export_rtl(app);
    set_options_pack_message(app);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        // PK_MESSAGE("%s", app.help().c_str());
        return app.exit(e);
    }

    // handel main options here
    // if need version, print version
    if (main_opts.version) {
        PK_MESSAGE("version: %s-%s-%s%s", PROJECT_VERSION, GIT_BRANCH, GIT_HASH,
                GIT_DIRTY);
        exit(0);
    }

    // get random for random library name
    srand48(time(NULL));
    std::string hash_name   = to_base62(mrand48() << 32 | mrand48());
    picker::lib_random_hash = new char[hash_name.size() + 1];
    std::copy(hash_name.begin(), hash_name.end(), picker::lib_random_hash);
    picker::lib_random_hash[hash_name.size()] = '\0';

    if (main_opts.show_default_template_path) {
        auto temp_path = picker::get_template_path();
        if (temp_path.empty()) {
            PK_ERROR("Can't find default template path");
            exit(1);
        }
        PK_MESSAGE("%s", temp_path.c_str());
        exit(0);
    }

    // get xcom lib location
    std::string erro_message = "";
    if (main_opts.show_xcom_lib_location_cpp) {
        auto lib_location = picker::get_xcomm_lib("lib", erro_message);
        if (lib_location.size() == 0) {
            PK_ERROR("%s", erro_message.c_str());
            exit(1);
        }
        auto include_location = picker::get_xcomm_lib("include", erro_message);
        if (include_location.size() == 0) {
            PK_ERROR("%s", erro_message.c_str());
            exit(1);
        }
        PK_MESSAGE("Lib:     %s", lib_location.c_str());
        PK_MESSAGE("Include: %s", include_location.c_str());
        exit(0);
    }
    if (main_opts.show_xcom_lib_location_java) {
        auto java_location =
            picker::get_xcomm_lib("java/xspcomm-java.jar", erro_message);
        if (java_location.size() == 0) {
            PK_ERROR("%s", erro_message.c_str());
            exit(1);
        }
        PK_MESSAGE("%s", java_location.c_str());
        exit(0);
    }
    if (main_opts.show_xcom_lib_location_scala) {
        auto scala_location =
            picker::get_xcomm_lib("java/xspcomm-scala.jar", erro_message);
        if (scala_location.size() == 0) {
            PK_ERROR("%s", erro_message.c_str());
            exit(1);
        }
        PK_MESSAGE("%s", scala_location.c_str());
        exit(0);
    }
    if (main_opts.show_xcom_lib_location_python) {
        auto python_location = picker::get_xcomm_lib("python", erro_message);
        if (python_location.size() == 0) {
            PK_ERROR("%s", erro_message.c_str());
            exit(1);
        }
        PK_MESSAGE("%s", python_location.c_str());
        exit(0);
    }
    if (main_opts.show_xcom_lib_location_golang) {
        auto golang_location = picker::get_xcomm_lib("golang", erro_message);
        if (golang_location.size() == 0) {
            PK_ERROR("%s", erro_message.c_str());
            exit(1);
        }
        PK_MESSAGE("%s", golang_location.c_str());
        exit(0);
    }


    int lang_index            = 0;
    const char *check_libs[]  = {"include", "java/xspcomm-java.jar",
                                 "scala/xspcomm-scala.jar", "python", "golang"};
    const char *check_langs[] = {"Cpp", "Java", "Scala", "Python", "Golang"};
    std::map<std::string, int> lang_map = {
        {"cpp", 0}, {"java", 1}, {"scala", 2}, {"python", 3}, {"golang", 4}};
    if (main_opts.check) {
        PK_MESSAGE("[OK ] Version: %s-%s-%s%s", PROJECT_VERSION, GIT_BRANCH,
                GIT_HASH, GIT_DIRTY);
        PK_MESSAGE("[OK ] Exec path: %s", picker::get_executable_path().c_str());
        auto temp_path = picker::get_template_path();
        if (temp_path.empty()) {
            PK_MESSAGE("[Err] Can't find default template path");
        } else {
            PK_MESSAGE("[OK ] Template path: %s", temp_path.c_str());

        }
        int i = 0;
        for (auto lang : check_langs) {
            auto lib_location =
                picker::get_xcomm_lib(check_libs[i], erro_message);
            if (lib_location.size() == 0) {
                PK_ERROR("[Err] Support %6s (find: '%s' fail)", lang,
                      check_libs[i]);
            } else {
                PK_MESSAGE("[OK ] Support %6s (find: '%s' success)", lang,
                        lib_location.c_str());
            }
            i += 1;
        }
        exit(0);
    }
    if (export_opts.language.size() != 0) {
        lang_index = lang_map[export_opts.language];
        auto lib_location =
            picker::get_xcomm_lib(check_libs[lang_index], erro_message);
        if (lib_location.size() == 0) {
            PK_ERROR("[Err] Support %6s (find: '%s' fail)",
                  check_langs[lang_index], check_libs[lang_index]);
            exit(1);
        }
    }

    // check if app parsed export subcommand
    if (app.get_subcommands().size() == 0) {
        PK_MESSAGE("%s", app.help().c_str());
        PK_MESSAGE("No subcommand specified: need export or pack");
        exit(0);
    }

    // subcommand export
    if (app.get_subcommand_ptr("export")->parsed()) {
        std::vector<picker::sv_module_define> sv_module_result;
        std::vector<picker::sv_signal_define> internal_sginal_result;
        nlohmann::json signal_tree_json;
        picker::parser::sv(export_opts, sv_module_result);
        picker::parser::internal(export_opts, internal_sginal_result);

        auto sv_pin_result = picker::codegen::lib(export_opts, sv_module_result, internal_sginal_result,
                                                  signal_tree_json);

        std::map<std::string,
                 std::function<void(picker::export_opts &,
                                    std::vector<picker::sv_signal_define>,
                                    std::vector<picker::sv_signal_define>,
                                    nlohmann::json &
                                    )>> func_map = {
                {"cpp", picker::codegen::cpp},
                {"python", picker::codegen::python},
                {"java", picker::codegen::java},
                {"scala", picker::codegen::scala},
                {"golang", picker::codegen::golang},
            };
        func_map[export_opts.language](export_opts, sv_pin_result,
                                       internal_sginal_result,
                                       signal_tree_json);
        // build the result with make
        if (export_opts.autobuild) {
            const std::string cmd = "cd " + export_opts.target_dir + " && make";
            if (system(cmd.c_str()) != 0) {
                PK_MESSAGE("Build failed");
                exit(1);
            }
        }
        // subcommand pack
    } else if (app.get_subcommand_ptr("pack")->parsed()) {
        // todo
        picker::uvm_transaction_define uvm_transaction;
        std::string filepath, filename, macro_define;
        // std::queue<std::string> rnames = pack_opts.rname;
        int i = 0;
        if (pack_opts.files.size() != 0) {
            for (auto &path : pack_opts.files) {
                picker::parser::uvm(pack_opts, path, filename, uvm_transaction);
                if (i < pack_opts.rename.size()) {
                    filename             = pack_opts.rename[i];
                    uvm_transaction.name = pack_opts.rename[i];
                    i++;
                }
                picker::codegen::gen_uvm_param(pack_opts, uvm_transaction,
                                               filename);
            }
            exit(0);
        }
    }

    nlohmann::json sync_opts;
    return 0;
}
