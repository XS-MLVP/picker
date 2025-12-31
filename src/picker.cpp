#include <iostream>
#include <algorithm>
#include <filesystem>
#include <unordered_set>
#include "picker.hpp"
#include "codegen/lib.hpp"
#include "codegen/mem_direct.hpp"
#include "codegen/cpp.hpp"
#include "codegen/python.hpp"
#include "codegen/java.hpp"
#include "codegen/scala.hpp"
#include "codegen/golang.hpp"
#include "codegen/lua.hpp"
#include "codegen/sv.hpp"
#include "codegen/uvm.hpp"
#include "codegen/firrtl.hpp"
#include "parser/parser_map.hpp"
#include "parser/internalcfg.hpp"
#include "parser/sv.hpp"
#include "parser/uvm.hpp"

picker::main_opts main_opts;
picker::export_opts export_opts;
picker::pack_opts pack_opts;
char *picker::lib_random_hash;
std::map<std::string, std::string> lang_lib_map  = {{"cpp", "lib"},
                                                    {"java", "java/xspcomm-java.jar"},
                                                    {"scala", "scala/xspcomm-scala.jar"},
                                                    {"python", "python"},
                                                    {"golang", "golang"},
                                                    {"lua", "lua/luaxspcomm.so"}};
std::map<std::string, std::string> display_names = {{"cpp", "Cpp"},       {"java", "Java"},     {"scala", "Scala"},
                                                    {"python", "Python"}, {"golang", "Golang"}, {"lua", "Lua"}};

namespace picker {
bool is_debug = false;
}

void check_debug()
{
    if (std::getenv("PICKER_DEBUG") != NULL) { picker::is_debug = true; }
}

std::string to_base62(uint64_t num)
{
    const std::string base62_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string result;
    do {
        result = base62_chars[num % 62] + result;
        num >>= 6;
    } while (num > 0);
    return result;
}

int set_options_export_rtl(CLI::App &top_app)
{
    auto app = top_app.add_subcommand("export", "Export RTL Projects Sources as Software libraries such as C++/Python");

    // Set DUT RTL Source File, conditionally required
    // Note: Required only when --sname/--source_module_name is NOT specified
    app->add_option("file", export_opts.file,
                    "DUT .v/.sv source file. Required if --sname is not specified.")
        ->delimiter(',');

    // Set DUT RTL Extra Source File List, Optional
    app->add_option("--fs,--filelist", export_opts.filelists,
                    "DUT .v/.sv source files, contain the top module, split by comma.\n"
                    "Or use '*.txt' file  with one RTL file path per line to specify the file list")
        ->delimiter(',');

    // Set DUT RTL Simulator, Optional, default is verilator
    app->add_option("--sim", export_opts.sim, "vcs, gsim or verilator as simulator, default is verilator")
        ->default_val("verilator");

    // Set DUT RTL RW Type, Optional, default is DPI
    app->add_option("--rw,--access-mode", export_opts.rw_type,
                    "How to drive the hardware model, DPI or MEM_DIRECT(verilator only), default is DPI")
        ->transform(CLI::CheckedTransformer(
            std::map<std::string, picker::SignalAccessType>{{"dpi", picker::SignalAccessType::DPI},
                                                            {"mem_direct", picker::SignalAccessType::MEM_DIRECT}},
            CLI::ignore_case));

    // Set DUT RTL Language, Optional, default is python
    std::vector<std::string> select_languages = {"python", "cpp", "java", "scala", "golang", "lua"};
    app->add_option("--lang,--language", export_opts.language,
                    "Build target project with assigned language, default python")
        ->default_val("python")
        ->check(CLI::IsMember(select_languages));

    // Set DUT RTL Source Dir, Optional, default is
    // ${picker_install_path}/../picker/template
    app->add_option("--sdir,--source_dir", export_opts.source_dir,
                    "Template Files Dir, default is ${picker_install_path}/../picker/template")
        ->default_val(picker::get_template_path());

    // Set DUT RTL Source Module Name, Optional, default is the last module in
    app->add_option("--sname,--source_module_name", export_opts.source_module_name_list,
                    "Pick the module in DUT .v file, default is the last module in the -f marked file")
        ->delimiter(',');

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
    app->add_option("--internal", export_opts.internal,
                    "Exported internal signal config file, default is empty, means no internal pin");

    app->add_flag("--checkpoints", export_opts.checkpoints, "Enable checkpoints, save/restore , default is OFF");
    app->add_flag("--vpi", export_opts.vpi, "Enable VPI, for flexible internal signal access default is OFF");

    // Simulate frequency while using VCS simulator, Optional, default is 100MHz
    app->add_option("-F,--frequency", export_opts.frequency,
                    "Set the frequency of the **only VCS** DUT, default is 100MHz, use Hz, KHz, MHz, GHz as unit")
        ->default_val("100MHz");

    // Dump wave file name, Optional, default is empty means don't dump wave
    app->add_option("-w,--wave_file_name", export_opts.wave_file_name, "Wave file name, empty means don't dump wave");

    // Enable coverage, Optional, default is OFF
    app->add_flag("-c,--coverage", export_opts.coverage, "Enable coverage, default is not selected as OFF");

    // Enable copy xspcomm lib to DUT location
    app->add_option("--cp_lib, --copy_xspcomm_lib", export_opts.cp_lib,
                    "Copy xspcomm lib to generated DUT dir, default is true")
        ->default_val(true);

    // User defined simulator compile args, passthrough. Eg: '-v -x-assign=fast
    // -Wall --trace' || '-C vcs -cc -f filelist.f'
    app->add_option(
        "-V,--vflag", export_opts.vflag,
        "User defined simulator compile args, passthrough. \nEg: '-v -x-assign=fast -Wall --trace' || '-C vcs -cc -f filelist.f'");

    // User defined gcc/clang compile command, passthrough. Eg:'-O3 -std=c++17
    // -I./include'
    app->add_option("-C,--cflag", export_opts.cflag,
                    "User defined gcc/clang compile command, passthrough. Eg:'-O3 -std=c++17 -I./include'");

    // app->add_option(
    //     "-r,--rename", export_opts.rename,
    //     "User defined gcc/clang compile command, passthrough. Eg:'-O3
    //     -std=c++17 -I./include'");

    app->add_flag("--verbose", export_opts.verbose, "Verbose mode");
    app->add_flag("-e,--example", export_opts.example, "Build example project, default is OFF");
    app->add_option("--autobuild", export_opts.autobuild, "Auto build the generated project, default is true")
        ->default_val(true);

    return 0;
}

