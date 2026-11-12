#include <cassert>
#include <string>
#include <vector>

#include "codegen/sv.hpp"
#include "codegen/lib.hpp"

bool picker::is_debug = false;
char *picker::lib_random_hash = const_cast<char *>("H");

static bool contains_line(const std::string &haystack, const std::string &needle)
{
    return haystack.find(needle) != std::string::npos;
}

static void test_unpacked_array_ports()
{
    picker::sv_module_define mod{"ArrayTop", 1, {
        {"data", "input", 4, 0, {{4, 2}}},
        {"matrix", "output", 4, 0, {{-1, 0}, {3, 2}}},
        {"flag", "input", -1, 0, {{7, 7}}, "bit signed"},
        {"clk", "input", -1, 0},
    }};
    for (const auto access : {picker::SignalAccessType::DPI, picker::SignalAccessType::MEM_DIRECT}) {
        // Exercise the same lowering for single and multiple module instances.
        for (const int instances : {1, 2}) {
            mod.module_nums = instances;
            nlohmann::json data{{"__TOP_MODULE_NAME__", "Top"}}, tree;
            const auto external = picker::codegen::gen_sv_param(data, {mod}, {}, tree, "", "verilator", access);
            assert(external.size() == 9 * instances);
            const std::vector<std::string> names{
                "data[4]", "data[3]", "data[2]", "matrix[-1][3]", "matrix[-1][2]",
                "matrix[0][3]", "matrix[0][2]", "flag[7]", "clk",
            };
            const auto logic = data["__LOGIC_PIN_DECLARATION__"].get<std::string>();
            const auto wire = data["__WIRE_PIN_DECLARATION__"].get<std::string>();
            const auto inner = data["__INNER_MODULES__"].get<std::string>();
            const auto dpi = data["__DPI_FUNCTION_IMPLEMENT__"].get<std::string>();
            const auto exports = data["__DPI_FUNCTION_EXPORT__"].get<std::string>();
            for (int instance = 0; instance < instances; ++instance) {
                const auto prefix = instances == 1 ? "" : "ArrayTop_" + std::to_string(instance) + "_";
                for (size_t i = 0; i < names.size(); ++i) {
                    const auto &pin = external[instance * names.size() + i];
                    assert(picker::codegen::signal_name(pin) == prefix + names[i]);
                    assert(pin.array_name.empty() == (i == 8));
                    assert(pin.logic_pin_hb == (i < 7 ? 4 : -1));
                    assert(pin.logic_pin_lb == 0);
                }
                assert(contains_line(logic, "logic [4:0] " + prefix + "data [4:2];"));
                assert(contains_line(wire, "wire [4:0] " + prefix + "matrix [-1:0] [3:2];"));
                assert(contains_line(inner, ".data(" + prefix + "data)"));
                assert(!contains_line(logic, "assign "));
                assert(!contains_line(logic, "_picker_array_"));
                if (access == picker::SignalAccessType::DPI) {
                    assert(contains_line(dpi, "value=" + prefix + "matrix[-1 + ((index / 2) % 2)][3 - ((index / 1) % 2)];"));
                    assert(contains_line(dpi, prefix + "data[4 - ((index / 1) % 3)]=value;"));
                    assert(!contains_line(exports, "set_" + prefix + "matrixxx"));
                    assert(contains_line(exports, "set_" + prefix + "flagxx"));
                    assert(!contains_line(exports, "_picker_array_"));
                } else {
                    assert(dpi.empty());
                    assert(exports.empty());
                }
                assert(contains_line(logic, "bit signed  " + prefix + "flag [7:7];"));
                assert(contains_line(wire, "bit signed  " + prefix + "flag [7:7];"));
            }
            if (instances == 1) {
                assert(tree["data"]["High"] == 4);
                assert(tree["data"]["Unpacked"] == nlohmann::json({{4, 2}}));
                assert(tree["matrix"]["Pin"] == "output");
                assert(tree["matrix"]["Unpacked"] == nlohmann::json({{-1, 0}, {3, 2}}));
                assert(!contains_line(data["__SIGNAL_TREE__"].get<std::string>(), "_picker_array_"));
                for (const auto &language : {"python", "cpp", "java", "scala", "golang", "lua"}) {
                    std::string declarations, initialization;
                    assert(picker::codegen::render_array_ports(external, language, declarations, initialization));
                    assert(contains_line(initialization, external[3].logic_pin));
                    assert(contains_line(initialization, "matrix") || contains_line(initialization, "Matrix"));
                    assert(!contains_line(initialization, "matrix_n1"));
                    const auto binding = picker::codegen::render_array_binding(external[4], language, external[4].logic_pin);
                    assert(contains_line(binding, "BindArrayElement("));
                    assert(contains_line(binding, "CSelf(), \"matrix\", 1)"));
                }
            }
        }
    }
}

