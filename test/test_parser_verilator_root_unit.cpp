#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "parser/verilator_root.hpp"

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

    assert(c->type == "VlWide<3>");
    assert(c->width == 96);

    fs::remove_all(base);
    return 0;
}
