#include <iostream>
#include <algorithm>
#include <filesystem>
#include "mcv.hpp"

int main(int argc, char **argv)
{
    std::string file;

    cxxopts::Options options(
        "XDut Gen", "XDut Generate. \nConvert DUT(*.v) to C++ DUT libs.\n");

    // 原始源文件
    options.add_options()("f,file", "DUT .v file",
                          cxxopts::value<std::string>(file));

    options.add_options()("s,source_dir", "Template Files",
                          cxxopts::value<std::string>());
    options.add_options()("t,target_dir", "Gen Files in the target dir",
                          cxxopts::value<std::string>());

    options.add_options()(
        "S,source_module_name",
        "Pick the module in DUT .v file, default is the last module",
        cxxopts::value<std::string>()->default_value(""));
    options.add_options()(
        "T,target_module_name",
        "Set the module name and file name of target DUT, default is the same as source",
        cxxopts::value<std::string>()->default_value(""));

    options.add_options()(
        "i,internal",
        "Exported internal signal config file, default is empty, means no internal pin",
        cxxopts::value<std::string>()->default_value(""));

    options.add_options()("w,wave_file_name",
                          "Wave file name, emtpy mean don't dump wave",
                          cxxopts::value<std::string>()->default_value(""));
    options.add_options()(
        "sim", "VCS or verilator as simulator, default is verilator",
        cxxopts::value<std::string>()->default_value("verilator"));
    options.add_options()(
        "V,vflag",
        "User defined simulator compile args, split by comma. Eg: -v -x-assign=fast,-Wall,--trace. Or a file contain params.",
        cxxopts::value<std::string>()->default_value(""));
    options.add_options()("h,help", "Print usage");

    cxxopts::ParseResult opts = options.parse(argc, argv);
    if (opts.count("help") || !opts.count("file") || !opts.count("source_dir")
        || !opts.count("target_dir")) {
        MESSAGE("%s", options.help().c_str());
        exit(0);
    }

    std::vector<mcv::sv_signal_define> sv_pin_result, internal_sginal_result;
    std::string src_module_name = opts["source_module_name"].as<std::string>();
    mcv::parser::sv(opts, sv_pin_result, src_module_name);
    mcv::parser::internal(opts, internal_sginal_result);

    mcv::codegen::render(opts, src_module_name, sv_pin_result,
                         internal_sginal_result);
    return 0;
}
