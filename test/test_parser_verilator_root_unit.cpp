#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "parser/verilator_root.hpp"
#include "verilator_root/common.hpp"

static void write_text(const std::filesystem::path &p, const std::string &t)
{
    std::ofstream o(p, std::ios::trunc);
    o << t;
}

static const picker::cpp_variableInfo *find_var(const std::vector<picker::cpp_variableInfo> &vars,
                                                const std::string &name)
{
    for (const auto &v : vars) {
        if (v.name == name) return &v;
    }
    return nullptr;
}

int main()
{
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() / "picker_test_verilator_root";
    fs::remove_all(base);
    fs::create_directories(base);

    const fs::path hdr = base / "VTop___024root.h";
    write_text(hdr,
               "struct {\n"
               "  CData/*3:0*/ a;\n"
               "  VlUnpacked<IData/*31:0*/, 2> b;\n"
                "  VlWide<3>/*95:0*/ c;\n"
                "  VlUnpacked<VlUnpacked<VlWide<3>/*95:0*/, 3>, 2> matrix;\n"
               "};\n"
               "VTop___024root::VTop___024root() {}\n");

    auto decls = picker::parser::verilator::readVarDeclarations(hdr.string());
    auto vars = picker::parser::verilator::processDeclarations(decls);

    const auto *a = find_var(vars, "a");
    const auto *b = find_var(vars, "b");
    const auto *c = find_var(vars, "c");
    assert(a && b && c);

    assert(a->type == "CData");
    assert(a->width == 4);
    assert(a->array_size == 0);

    assert(b->type == "IData");
    assert(b->width == 32);
    assert(b->array_size == 2);
    assert(b->array_dimensions == std::vector<int>{2});

    assert(c->type == "VlWide<3>");
    assert(c->width == 96);
    const auto *matrix = find_var(vars, "matrix");
    assert(matrix && matrix->type == "VlWide<3>" && matrix->width == 96);
    assert(matrix->array_size == 6);
    assert((matrix->array_dimensions == std::vector<int>{2, 3}));

    const fs::path non_struct_hdr = base / "VFlat___024root.h";
    write_text(non_struct_hdr,
               "class VFlat___024root final {\n"
               "  CData/*0:0*/ first_signal;\n"
               "  alignas(VL_CACHE_LINE_BYTES) QData/*63:0*/ aligned_signal;\n"
               "  VFlat___024root(VFlat__Syms* symsp, const char* v__name);\n"
               "};\n");

    decls = picker::parser::verilator::readVarDeclarations(non_struct_hdr.string());
    vars = picker::parser::verilator::processDeclarations(decls);

    const auto *first_signal   = find_var(vars, "first_signal");
    const auto *aligned_signal = find_var(vars, "aligned_signal");
    if (!first_signal || first_signal->type != "CData" || first_signal->width != 1) {
        return 1;
    }
    if (!aligned_signal || aligned_signal->type != "QData" || aligned_signal->width != 64) {
        return 1;
    }

    // Verilator 5.054 renamed the state section and emits globally-qualified
    // types, including inside VlUnpacked templates.
    const fs::path modern_hdr = base / "VModern___024root.h";
    write_text(modern_hdr,
               "class alignas(VL_CACHE_LINE_BYTES) VModern___024root final {\n"
               "  public:\n"
               "    // TRIVIALLY CONSTRUCTIBLE STATE\n"
               "    ::CData/*0:0*/ clk;\n"
               "    ::QData/*63:0*/ counter;\n"
               "    ::VlWide<4>/*127:0*/ payload;\n"
               "    ::VlUnpacked<::QData/*63:0*/, 2> lanes;\n"
               "    alignas(8) ::VlUnpacked<::VlUnpacked<::SData/*7:0*/, 3>, 2> matrix;\n"
               "    // INTERNAL VARIABLES\n"
               "    VModern__Syms* vlSymsp;\n"
               "    VModern___024root(VModern__Syms* symsp, const char* namep);\n"
               "};\n"
               "VModern___024root::VModern___024root() {}\n");

    decls = picker::parser::verilator::readVarDeclarations(modern_hdr.string());
    vars = picker::parser::verilator::processDeclarations(decls);

    const auto *modern_clk     = find_var(vars, "clk");
    const auto *modern_counter = find_var(vars, "counter");
    const auto *modern_payload = find_var(vars, "payload");
    const auto *modern_lanes   = find_var(vars, "lanes");
    const auto *modern_matrix  = find_var(vars, "matrix");
    if (!modern_clk || modern_clk->type != "CData" || modern_clk->width != 1) return 1;
    if (!modern_counter || modern_counter->type != "QData" || modern_counter->width != 64) return 1;
    if (!modern_payload || modern_payload->type != "VlWide<4>" || modern_payload->width != 128) return 1;
    if (!modern_lanes || modern_lanes->type != "QData" || modern_lanes->width != 64
        || modern_lanes->array_size != 2
        || modern_lanes->array_dimensions != std::vector<int>{2}) return 1;
    if (!modern_matrix || modern_matrix->type != "SData" || modern_matrix->width != 8
        || modern_matrix->array_size != 6
        || modern_matrix->array_dimensions != std::vector<int>{2, 3}) return 1;

    // A header without a state section marker falls back to the type spelling
    // for format selection, so the 5.054 grammar still parses.
    const fs::path flat_modern_hdr = base / "VFlatModern___024root.h";
    write_text(flat_modern_hdr,
               "class VFlatModern___024root final {\n"
               "  ::VlWide<1>/*0:0*/ valid;\n"
               "  alignas(8) ::VlUnpacked<::QData/*63:0*/, 2> lanes;\n"
               "  VFlatModern___024root(VFlatModern__Syms* symsp, const char* namep);\n"
               "};\n"
               "VFlatModern___024root::VFlatModern___024root() {}\n");

    decls = picker::parser::verilator::readVarDeclarations(flat_modern_hdr.string());
    vars = picker::parser::verilator::processDeclarations(decls);

    const auto *flat_valid = find_var(vars, "valid");
    const auto *flat_lanes = find_var(vars, "lanes");
    if (!flat_valid || flat_valid->type != "VlWide<1>" || flat_valid->width != 1) return 1;
    if (!flat_lanes || flat_lanes->type != "QData" || flat_lanes->width != 64
        || flat_lanes->array_size != 2) return 1;

    // A 5.054 header opens with a `TOP LEVEL PORTS` union. Those are port
    // aliases, not addressable storage, and the prelude must not swallow the
    // state section that follows.
    const fs::path prelude_hdr = base / "VPrelude___024root.h";
    write_text(prelude_hdr,
               "class alignas(VL_CACHE_LINE_BYTES) VPrelude___024root final {\n"
               "  public:\n"
               "    // TOP LEVEL PORTS\n"
               "    union {\n"
               "        struct {\n"
               "            VL_IN8(clk,0,0);\n"
               "            VL_OUT16(sum,15,0);\n"
               "        } __VprimaryPorts;\n"
               "    };\n"
               "    // TRIVIALLY CONSTRUCTIBLE STATE\n"
               "    ::CData/*0:0*/ valid;\n"
               "    ::VlUnpacked<::QData/*63:0*/, 2> lanes;\n"
               "    // INTERNAL VARIABLES\n"
               "    VPrelude__Syms* vlSymsp;\n"
               "    VPrelude___024root(VPrelude__Syms* symsp, const char* namep);\n"
               "};\n"
               "VPrelude___024root::VPrelude___024root() {}\n");

    decls = picker::parser::verilator::readVarDeclarations(prelude_hdr.string());
    vars = picker::parser::verilator::processDeclarations(decls);
    if (vars.size() != 2) return 1;
    if (find_var(vars, "clk") || find_var(vars, "sum")) return 1;
    const auto *prelude_valid = find_var(vars, "valid");
    const auto *prelude_lanes = find_var(vars, "lanes");
    if (!prelude_valid || prelude_valid->type != "CData" || prelude_valid->width != 1) return 1;
    if (!prelude_lanes || prelude_lanes->type != "QData" || prelude_lanes->width != 64
        || prelude_lanes->array_size != 2) return 1;

    // Trigger bookkeeping changed spelling inside the older range (5.026 wrote
    // `VlTriggerVec<N>`, 5.048 wrote `VlUnpacked<QData, N>`), but both are
    // handled by the same grammar: `VlTriggerVec` is not addressable and the
    // shared type grammar drops it, while the array form is exported.
    namespace detail = picker::parser::verilator::detail;
    if (!detail::parse_root_type("VlTriggerVec<1>", detail::root_format::v5_026).type.empty()) return 1;
    const auto trigger_array = detail::parse_root_type("VlUnpacked<QData/*63:0*/, 2>",
                                                      detail::root_format::v5_026);
    if (trigger_array.type != "QData" || trigger_array.width != 64
        || trigger_array.array_size != 2) return 1;

    const fs::path old_trigger_hdr = base / "VOldTrigger___024root.h";
    write_text(old_trigger_hdr,
               "class alignas(VL_CACHE_LINE_BYTES) VOldTrigger___024root final : public VerilatedModule {\n"
               "  public:\n"
               "    // DESIGN SPECIFIC STATE\n"
               "    CData/*0:0*/ valid;\n"
               "    VlTriggerVec<1> __VstlTriggered;\n"
               "    // INTERNAL VARIABLES\n"
               "    VOldTrigger__Syms* const vlSymsp;\n"
               "    VOldTrigger___024root(VOldTrigger__Syms* symsp, const char* v__name);\n"
               "};\n");
    decls = picker::parser::verilator::readVarDeclarations(old_trigger_hdr.string());
    vars = picker::parser::verilator::processDeclarations(decls);
    if (!find_var(vars, "valid")) return 1;
    if (find_var(vars, "__VstlTriggered")) return 1;

    const fs::path new_trigger_hdr = base / "VNewTrigger___024root.h";
    write_text(new_trigger_hdr,
               "class alignas(VL_CACHE_LINE_BYTES) VNewTrigger___024root final {\n"
               "  public:\n"
               "    // DESIGN SPECIFIC STATE\n"
               "    CData/*0:0*/ valid;\n"
               "    VlUnpacked<QData/*63:0*/, 1> __VstlTriggered;\n"
               "    // INTERNAL VARIABLES\n"
               "    VNewTrigger__Syms* vlSymsp;\n"
               "    VNewTrigger___024root(VNewTrigger__Syms* symsp, const char* namep);\n"
               "};\n");
    decls = picker::parser::verilator::readVarDeclarations(new_trigger_hdr.string());
    vars = picker::parser::verilator::processDeclarations(decls);
    const auto *new_trigger = find_var(vars, "__VstlTriggered");
    if (!new_trigger || new_trigger->type != "QData" || new_trigger->width != 64
        || new_trigger->array_size != 1) return 1;

    // 5.050 aligned members use the older grammar.
    const auto aligned = detail::parse_root_type("alignas(VL_CACHE_LINE_BYTES) QData/*63:0*/",
                                                detail::root_format::v5_026);
    if (aligned.type != "QData" || aligned.width != 64) return 1;
    // Only 5.054 strips the global qualifier.
    if (!detail::parse_root_type("::CData/*0:0*/", detail::root_format::v5_026).type.empty()) return 1;
    const auto qualified = detail::parse_root_type("::VlUnpacked<::VlUnpacked<::SData/*7:0*/, 3>, 2>",
                                                  detail::root_format::v5_054);
    if (qualified.type != "SData" || qualified.width != 8 || qualified.array_size != 6
        || qualified.array_dimensions != std::vector<int>{2, 3}) return 1;

    fs::remove_all(base);
    return 0;
}
