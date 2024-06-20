#include <iostream>
#include <algorithm>
#include <filesystem>
#include "picker.hpp"

picker::exports_opts exports_opts;

picker::pack_opts pack_opts;

int exports(CLI::App &top_app)
{
    auto app = top_app.add_subcommand(
        "exports",
        "Export RTL Projects Sources as Software libraries such as C++/Python");

    // Set DUT RTL Source File, Required
    app->add_option("-f,--file", exports_opts.file,
                    "DUT .v/.sv source file, contain the top module")
        ->required();

    // Set DUT RTL Extra Source File List, Optional
    app->add_option(
        "--fs,--filelist", exports_opts.filelist,
        "DUT .v/.sv source files, contain the top module, split by comma.\n"
        "Or use '*.txt' file  with one RTL file path per line to specify the file list");

    // Set DUT RTL Simulator, Optional, default is verilator
    app->add_option("--sim", exports_opts.sim,
                    "vcs or verilator as simulator, default is verilator")
        ->default_val("verilator");

    // Set DUT RTL Language, Optional, default is python
    app->add_option(
           "--lang,--language", exports_opts.language,
           "Build example project, default is cpp, choose cpp or python")
        ->default_val("python");

    // Set DUT RTL Source Dir, Optional, default is
    // ${picker_install_path}/../picker/template
    app->add_option(
           "--sdir,--source_dir", exports_opts.source_dir,
           "Template Files Dir, default is ${picker_install_path}/../picker/template")
        ->default_val(picker::get_template_path());

    // Set DUT RTL Target Dir, Optional, default is ./picker_out
    app->add_option(
           "--tdir,--target_dir", exports_opts.target_dir,
           "Codegen render files to target dir, default is ./picker_out")
        ->default_val("./picker_out");

    // Set DUT RTL Source Module Name, Optional, default is the last module in
    app->add_option(
        "--sname,--source_module_name", exports_opts.source_module_name,
        "Pick the module in DUT .v file, default is the last module in the -f marked file");

    // Set Generated Software library module name, Optional, default is the same
    app->add_option(
        "--tname,--target_module_name", exports_opts.target_module_name,
        "Set the module name and file name of target DUT, default is the same as source. For example, -T top, will generate UTtop.cpp and UTtop.hpp with UTtop class");

    // Set DUT RTL Internal Signal Config File, Optional, default is empty
    app->add_option(
        "--internal", exports_opts.internal,
        "Exported internal signal config file, default is empty, means no internal pin");

    // Simulate frequncy while using VCS simulator, Optional, default is 100MHz
    app->add_option(
           "-F,--frequency", exports_opts.frequency,
           "Set the frequency of the **only VCS** DUT, default is 100MHz, use Hz, KHz, MHz, GHz as unit")
        ->default_val("100MHz");

    // Dump wave file name, Optional, default is empty means don't dump wave
    app->add_option("-w,--wave_file_name", exports_opts.wave_file_name,
                    "Wave file name, emtpy mean don't dump wave");

    // Enable coverage, Optional, default is OFF
    app->add_flag("-c,--coverage", exports_opts.coverage,
                  "Enable coverage, default is not selected as OFF");

    // User defined simulator compile args, passthrough. Eg: '-v -x-assign=fast
    // -Wall --trace' || '-C vcs -cc -f filelist.f'
    app->add_option(
        "-V,--vflag", exports_opts.vflag,
        "User defined simulator compile args, passthrough. Eg: '-v -x-assign=fast -Wall --trace' || '-C vcs -cc -f filelist.f'");

    // User defined gcc/clang compile command, passthrough. Eg:'-O3 -std=c++17
    // -I./include'
    app->add_option(
        "-C,--cflag", exports_opts.cflag,
        "User defined gcc/clang compile command, passthrough. Eg:'-O3 -std=c++17 -I./include'");


    
    //app->add_option(
    //    "-r,--rename", exports_opts.rename,
    //    "User defined gcc/clang compile command, passthrough. Eg:'-O3 -std=c++17 -I./include'");    

    app->add_flag("--verbose", exports_opts.verbose, "Verbose mode");
    app->add_flag("--version", exports_opts.version, "Print version");
    app->add_flag("-e,--example", exports_opts.example,
                  "Build example project, default is OFF");
    app->add_flag("--autobuild", exports_opts.autobuild,
                  "Auto build the generated project, default is true")
        ->default_val(true);

    return 0;
}

int pack(CLI::App &top_app)
{
    auto app = top_app.add_subcommand(
         "pack",
         "pack uvm transaction as a uvm agent and python class");

    // Set DUT RTL Source File, Required
    app->add_flag(
        "-e,--example", pack_opts.example,
        "generate example project based on transaction, default is OFF");
    
    app->add_flag(
        "-c,--force", pack_opts.force,
        "force delete folder when the code has already generated by picker ");
    
    app->add_option(
        "-f,--files",pack_opts.files,
        "sv source file, contain the transaction define");

    app->add_option(
        "-r,--rename",pack_opts.rename,
        "rename transaction name in picker generate code");

    return 0;
}

int main(int argc, char **argv)
{
    CLI::App app{"XDut Generate. \n"
                 "Convert DUT(*.v/*.sv) to C++ DUT libs.\n"};
    exports(app);
    pack(app);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        // MESSAGE("%s", app.help().c_str());
        return app.exit(e);
    }

    // if need version, print version
    if (exports_opts.version) {
        MESSAGE("version: %s-%s-%s%s", PROJECT_VERSION, GIT_BRANCH, GIT_HASH,
                GIT_DIRTY);
        exit(0);
    }

    //if need help or no file specified without version option, print help
    if (exports_opts.file.empty() && pack_opts.files.empty() /*pack_opts.file.empty()*/) {
        exit(0);
    }

    // check if app parsed export subcommand
    if (app.get_subcommands().size() == 0) {
        MESSAGE("Please specify subcommand: exports or pack");
        exit(0);
    }

    if (app.get_subcommand_ptr("exports")->parsed()) {
        std::vector<picker::sv_signal_define> sv_pin_result,
            internal_sginal_result;
        picker::parser::sv(exports_opts, sv_pin_result);
        picker::parser::internal(exports_opts, internal_sginal_result);

        picker::codegen::lib(exports_opts, sv_pin_result,
                             internal_sginal_result);
        picker::codegen::cpp(exports_opts, sv_pin_result,
                             internal_sginal_result);
        picker::codegen::python(exports_opts, sv_pin_result,
                                internal_sginal_result);
        // build the result with make
        if (exports_opts.autobuild) {
            const std::string cmd =
                "cd " + exports_opts.target_dir + " && make";
            if (system(cmd.c_str()) != 0) {
                MESSAGE("Build failed");
                exit(1);
            }
        }
    } else if (app.get_subcommand_ptr("pack")->parsed()) {
        // todo
        picker::uvm_transaction_define uvm_transaction;
        std::string filepath,filename,macro_define;
        //std::queue<std::string> rnames = pack_opts.rname;
        int i = 0;
        if (pack_opts.files.size() != 0) {
            for(auto& path :pack_opts.files){
                picker::parser::uvm(pack_opts,path, filename, uvm_transaction);
                if(i< pack_opts.rename.size()){
                    filename = pack_opts.rename[i];
                    uvm_transaction.name = pack_opts.rename[i];
                    i++;
                }
                picker::codegen::gen_uvm_param(pack_opts,uvm_transaction, filename);
                

            }
            exit(0);
        }
        
    }

    nlohmann::json sync_opts;

    return 0;
}
