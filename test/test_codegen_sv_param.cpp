#include <cassert>
#include <cstdlib>
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
                    assert(contains_line(initialization, "XView"));
                    assert(contains_line(initialization, "FromPort"));
                    assert(!contains_line(initialization, "_SignalArray"));
                    assert(!contains_line(declarations, "PickerSignalArray"));
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

static void test_recursive_struct_bindings()
{
    using J = nlohmann::json;
    const J bits{{"kind", "bits"}, {"packed", true}, {"width", 8}};
    const J structure{{"kind", "struct"}, {"packed", false}, {"width", 0},
                      {"fields", J::array({{{"name", "tag"}, {"type", bits}}})}};
    const J layout{{"kind", "array"}, {"packed", false}, {"width", 0},
                   {"left", -1}, {"right", 1}, {"element", structure}};
    picker::sv_signal_define pin{"packets", "input", -1, 0};
    pin.type_layout = std::make_shared<const std::string>(layout.dump());
    for (auto access : {picker::SignalAccessType::DPI, picker::SignalAccessType::MEM_DIRECT}) {
        J data{{"__TOP_MODULE_NAME__", "Top"}}, tree;
        const auto leaves = picker::codegen::gen_sv_param(data, {{"Dut", 1, {pin}}}, {}, tree, "", "verilator", access);
        assert(leaves.size() == 3);
        assert(leaves[0].type_layout == leaves[2].type_layout);
        assert(leaves[0].array_name == leaves[2].array_name);
        assert(picker::codegen::signal_name(leaves[0]) == "packets[-1].tag");
        assert(picker::codegen::signal_name(leaves[2]) == "packets[1].tag");
        assert(leaves[0].binding_native == "packets[0].__PVT__tag");
        assert(leaves[0].binding_strides == std::vector<std::string>{"packets[0]"});
        const auto source = data["__LOGIC_PIN_DECLARATION__"].get<std::string>();
        assert(contains_line(source, "type(Dut.packets) packets;"));
        assert(!contains_line(source, "_picker_leaf_"));
        assert(tree["packets"]["Layout"] == layout);
        const auto functions = data["__DPI_FUNCTION_IMPLEMENT__"].get<std::string>();
        if (access == picker::SignalAccessType::DPI) {
            assert(contains_line(functions, "packets[-1 + ((index / 1) % 3)].tag"));
            assert(data["__DPI_FUNCTION_EXPORT__"].get<std::string>().find("_picker_leaf_") == std::string::npos);
        } else assert(functions.empty());
        for (const auto &language : {"python", "cpp", "java", "scala", "golang", "lua"}) {
            std::string declarations, initialization;
            picker::codegen::render_array_ports(leaves, language, declarations, initialization);
            assert(contains_line(initialization, "FromPort"));
            assert(!contains_line(initialization, "_SignalArray"));
            if (std::string(language) == "golang")
                assert(contains_line(initialization, "DeleteXView(view)"));
        }
    }
}

static void test_struct_member_array_setter()
{
    using J = nlohmann::json;
    for (const int width : {4, 65}) {
        const J element{{"kind", "bits"}, {"packed", true}, {"width", width}};
        const J array{{"kind", "array"}, {"packed", false}, {"width", 0},
                      {"left", 2}, {"right", 1}, {"element", element}};
        const J layout{{"kind", "struct"}, {"packed", false}, {"width", 0},
                       {"fields", J::array({{{"name", "words"}, {"type", array}}})}};
        picker::sv_signal_define pin{"packet", "input", -1, 0};
        pin.type_layout = std::make_shared<const std::string>(layout.dump());
        J data{{"__TOP_MODULE_NAME__", "Top"}}, tree;
        const auto leaves = picker::codegen::gen_sv_param(data, {{"Dut", 1, {pin}}}, {}, tree, "", "verilator",
                                                         picker::SignalAccessType::DPI);
        assert(leaves.size() == 2 && leaves[0].array_name == leaves[1].array_name);
        const auto functions = data["__DPI_FUNCTION_IMPLEMENT__"].get<std::string>();
        assert(contains_line(functions, "packet.words[2 - ((index / 1) % 2)]=type(packet.words[2 - ((index / 1) % 2)])'(value);"));
        assert(!contains_line(functions, "_picker_array_value"));
        assert(!contains_line(functions, "_picker_index_"));
    }
}