int set_options_pack_message(CLI::App &top_app)
{
    auto app = top_app.add_subcommand("pack", "Pack UVM transaction as a UVM agent and Python class");

    // Set DUT RTL Source File, Required
    app->add_flag("-e,--example", pack_opts.example, "Generate example project based on transaction, default is OFF")
        ->default_val(false);

    app->add_flag("-c,--force", pack_opts.force, "Force delete folder when the code has already generated by picker")
        ->default_val(false);

    app->add_flag("-d,--generate-dut", pack_opts.generate_dut, "Generate DUT abstraction class with pin-level interface (using xspcomm)")
        ->default_val(false);

    app->add_option("--from-rtl", pack_opts.from_rtl_file, "RTL source file (.v/.sv) to auto-generate transaction from module ports");

    app->add_option("file", pack_opts.files, "Sv source file, contain the transaction define");

    app->add_option("-r,--rename", pack_opts.rename, "Rename transaction name in picker generate code");
    
    app->add_option("-f,--filelist", pack_opts.filelist, "File list containing transaction files");
    
    app->add_option("-n,--name", pack_opts.name, "Name for the generated package (default: auto-generated from files)");

    return 0;
}

int set_options_main(CLI::App &app)
{
    app.add_flag("-v, --version", main_opts.version, "Print version");
    app.add_flag("--show_default_template_path", main_opts.show_default_template_path, "Print default template path");
    app.add_flag("--show_xcom_lib_location_cpp", main_opts.show_xcom_lib_location_cpp,
                 "Print xspcomm lib and include location");
    app.add_flag("--show_xcom_lib_location_java", main_opts.show_xcom_lib_location_java,
                 "Print xspcomm-java.jar location");
    app.add_flag("--show_xcom_lib_location_scala", main_opts.show_xcom_lib_location_scala,
                 "Print xspcomm-scala.jar location");
    app.add_flag("--show_xcom_lib_location_python", main_opts.show_xcom_lib_location_python,
                 "Print python module xspcomm location");
    app.add_flag("--show_xcom_lib_location_golang", main_opts.show_xcom_lib_location_golang,
                 "Print golang module xspcomm location");
    app.add_flag("--show_xcom_lib_location_lua", main_opts.show_xcom_lib_location_lua,
                 "Print lua module xspcomm location");
    app.add_flag("--check", main_opts.check, "check install location and supported languages");
    return 0;
}

