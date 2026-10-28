#include <algorithm>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include "parser/verilator_trace.hpp"

namespace fs = std::filesystem;

// Test generated forms, not a second parser implementation. In particular an
// array and a scalar projection intentionally share the same source and slot.
struct format {
    bool modern;
    std::string push(const std::string &name, bool array = false) const {
        auto kind = array ? "ARRAY_UNPACKED" : "SCOPE_MODULE";
        return modern ? "VL_TRACE_PUSH_PREFIX(tracep," + name + "," + kind + ",0,1);\n"
                      : "tracep->pushPrefix(" + name + "," + kind + ");\n";
    }
    std::string pop() const {
        return modern ? "VL_TRACE_POP_PREFIX(tracep);\n" : "tracep->popPrefix();\n";
    }
    std::string decl(const std::string &slot, const std::string &name, bool array = false) const {
        if (modern) return "VL_TRACE_DECL_BUS" + std::string(array ? "_ARRAY" : "")
            + "(tracep," + slot + ",0," + name + ",-1,NONE,VAR,LOGIC,"
            + (array ? "(i+0)," : "") + "15,0);\n";
        return "tracep->declBus(" + slot + ",0," + name
            + ",-1,NONE,VAR,LOGIC," + (array ? "true,(i+0)," : "false,-1,") + "15,0);\n";
    }
};

static void write(const fs::path &file, const std::string &text) {
    std::ofstream stream(file); stream << text; assert(stream.good());
}

static std::vector<picker::cpp_variableInfo> vars = {
    {"Top__DOT__mem", "SData", 16, 2, 0},
    {"Top__DOT__native", "SData", 16, 0, 4},
    {"Top__DOT__scalar", "SData", 16, 0, 6}};

static void expect_error(const fs::path &file, const std::string &code, const std::string &message) {
    write(file, code);
    bool rejected = false;
    try {
        picker::parser::verilator::processTraceFiles({file.string()}, vars);
    } catch (const std::exception &e) {
        rejected = std::string(e.what()).find(message) != std::string::npos;
        if (!rejected) std::cerr << "Expected: " << message << "; got: " << e.what() << '\n';
    }
    if (!rejected) std::cerr << "Not rejected as expected: " << code << '\n';
    assert(rejected);
}

