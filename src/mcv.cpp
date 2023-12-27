#include <iostream>
#include <algorithm>
#include <filesystem>
#include "mcv.hpp"

int main(int argc, char **argv)
{
    cxxopts::Options options(
        "XDut Gen", "XDut Generate. \nConvert DUT(*.v/*.sv) to C++ DUT libs. Notice that [file] option allow only one file.\n");

    // Basic Options

    options.add_options()("file",
                          "DUT .v/.sv source file, contain the top module",
                          cxxopts::value<std::string>());
    options.add_options()(
        "f,filelist",
        "DUT .v/.sv source files, contain the top module, split by comma.\n"
        "Or use '*.txt' file  with one RTL file path per line to specify the file list",
        cxxopts::value<std::string>()->default_value(""));
    options.add_options()(
        "sim", "vcs or verilator as simulator, default is verilator",
        cxxopts::value<std::string>()->default_value("verilator"));
    options.add_options()(
        "l,language",
        "Build example project, default is cpp, choose cpp or python",
        cxxopts::value<std::string>()->default_value("cpp"));

    options.add_options()(
        "s,source_dir",
        "Template Files Dir, default is ${mcv_install_path}/../mcv/template",
        cxxopts::value<std::string>()->default_value(mcv::get_template_path()));
    options.add_options()(
        "t,target_dir", "Render files to target dir, default is ./mcv_out",
        cxxopts::value<std::string>()->default_value("./mcv_out"));

    // Extra Basic Options
    options.add_options()(
        "S,source_module_name",
        "Pick the module in DUT .v file, default is the last module in the -f marked file",
        cxxopts::value<std::string>()->default_value(""));
    options.add_options()(
        "T,target_module_name",
        "Set the module name and file name of target DUT, default is the same as source. For example, -T top, will generate UTtop.cpp and UTtop.hpp with UTtop class",
        cxxopts::value<std::string>()->default_value(""));

    // Extra Internal Signal Options
    options.add_options()(
        "internal",
        "Exported internal signal config file, default is empty, means no internal pin",
        cxxopts::value<std::string>()->default_value(""));

    // Advanced Basic Options
    options.add_options()(
        "F,frequency",
        "Set the frequency of the **only VCS** DUT, default is 100MHz, use Hz, KHz, MHz, GHz as unit",
        cxxopts::value<std::string>()->default_value("100MHz"));
    options.add_options()("w,wave_file_name",
                          "Wave file name, emtpy mean don't dump wave",
                          cxxopts::value<std::string>()->default_value(""));
    options.add_options()("c,coverage",
                            "Enable coverage, default is not selected as OFF");

    // Expert Options
    options.add_options()(
        "V,vflag",
        "User defined simulator compile args, passthrough. Eg: '-v -x-assign=fast -Wall --trace' || '-C vcs -cc -f filelist.f'",
        cxxopts::value<std::string>()->default_value(""));
    options.add_options()(
        "C,cflag",
        "User defined gcc/clang compile command, passthrough. Eg:'-O3 -std=c++17 -I./include'",
        cxxopts::value<std::string>()->default_value(""));

    // Help Options
    options.add_options()("v,verbose", "Verbose mode");
    options.add_options()("e,example", "Build example project, default is OFF");
    options.add_options()("h,help", "Print usage");

    // Parse positional arguments
    options.parse_positional({"file"});
    options.positional_help("[file]");

    cxxopts::ParseResult opts = options.parse(argc, argv);
    if (opts.count("help") || !opts.count("file")) {
        MESSAGE("%s", options.help().c_str());
        exit(0);
    }

    nlohmann::json sync_opts;
    std::vector<mcv::sv_signal_define> sv_pin_result, internal_sginal_result;
    mcv::parser::sv(opts, sv_pin_result, sync_opts);
    mcv::parser::internal(opts, internal_sginal_result, sync_opts);

    // mcv::codegen::render(opts, sync_opts, sv_pin_result,
    //                      internal_sginal_result);
    mcv::codegen::lib(opts, sync_opts, sv_pin_result, internal_sginal_result);
    mcv::codegen::cpp(opts, sync_opts, sv_pin_result, internal_sginal_result);
    mcv::codegen::python(opts, sync_opts, sv_pin_result,
                         internal_sginal_result);
    return 0;
}
