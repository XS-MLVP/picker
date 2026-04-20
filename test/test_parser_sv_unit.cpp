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

    void test_filelist_macro_defines_are_shared_across_sources()
    {
        namespace fs = std::filesystem;
        const fs::path base = fs::temp_directory_path() / "picker_test_sv_filelist_macros";
        fs::remove_all(base);
        fs::create_directories(base);

        write_text(base / "defs.v",
                   "`define CAL_BITWIDTH(WIDTH) ((WIDTH) + 1)\n");
        write_text(base / "macro_top.v",
                   "module MacroTop #(parameter int DEPTH = 7) (\n"
                   "    input logic [`CAL_BITWIDTH(DEPTH)-1:0] data\n"
                   ");\n"
                   "endmodule\n");
        write_text(base / "filelist.txt",
                   "defs.v\n"
                   "macro_top.v\n");

        picker::export_opts opts {};
        opts.sim = "verilator";
        opts.filelists = {(base / "filelist.txt").string()};
        opts.source_module_name_list = {"MacroTop"};

        std::vector<picker::sv_module_define> modules;
        picker::parser::sv(opts, modules);

        assert(modules.size() == 1);
        const auto &pin_data = find_pin(modules.front(), "data");
        assert(pin_data.logic_pin_type == "input");
        assert(pin_data.logic_pin_hb == 7);
        assert(pin_data.logic_pin_lb == 0);

        fs::remove_all(base);
    }

    void test_filelist_order_wins_when_explicit_file_is_duplicate()
    {
        namespace fs = std::filesystem;
        const fs::path base = fs::temp_directory_path() / "picker_test_sv_filelist_order";
        fs::remove_all(base);
        fs::create_directories(base);

        write_text(base / "defs.v",
                   "`define W 4\n");
        write_text(base / "ordered_top.v",
                   "module OrderedTop(input logic [`W-1:0] data);\n"
                   "endmodule\n");
        write_text(base / "filelist.txt",
                   "defs.v\n"
                   "ordered_top.v\n");

        picker::export_opts opts {};
        opts.sim = "verilator";
        opts.file = {(base / "ordered_top.v").string()};
        opts.filelists = {(base / "filelist.txt").string()};
        opts.source_module_name_list = {"OrderedTop"};

        std::vector<picker::sv_module_define> modules;
        picker::parser::sv(opts, modules);

        assert(modules.size() == 1);
        const auto &pin_data = find_pin(modules.front(), "data");
        assert(pin_data.logic_pin_hb == 3);
        assert(pin_data.logic_pin_lb == 0);

        fs::remove_all(base);
    }

    void test_missing_child_modules_do_not_block_top_port_extraction()
    {
        namespace fs = std::filesystem;
        const fs::path base = fs::temp_directory_path() / "picker_test_sv_missing_child";
        fs::remove_all(base);
        fs::create_directories(base);

        write_text(base / "missing_child_top.sv",
                   "module MissingChildTop(input logic clk, output logic ready);\n"
                   "    MissingChild u_missing(.clk(clk));\n"
                   "    assign ready = clk;\n"
                   "endmodule\n");

        picker::export_opts opts {};
        opts.sim = "verilator";
        opts.file = {(base / "missing_child_top.sv").string()};
        opts.source_module_name_list = {"MissingChildTop"};

        std::vector<picker::sv_module_define> modules;
        picker::parser::sv(opts, modules);

        assert(modules.size() == 1);
        const auto &pin_clk = find_pin(modules.front(), "clk");
        assert(pin_clk.logic_pin_type == "input");
        assert(pin_clk.logic_pin_hb == -1);
        assert(pin_clk.logic_pin_lb == 0);

        const auto &pin_ready = find_pin(modules.front(), "ready");
        assert(pin_ready.logic_pin_type == "output");
        assert(pin_ready.logic_pin_hb == -1);
        assert(pin_ready.logic_pin_lb == 0);

        fs::remove_all(base);
    }

} // namespace

int main()
{
    test_filelist_include_define_and_math();
    test_default_module_is_last_declared();
    test_filelist_macro_defines_are_shared_across_sources();
    test_filelist_order_wins_when_explicit_file_is_duplicate();
    test_missing_child_modules_do_not_block_top_port_extraction();
    return 0;
}