static void test_packed_struct_dpi_fragments()
{
    using J = nlohmann::json;
    const J scalar{{"kind", "bits"}, {"packed", true}, {"width", 0}};
    const J byte{{"kind", "bits"}, {"packed", true}, {"width", 8}};
    const J layout{{"kind", "struct"}, {"packed", true}, {"width", 9},
                   {"fields", J::array({
                       {{"name", "tag"}, {"offset", 1}, {"type", byte}},
                       {{"name", "valid"}, {"offset", 0}, {"type", scalar}},
                   })}};
    picker::sv_signal_define input{"packet", "input", 8, 0};
    input.type_layout = std::make_shared<const std::string>(layout.dump());
    auto output = input;
    output.logic_pin = "result";
    output.logic_pin_type = "output";
    for (const auto access : {picker::SignalAccessType::DPI, picker::SignalAccessType::MEM_DIRECT}) {
        J data{{"__TOP_MODULE_NAME__", "Top"}}, tree;
        const auto pins = picker::codegen::gen_sv_param(
            data, {{"Dut", 1, {input, output}}}, {}, tree, "", "verilator", access);
        assert(pins.size() == 2);
        assert(pins[0].logic_pin == "packet" && pins[0].aggregate_name == "packet");
        assert(pins[0].signal_path == "packet" && pins[0].array_name.empty());
        assert(pins[1].logic_pin == "result" && pins[1].aggregate_name == "result");
        assert(pins[0].type_layout == input.type_layout && pins[1].type_layout == input.type_layout);
        assert(tree["packet"]["Layout"] == layout && tree["result"]["Layout"] == layout);
        assert(data["__WIRE_PIN_DECLARATION__"] == "  logic [8:0] packet;\n  logic [8:0] result;\n");
        if (access == picker::SignalAccessType::DPI) {
            assert(data["__DPI_FUNCTION_EXPORT__"] ==
                "  export \"DPI-C\" function get_packetxxH;\n"
                "  export \"DPI-C\" function set_packetxxH;\n"
                "  export \"DPI-C\" function get_resultxxH;\n");
            assert(data["__DPI_FUNCTION_IMPLEMENT__"] ==
                "  function void get_packetxxH;\n"
                "    output logic [8:0] value;\n"
                "    value=packet;\n"
                "  endfunction\n\n"
                "  function void set_packetxxH;\n"
                "    input logic [8:0] value;\n"
                "    packet=type(packet)'(value);\n"
                "  endfunction\n\n"
                "  function void get_resultxxH;\n"
                "    output logic [8:0] value;\n"
                "    value=result;\n"
                "  endfunction\n\n");
        } else {
            assert(data["__DPI_FUNCTION_EXPORT__"] == "");
            assert(data["__DPI_FUNCTION_IMPLEMENT__"] == "");
        }
    }
}