int show_xcom_lib_location()
{
    std::string err_message;

#define SHOW_CPP_LOCATION(flag)                                                                                        \
    if (main_opts.flag) {                                                                                              \
        auto lib_location = picker::get_xcomm_lib("lib", err_message);                                                 \
        if (lib_location.empty()) {                                                                                    \
            PK_ERROR("%s", err_message.c_str());                                                                       \
            exit(1);                                                                                                   \
        }                                                                                                              \
        auto include_location = picker::get_xcomm_lib("include", err_message);                                         \
        if (include_location.empty()) {                                                                                \
            PK_ERROR("%s", err_message.c_str());                                                                       \
            exit(1);                                                                                                   \
        }                                                                                                              \
        PK_MESSAGE("Lib:     %s", lib_location.c_str());                                                               \
        PK_MESSAGE("Include: %s", include_location.c_str());                                                           \
        exit(0);                                                                                                       \
    }

#define SHOW_SINGLE_LOCATION(flag, path)                                                                               \
    if (main_opts.flag) {                                                                                              \
        auto location = picker::get_xcomm_lib(path, err_message);                                                      \
        if (location.empty()) {                                                                                        \
            PK_ERROR("%s", err_message.c_str());                                                                       \
            exit(1);                                                                                                   \
        }                                                                                                              \
        PK_MESSAGE("%s", location.c_str());                                                                            \
        exit(0);                                                                                                       \
    }

    SHOW_CPP_LOCATION(show_xcom_lib_location_cpp);
    SHOW_SINGLE_LOCATION(show_xcom_lib_location_java, "java/xspcomm-java.jar");
    SHOW_SINGLE_LOCATION(show_xcom_lib_location_scala, "scala/xspcomm-scala.jar");
    SHOW_SINGLE_LOCATION(show_xcom_lib_location_python, "python");
    SHOW_SINGLE_LOCATION(show_xcom_lib_location_golang, "golang");
    SHOW_SINGLE_LOCATION(show_xcom_lib_location_lua, "lua/luaxspcomm.so");

    return 0;
}

int check_picker_support()
{
    std::string err_message;

    // check if the xspcomm lib is available
    if (main_opts.check) {
        PK_MESSAGE("[OK ] Version: %s-%s-%s%s", PROJECT_VERSION, GIT_BRANCH, GIT_HASH, GIT_DIRTY);
        PK_MESSAGE("[OK ] Exec path: %s", picker::get_executable_path().c_str());
        auto temp_path = picker::get_template_path();
        if (temp_path.empty()) {
            PK_MESSAGE("[Err] Can't find default template path");
        } else {
            PK_MESSAGE("[OK ] Template path: %s", temp_path.c_str());
        }
        for (auto &kv : lang_lib_map) {
            auto lib_location = picker::get_xcomm_lib(kv.second, err_message);
            if (lib_location.empty()) {
                PK_ERROR("[Err] Support %6s (find: '%s' fail)", display_names[kv.first].c_str(), kv.second.c_str());
            } else {
                PK_MESSAGE("[OK ] Support %6s (find: '%s' success)", display_names[kv.first].c_str(),
                           lib_location.c_str());
            }
        }
        exit(0);
    }

    // check if the language is supported
    if (!export_opts.language.empty()) {
        auto it = lang_lib_map.find(export_opts.language);
        if (it == lang_lib_map.end()) {
            PK_ERROR("[Err] Unknown language '%s'", export_opts.language.c_str());
            exit(1);
        }
        auto lib_location = picker::get_xcomm_lib(it->second, err_message);
        if (lib_location.empty()) {
            PK_ERROR("[Err] Support %6s (find: '%s' fail)", display_names[it->first].c_str(), it->second.c_str());
            exit(1);
        }
    }

    // check if the rw_type is supported
    switch (export_opts.rw_type) {
    case picker::SignalAccessType::DPI:
        if (export_opts.internal != "") { PK_MESSAGE("It's recommended to use MEM_DIRECT for internal signal access"); }
        break;
    case picker::SignalAccessType::MEM_DIRECT:
        std::unordered_set<std::string> supported_simulators = {"verilator", "gsim"};
        if (supported_simulators.count(export_opts.sim) == 0) {
            PK_ERROR("[Err] MEM_DIRECT not support simulator '%s'", export_opts.sim.c_str());
        }
        break;
    }

    return 0;
}


