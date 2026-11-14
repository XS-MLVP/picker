#include "parser/native_layout.hpp"
#include "parser/sv.hpp"
#include <cassert>
#include <fstream>

int main() {
    namespace fs = std::filesystem;
    using J = nlohmann::json;
    const auto base = fs::temp_directory_path() / "picker_native_layout_unit";
    fs::create_directories(base);
    const auto source = base / "native.sv";
    std::ofstream(source) << R"(
typedef struct packed { logic [7:0] a; logic [23:0] b; } P;
typedef struct { logic flag; P words [-1:0]; } U;
module Child #(parameter W=8);
    P packet;
    U records [4:3];
    logic [W-1:0] parameterized;
endmodule
module Top;
    Child #(.W(13)) a();
    Child #(.W(19)) b();
endmodule
)";
    picker::export_opts opts{};
    opts.file = {source.string()}; opts.source_module_name_list = {"Top"};
    opts.sim = "verilator"; opts.rw_type = picker::SignalAccessType::MEM_DIRECT;
    std::vector<picker::sv_module_define> modules;
    picker::parser::sv(opts, modules);
    assert(modules.size() == 1);
    const auto &module = modules.front();
    const std::map<std::string, std::string> signals(module.native_layouts.begin(), module.native_layouts.end());
    assert(signals.at("Top.a.packet") == signals.at("Top.b.packet"));
    assert(signals.at("Top.a.records") == signals.at("Top.b.records"));
    assert(signals.at("Top.a.parameterized") != signals.at("Top.b.parameterized"));
    assert(signals.count("Top.a.records[4]") == 0);
    assert(module.native_types.size() < 12);
    J metadata{{"version", 1}, {"signals", J::object()}, {"types", J::object()}};
    for (const auto &[id, type] : module.native_types) metadata["types"][id] = J::parse(type);
    for (const auto &[name, id] : signals) metadata["signals"][name] = id;
    const auto header = base / "VTop___024root.h";
    std::ofstream(base / "VTop_types.h") << R"(
struct VRecord {
    CData/*0:0*/ __PVT__flag;
    VlUnpacked<IData/*31:0*/, 2> __PVT__words;
    bool operator==(const VRecord& rhs) const { return true; }
};
)";
    std::ofstream(header) << R"(
class VTop___024root {
    // DESIGN SPECIFIC STATE
    IData/*31:0*/ Top__DOT__a__DOT__packet;
    IData/*15:0*/ Top__DOT__a__DOT__parameterized;
    VlUnpacked<VRecord, 2> Top__DOT__a__DOT__records;
    // INTERNAL METHODS
};
)";
    std::vector<picker::cpp_variableInfo> vars{
        {"Top__DOT__a__DOT__packet", "IData", 32},
        {"Top__DOT__a__DOT__parameterized", "IData", 16}};
    const auto used = picker::parser::bind_native_layouts(header.string(), metadata, vars);
    assert(vars.size() == 3);
    assert(vars[0].layout == signals.at("Top.a.packet"));
    assert(vars[0].native_groups.size() == 1);
    assert(vars[1].layout.empty()); // Reject a width mismatch, do not attach a wrong layout.
    assert(!vars[1].view_error.empty());
    assert(vars[2].native_groups.size() == 2);
    assert(vars[2].native_groups[1].path == "[].words[]");
    assert(vars[2].native_groups[1].dimensions == std::vector<int>({2, 2}));
    assert(vars[2].native_groups[1].member == "Top__DOT__a__DOT__records[0].__PVT__words[0]");
    assert(!used.contains(signals.at("Top.b.parameterized")));
    auto aliases_metadata = metadata;
    aliases_metadata["signals"]["Top.alias"] = signals.at("Top.a.packet");
    picker::parser::trace_signal_info alias;
    alias.name = "Top.alias"; alias.kind = "direct"; alias.width = 32; alias.root_name = "Top.a.packet";
    std::vector<picker::cpp_variableInfo> aliases;
    picker::parser::bind_native_layouts(header.string(), aliases_metadata, aliases, {alias});
    bool matched_alias = false;
    for (const auto &variable : aliases) {
        if (variable.public_name == "Top.alias") {
            matched_alias = true;
            assert(variable.name == "Top__DOT__a__DOT__packet");
            assert(variable.layout == signals.at("Top.a.packet"));
        }
    }
    assert(matched_alias);
    // The source/header files are tiny private fixtures, not product RTL.
    fs::remove(source); fs::remove(header); fs::remove(base / "VTop_types.h"); fs::remove(base);
}
