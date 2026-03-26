#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "parser/sv.hpp"

namespace {

    void write_text(const std::filesystem::path &path, const std::string &text)
    {
        std::ofstream stream(path, std::ios::trunc);
        stream << text;
    }

    const picker::sv_module_define &find_module(const std::vector<picker::sv_module_define> &modules,
                                                const std::string &module_name)
    {
        for (const auto &module : modules) {
            if (module.module_name == module_name) { return module; }
        }
        assert(false && "module not found");
        return modules.front();
    }

    const picker::sv_signal_define &find_pin(const picker::sv_module_define &module, const std::string &pin_name)
    {
        for (const auto &pin : module.pins) {
            if (pin.logic_pin == pin_name) { return pin; }
        }
        assert(false && "pin not found");
        return module.pins.front();
    }

    void test_filelist_include_define_and_math()
    {
        namespace fs = std::filesystem;
        const fs::path base = fs::temp_directory_path() / "picker_test_sv_filelist";
        fs::remove_all(base);
        fs::create_directories(base / "list");
        fs::create_directories(base / "rtl" / "inc");

        write_text(base / "rtl" / "inc" / "inner.svh", "`define BASE_W 3\n");
        write_text(base / "rtl" / "inc" / "outer.svh",
                   "`include \"inner.svh\"\n"
                   "`define DATA_W (`BASE_W + 5)\n");
        write_text(base / "rtl" / "feature_top.sv",
                   "`include \"inc/outer.svh\"\n"
                   "module FeatureTop(\n"
                   "    input logic [`DATA_W-1:0] a,\n"
                   "    output logic [$clog2(16)-1:0] b,\n"
                   "    input logic scalar\n"
                   ");\n"
                   "endmodule\n"
                   "module Beta(input logic ready);\n"
                   "endmodule\n");
        write_text(base / "list" / "design.f", "../rtl/feature_top.sv\n");

        picker::export_opts opts {};
        opts.filelists = {(base / "list" / "design.f").string()};
        opts.source_module_name_list = {"FeatureTop", "3", "Beta"};

        std::vector<picker::sv_module_define> modules;
        picker::parser::sv(opts, modules);

        assert(modules.size() == 2);

        const auto &feature_top = find_module(modules, "FeatureTop");
        assert(feature_top.module_nums == 3);
        assert(feature_top.pins.size() == 3);

        const auto &pin_a = find_pin(feature_top, "a");
        assert(pin_a.logic_pin_type == "input");
        assert(pin_a.logic_pin_hb == 7);
        assert(pin_a.logic_pin_lb == 0);

        const auto &pin_b = find_pin(feature_top, "b");
        assert(pin_b.logic_pin_type == "output");
        assert(pin_b.logic_pin_hb == 3);
        assert(pin_b.logic_pin_lb == 0);

        const auto &pin_scalar = find_pin(feature_top, "scalar");
        assert(pin_scalar.logic_pin_type == "input");
        assert(pin_scalar.logic_pin_hb == -1);
        assert(pin_scalar.logic_pin_lb == 0);

        const auto &beta = find_module(modules, "Beta");
        assert(beta.module_nums == 1);
        assert(beta.pins.size() == 1);
        assert(beta.pins.front().logic_pin == "ready");
        assert(beta.pins.front().logic_pin_hb == -1);
        assert(beta.pins.front().logic_pin_lb == 0);

        fs::remove_all(base);
    }

    void test_default_module_is_last_declared()
    {
        namespace fs = std::filesystem;
        const fs::path base = fs::temp_directory_path() / "picker_test_sv_default_top";
        fs::remove_all(base);
        fs::create_directories(base);

        write_text(base / "dual.sv",
                   "module First(input logic a);\n"
                   "endmodule\n"
                   "module Second(output logic [2:0] y);\n"
                   "endmodule\n");

        picker::export_opts opts {};
        opts.file = {(base / "dual.sv").string()};

        std::vector<picker::sv_module_define> modules;
        picker::parser::sv(opts, modules);

        assert(modules.size() == 1);
        assert(modules.front().module_name == "Second");
        assert(opts.target_module_name == "Second");
        assert(opts.target_dir == "Second");

        const auto &pin_y = find_pin(modules.front(), "y");
        assert(pin_y.logic_pin_type == "output");
        assert(pin_y.logic_pin_hb == 2);
        assert(pin_y.logic_pin_lb == 0);

        fs::remove_all(base);
    }

} // namespace

int main()
{
    test_filelist_include_define_and_math();
    test_default_module_is_last_declared();
    return 0;
}