int main(int argc, char **argv)
{
    check_debug();
    if(picker::appimage::is_running_as_appimage()) {
        PK_MESSAGE("Running as AppImage");
        picker::appimage::initialize();
    } else {
        PK_MESSAGE("Running as normal executable");
    }
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

    // handle main options here
    // if need version, print version
    if (main_opts.version) {
        PK_MESSAGE("version: %s-%s-%s%s", PROJECT_VERSION, GIT_BRANCH, GIT_HASH, GIT_DIRTY);
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
        if (temp_path.empty()) { PK_FATAL("Can't find default template path"); }
        PK_MESSAGE("%s", temp_path.c_str());
        exit(0);
    }

    show_xcom_lib_location();
    check_picker_support();

    // check if app parsed export subcommand
    if (app.get_subcommands().size() == 0) {
        PK_MESSAGE("%s", app.help().c_str());
        PK_MESSAGE("No subcommand specified: need export or pack");
        exit(0);
    }

    // subcommand export
    if (app.get_subcommand_ptr("export")->parsed()) {

        // render mem_direct depends on the verilator codegen, so we need to render it between verilator and dut_base
        if(export_opts.source_dir.ends_with("/mem_direct")){
            PK_MESSAGE("Rendering mem_direct");         
            // picker is invoked by codegen Makefile, so the export_opts.file is NOT sv but the Verilator CPP (for now).
            std::string source_file = export_opts.file[0];
            std::vector<std::string> declarations;
            std::vector<picker::cpp_variableInfo> vars;

            auto readVarDeclarations = picker::parser::GetReadVarDeclarations(export_opts.sim);
            auto processDeclarations = picker::parser::GetProcessDeclarations(export_opts.sim);
            declarations = readVarDeclarations(source_file);
            vars = processDeclarations(declarations);

            picker::codegen::render_md_addr_generator(vars, export_opts);
            picker::parser::outputYAML(vars, export_opts.target_dir + "/vars.yaml"); 
            exit(0);
        }

        std::vector<picker::sv_module_define> sv_module_result; // sv signal pins
        std::vector<picker::sv_signal_define> internal_signal_result; // configuration signal pins

        nlohmann::json signal_tree_json;
        auto input_parser = picker::parser::GetInputParser(export_opts.sim);
        input_parser(export_opts, sv_module_result);
        picker::parser::internal(export_opts, internal_signal_result);

        auto sv_pin_result =
            picker::codegen::lib(export_opts, sv_module_result, internal_signal_result, signal_tree_json);

        std::map<std::string, std::function<void(picker::export_opts &, std::vector<picker::sv_signal_define>,
                                                 std::vector<picker::sv_signal_define>, nlohmann::json &)>>
            func_map = {
                {"cpp", picker::codegen::cpp},     {"python", picker::codegen::python}, {"java", picker::codegen::java},
                {"scala", picker::codegen::scala}, {"golang", picker::codegen::golang}, {"lua", picker::codegen::lua},
            };
        func_map[export_opts.language](export_opts, sv_pin_result, internal_signal_result, signal_tree_json);

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
        
        std::vector<picker::uvm_transaction_define> transactions;
        std::vector<std::string> filenames;

        if (!pack_opts.from_rtl_file.empty()) {
            // RTL Mode: auto-generate transaction from module ports
            PK_MESSAGE("Mode: RTL (auto-generate transaction from module ports)");

            std::string module_name;
            transactions.push_back(picker::parser::parse_rtl_file(pack_opts.from_rtl_file, module_name, pack_opts.name));
            filenames.push_back(module_name);

        } else {
            // Transaction Mode: parse SV transaction classes
            std::vector<std::string> all_files = pack_opts.files;
            picker::parser::collect_verilog_from_filelists(pack_opts.filelist, all_files);

            if (all_files.empty()) {
                PK_FATAL("No transaction files specified. Use 'file' argument or -f/--filelist option.");
            }

            PK_MESSAGE("Mode: %s Transaction%s", all_files.size() == 1 ? "Single" : "Multi",
                       all_files.size() > 1 ? ("s (" + std::to_string(all_files.size()) + " files)").c_str() : "");

            for (size_t i = 0; i < all_files.size(); i++) {
                auto transaction = picker::parser::parse_sv(all_files[i], "");

                // Apply rename if specified
                std::string filename = (i < pack_opts.rename.size()) ? pack_opts.rename[i]
                                      : std::filesystem::path(all_files[i]).stem().string();
                if (i < pack_opts.rename.size()) {
                    transaction.name = pack_opts.rename[i];
                }

                transactions.push_back(transaction);
                filenames.push_back(filename);
            }
        }

        std::string package_name = !pack_opts.name.empty() ? pack_opts.name :
                                  (!filenames.empty() ? filenames[0] : "");

        if (package_name.empty()) {
            PK_FATAL("Cannot determine package name");
        }

        PK_MESSAGE("Package name: %s", package_name.c_str());

        auto package_data = picker::parser::prepare_uvm_package_data(
            transactions,
            filenames,
            package_name,
            pack_opts.generate_dut,
            pack_opts.from_rtl_file
        );

        // Generate UVM package files
        picker::codegen::generate_uvm_package(package_data, pack_opts, package_name);

        PK_MESSAGE("Successfully generated UVM package: %s", package_name.c_str());
        exit(0);
    }

    nlohmann::json sync_opts;
    return 0;
}