static void test_waveform_and_finish_fragments()
{
    // Exercise both values independently of the caller's environment, then
    // restore it so the rest of the test process sees its original settings.
    struct DumpvarsEnvironment {
        bool present = std::getenv("DUMPVARS_OPTION") != nullptr;
        std::string previous = present ? std::getenv("DUMPVARS_OPTION") : "";
        ~DumpvarsEnvironment() {
            if (present) setenv("DUMPVARS_OPTION", previous.c_str(), 1);
            else unsetenv("DUMPVARS_OPTION");
        }
    } restore;

    auto render = [](const std::string &simulator, const std::string &wave,
                     picker::SignalAccessType access) {
        nlohmann::json data{{"__TOP_MODULE_NAME__", "Top"}}, tree;
        picker::codegen::gen_sv_param(data, {{"Dut", 1, {{"clk", "input", -1, 0}}}},
                                     {}, tree, wave, simulator, access);
        assert(data["__WAVE_FILE_NAME__"] == wave);
        assert(data["__LIB_DPI_FUNC_NAME_HASH__"] == "H");
        assert(data["__EXTEND_SV__"] ==
            "  export \"DPI-C\" function finish_H;\n"
            "  function void finish_H;\n"
            "    $finish;\n"
            "  endfunction\n");
        return data;
    };
    for (const auto option : {"", "all"}) {
        setenv("DUMPVARS_OPTION", option, 1);
        const std::string dump_options = *option ? ", \"all\"" : "";
        for (const auto access : {picker::SignalAccessType::DPI, picker::SignalAccessType::MEM_DIRECT}) {
            for (const std::string wave : {"", "trace.vcd", "trace.fst"}) {
                const auto data = render("verilator", wave, access);
                const auto expected = wave.empty() ? "" :
                    "  initial begin\n"
                    "    $dumpfile(\"" + wave + "\");\n"
                    "    $dumpvars(0, Top_top" + dump_options + ");\n"
                    "  end";
                assert(data["__SV_DUMP_WAVE__"] == expected);
                assert(data["__TRACE__"] == (wave.empty() ? "OFF" : wave.substr(wave.find_last_of('.') + 1)));
                assert(data["__DUMP_VAR_OPTIONS__"] == dump_options);
            }
            for (const std::string simulator : {"vcs", "uvs"}) {
                const std::string format = simulator == "vcs" ? "fsdb" : "usdb";
                const auto guard = "  bit picker_" + format + "_configured = 0;\n";
                const auto disabled = render(simulator, "", access);
                const auto disabled_source = disabled["__SV_DUMP_WAVE__"].get<std::string>();
                assert(disabled["__TRACE__"] == "OFF");
                assert(disabled_source.starts_with(guard));
                assert(!contains_line(disabled_source, "initial begin"));
                const auto wave = "trace." + format;
                const auto enabled = render(simulator, wave, access);
                const auto enabled_source = enabled["__SV_DUMP_WAVE__"].get<std::string>();
                assert(enabled["__TRACE__"] == format);
                assert(enabled["__DUMP_VAR_OPTIONS__"] == dump_options);
                const auto initial =
                    "  initial begin\n"
                    "    if (!picker_" + format + "_configured) begin\n"
                    "    $" + format + "Dumpfile(\"" + wave + "\");\n"
                    "    $" + format + "Dumpvars(0, Top_top" + dump_options + ");\n"
                    "    end\n"
                    "  end\n\n";
                assert(enabled_source == guard + initial + disabled_source.substr(guard.size()));
                for (const auto action : {"set_waveform", "finish_waveform", "flush_waveform", "waveform_enable"}) {
                    assert(contains_line(disabled_source,
                        "export \"DPI-C\" function " + simulator + "_" + format + "_" + action + "_H;"));
                }
                assert(contains_line(disabled_source, "$" + format + "Dumpvars(0, Top_top" + dump_options + ");"));
                assert(contains_line(disabled_source, "$" + format + "DumpFinish;"));
                assert(contains_line(disabled_source, "$" + format + "Dumpflush;"));
                assert(contains_line(disabled_source, "if (enable) $" + format + "Dumpon;"));
                assert(contains_line(disabled_source, "else $" + format + "Dumpoff;"));
                if (simulator == "vcs") {
                    for (const auto action : {"start", "stop", "reset", "dump"})
                        assert(contains_line(disabled_source, std::string("export \"DPI-C\" task vcs_coverage_") + action + "_H;"));
                    assert(contains_line(disabled_source, "$cg_coverage_control(1);"));
                    assert(contains_line(disabled_source, "$cg_coverage_control(0);"));
                    assert(contains_line(disabled_source, "$coverage_reset();"));
                    assert(contains_line(disabled_source, "$coverage_dump(name);"));
                } else {
                    assert(!contains_line(disabled_source, "vcs_coverage"));
                }
            }
        }
    }
}

int main()
{
    test_unpacked_array_ports();
    test_dpi_array_accessor_count_is_constant();
    test_array_names_do_not_collide_with_ports();
    test_recursive_struct_bindings();
    test_struct_member_array_setter();
    test_packed_struct_dpi_fragments();
    test_waveform_and_finish_fragments();
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