static void test_dpi_array_accessor_count_is_constant()
{
    for (const int size : {1, 64, 4096}) {
        nlohmann::json data{{"__TOP_MODULE_NAME__", "Top"}}, tree;
        picker::sv_module_define mod{"dut", 1, {{"memory", "input", 64, 0, {{size - 1, 0}}}}};
        const auto external = picker::codegen::gen_sv_param(data, {mod}, {}, tree, "", "verilator", picker::SignalAccessType::DPI);
        assert(external.size() == size);
        assert(data["__DPI_FUNCTION_EXPORT__"].get<std::string>() ==
               "  export \"DPI-C\" function get_memoryxxH;\n  export \"DPI-C\" function set_memoryxxH;\n");
        assert(data["__DPI_FUNCTION_IMPLEMENT__"].get<std::string>().size() < 600);
    }
}

static void test_array_names_do_not_collide_with_ports()
{
    nlohmann::json data{{"__TOP_MODULE_NAME__", "Top"}}, tree;
    picker::sv_module_define mod{"dut", 1, {
        {"data", "input", -1, 0, {{0, 1}}},
        {"data_0", "input", -1, 0},
        {"_picker_array_0", "input", -1, 0},
    }};
    const auto external = picker::codegen::gen_sv_param(data, {mod}, {}, tree, "", "verilator", picker::SignalAccessType::DPI);
    assert(external[0].logic_pin == "_picker_array_1");
    assert(external[1].logic_pin == "_picker_array_2");
    assert(picker::codegen::signal_name(external[0]) == "data[0]");
    assert(external[2].logic_pin == "data_0");
    assert(external[3].logic_pin == "_picker_array_0");
}

int main()
{
    test_unpacked_array_ports();
    test_dpi_array_accessor_count_is_constant();
    test_array_names_do_not_collide_with_ports();
    nlohmann::json data;
    data["__TOP_MODULE_NAME__"] = "Top";

    picker::sv_module_define mod;
    mod.module_name = "dut";
    mod.module_nums = 1;
    mod.pins.push_back({"a", "input", 3, 0});
    mod.pins.push_back({"b", "output", -1, 0});

    std::vector<picker::sv_module_define> modules{mod};
    std::vector<picker::sv_signal_define> internal{
        {"sig0", "output", 7, 0},
    };

    nlohmann::json signal_tree;
    auto external = picker::codegen::gen_sv_param(
        data,
        modules,
        internal,
        signal_tree,
        "wave.vcd",
        "verilator",
        picker::SignalAccessType::DPI
    );

    assert(external.size() == 2);

    const auto logic_decl = data["__LOGIC_PIN_DECLARATION__"].get<std::string>();
    assert(contains_line(logic_decl, "logic [3:0] a;"));
    assert(contains_line(logic_decl, "b;"));

    const auto dpi_export = data["__DPI_FUNCTION_EXPORT__"].get<std::string>();
    assert(contains_line(dpi_export, "get_a"));
    assert(contains_line(dpi_export, "set_a"));
    assert(contains_line(dpi_export, "get_sig0"));

    const auto sv_wave = data["__SV_DUMP_WAVE__"].get<std::string>();
    assert(contains_line(sv_wave, "wave.vcd"));

    const auto tree = data["__SIGNAL_TREE__"].get<std::string>();
    assert(contains_line(tree, "a"));
    assert(contains_line(tree, "b"));

    // MEM_DIRECT adds public_flat markers
    nlohmann::json data2;
    data2["__TOP_MODULE_NAME__"] = "Top";
    nlohmann::json signal_tree2;
    picker::codegen::gen_sv_param(
        data2,
        modules,
        internal,
        signal_tree2,
        "",
        "verilator",
        picker::SignalAccessType::MEM_DIRECT
    );
    const auto logic_decl2 = data2["__LOGIC_PIN_DECLARATION__"].get<std::string>();
    assert(contains_line(logic_decl2, "public_flat_rw_on"));
    assert(contains_line(logic_decl2, "public_off"));

    return 0;
}