int main() {
    auto dir = fs::temp_directory_path() / ("picker-trace-structure-"
        + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(dir);
    for (bool modern : {false, true}) {
        format f{modern};
        auto a = dir / "a.cpp", b = dir / "b.cpp", c = dir / "c.cpp";
        write(a, "void VTop__trace_init_top(Root* vlSelf, Trace* tracep) {\n"
            "VTop__trace_init_sub__open(vlSelf,tracep);\n"
            "VTop__trace_init_sub__middle(vlSelf,tracep);\n"
            "VTop__trace_init_sub__close(vlSelf,tracep);\n}\n");
        auto array = [&](const std::string &name) {
            if (modern) return "VTop__trace_init_dtype____0(vlSelf,tracep,\""
                + name + "\",0,c+100,NONE);\n";
            return f.push("\"" + name + "\"", true) + "for (int i = 0; i < 2; ++i) {\n"
                + f.decl("c+100+i*1", "\"\"", true) + "}\n" + f.pop();
        };
        write(b, "void VTop__trace_init_sub__open(Root* vlSelf, Trace* tracep) {\n"
            + f.push("\"Top\"") + "}\n"
            "void VTop__trace_init_sub__middle(Root* vlSelf, Trace* tracep) {\n"
            "// Ignore a fake tracep->popPrefix(); and brace }\n"
            "VL_DEBUG_IF(VL_DBG_MSGF(\"brace } and fake __trace_init_bad()\\n\"););\n"
            + array("renamed") + array("array_alias")
            + f.decl("c+100", "\"mem\"") // same name as storage, but a scalar alias
            + f.decl("c+9", "\"scalar\"") + f.decl("c+209", "\"shifted\"") + "}\n");
        std::string dtype;
        if (modern) dtype = "void VTop__trace_init_dtype____0(Root* vlSelf, Trace* tracep, const char* name, int fidx, int c, Direction direction) {\n"
            + f.push("name", true) + "for (int i = 0; i < 2; ++i) {\n"
            + f.decl("c+0+i*1", "\"\"", true) + "}\n" + f.pop() + "}\n";
        write(c, "void VTop__trace_init_sub__close(Root* vlSelf, Trace* tracep) {\n"
            + f.pop() + "}\n" + dtype
            + "void VTop__trace_full_a(Root* vlSelf, Buffer* bufp) {\n"
            "uint32_t* oldp = bufp->oldp(vlSymsp->__Vm_baseCode + 0xc8U);\n"
            "bufp->fullSData(oldp+9, vlSelfRef.Top__DOT__native,16);\n}\n"
            "void VTop__trace_full_z(Root* vlSelf, Buffer* bufp) {\n"
            "bufp->fullSData(oldp+9, vlSelfRef.Top__DOT__scalar,16);\n"
            "VTop__trace_full_dtype____0(vlSelf,bufp,100U,vlSelfRef.Top__DOT__mem);\n}\n"
            "void VTop__trace_full_dtype____0(Root* vlSelf, Buffer* bufp, int offset, const Array& __VdtypeVar) {\n"
            "uint32_t* oldp = bufp->oldp(vlSymsp->__Vm_baseCode+offset);\n"
            "bufp->fullSData(oldp+0,(__VdtypeVar[0]),16);\n"
            "bufp->fullSData(oldp+1,(__VdtypeVar[1]),16);\n}\n");
        std::vector<std::string> files = {a.string(), b.string(), c.string()};
        for (int order = 0; order < 2; ++order) {
            auto result = picker::parser::verilator::processTraceFiles(files, vars);
            std::map<std::string, picker::parser::trace_signal_info> signals;
            for (const auto &s : result.signals) signals[s.name] = s;
            assert(signals.size() == 5);
            assert(signals.at("Top.renamed").kind == "direct");
            assert(signals.at("Top.array_alias").kind == "direct");
            assert(signals.at("Top.renamed").root_name == "Top.mem");
            assert(signals.at("Top.renamed").deps[0].array_size == 2);
            assert(signals.at("Top.mem").kind == "projection");
            assert(signals.at("Top.mem").source_expr == "Top.mem[0]");
            assert(signals.at("Top.scalar").source_expr == "Top.scalar");
            assert(signals.at("Top.shifted").source_expr == "Top.native");
            std::reverse(files.begin(), files.end());
        }

        auto bad = dir / "bad.cpp";
        expect_error(bad, f.pop(), "underflow");
        expect_error(bad, f.push("\"Top\""), "unbalanced");
        expect_error(bad, f.decl("c+2147483648", "\"bad\""), "out of range");
        expect_error(bad, f.decl("c+other", "\"bad\""), "unsupported");
        expect_error(bad, f.push("\"mem\"", true)
            + "for (int i = 1; i < 2; ++i) {\n" + f.decl("c+1+i*1", "\"\"", true)
            + "}\n" + f.pop(), "unsupported trace declaration loop");
        expect_error(bad, "void VTop__trace_init_top() { VTop__trace_init_sub__missing(vlSelf,tracep); }",
                     "missing trace initialization helper");
        expect_error(bad, "void VTop__trace_init_top() { VTop__trace_init_top(vlSelf,tracep); }", "cycle");
        expect_error(bad, "VTop__trace_full_dtype____missing(vlSelf,bufp,0U,vlSelfRef.Top__DOT__mem);",
                     "missing trace dtype helper");
        expect_error(bad, "VTop__trace_full_dtype____0(vlSelf,bufp,0U);", "invalid trace dtype call");
        expect_error(bad, "void VTop__trace_full_dtype____0() {}\n"
            "VTop__trace_full_dtype____0(vlSelf,bufp,0U,vlSelfRef.Top__DOT__mem);",
                     "empty trace dtype source helper");
        expect_error(bad, "void VTop__trace_full_dtype____0() {\n"
            "bufp->fullSData(oldp+1,__VdtypeVar[0],16); }\n"
            "VTop__trace_full_dtype____0(vlSelf,bufp,2147483647U,vlSelfRef.Top__DOT__mem);",
                     "overflow");
    }
    fs::remove_all(dir);
}
