#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "parser/gsim_root.hpp"

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
    const fs::path base = fs::temp_directory_path() / "picker_test_gsim_root";
    fs::remove_all(base);
    fs::create_directories(base);

    const fs::path hdr = base / "gsim.cpp";
    write_text(hdr,
               "uint32_t _var_start;\n"
               "uint8_t foo; // width = 3, lineno = 1\n"
               "unsigned _BitInt(192) bar; // width = 129, lineno = 2\n"
               "uint32_t arr[4][2]; // width = 21, lineno = 3\n"
               "uint32_t _var_end;\n");

    auto decls = picker::parser::gsim::readVarDeclarations(hdr.string());
    auto vars = picker::parser::gsim::processDeclarations(decls);

    const auto *foo = find_var(vars, "foo");
    const auto *bar = find_var(vars, "bar");
    const auto *arr = find_var(vars, "arr");
    assert(foo && bar && arr);

    assert(foo->type == "uint8_t");
    assert(foo->width == 3);
    assert(foo->array_size == 1);

    assert(bar->type == "unsigned _BitInt(192)");
    assert(bar->width == 129);
    assert(bar->array_size == 1);

    assert(arr->type == "uint32_t");
    assert(arr->width == 21);
    assert(arr->array_size == 8);

    fs::remove_all(base);
    return 0;
}
