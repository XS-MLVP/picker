#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "filelist.hpp"

static void write_text(const std::filesystem::path &p, const std::string &t)
{
    std::ofstream o(p, std::ios::trunc);
    o << t;
}

using picker::filelist::token;
using picker::filelist::token_kind;

static const token &find_arg(const std::vector<token> &tokens, const std::string &name)
{
    for (const auto &tok : tokens) {
        if (tok.name == name) { return tok; }
    }
    assert(false && "arg not found");
    std::abort();
}

int main()
{
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() / "picker_test_filelist_unit";
    fs::remove_all(base);
    fs::create_directories(base / "rtl");
    fs::create_directories(base / "lib");
    fs::create_directories(base / "inc");

    write_text(base / "rtl" / "a.v", "module a; endmodule\n");
    write_text(base / "rtl" / "b.sv", "module b; endmodule\n");
    write_text(base / "lib" / "cell.v", "module cell; endmodule\n");

    write_text(base / "common.f", "rtl/b.sv\n"
                                  "+define+FROM_NESTED\n");
    write_text(base / "top.f", "# comment line\n"
                               "+define+WIDTH=32\n"
                               "+incdir+inc\n"
                               "-y ./lib\n"
                               "-v lib/cell.v\n"
                               "-timescale=1ns/1ps\n"
                               "-l compile.log\n"
                               "-full64\n"
                               "-CFLAGS -O2\n"
                               "+notimingcheck\n"
                               "-f ./common.f\n"
                               "rtl/a.v # trailing comment\n");

    std::vector<token> tokens;
    picker::filelist::expand({(base / "top.f").string()}, tokens);

    // only include flags are singled out, every other option is a passthrough arg
    assert(find_arg(tokens, "+incdir").kind == token_kind::incdir);
    assert(find_arg(tokens, "+define").kind == token_kind::arg);
    assert(find_arg(tokens, "-timescale").kind == token_kind::arg);
    assert(find_arg(tokens, "-full64").kind == token_kind::arg);
    assert(find_arg(tokens, "+notimingcheck").kind == token_kind::arg);

    // '-y'/'-v' values are resolved against the filelist directory
    const auto &libdir = find_arg(tokens, "-y");
    assert(libdir.kind == token_kind::arg);
    assert(libdir.argv.size() == 2);
    assert(libdir.argv[1] == (base / "lib").string());
    const auto &libfile = find_arg(tokens, "-v");
    assert(libfile.argv[1] == (base / "lib" / "cell.v").string());

    // options with a value keep it attached to the same token
    const auto &cflags = find_arg(tokens, "-CFLAGS");
    assert(cflags.values == 1);
    assert(cflags.argv.size() == 2 && cflags.argv[1] == "-O2");

    // an attached '=' value is not counted as a separate token
    assert(find_arg(tokens, "-timescale").values == 0);

    // an unknown option owns the rest of its line, picker never reads its value
    const auto &logfile = find_arg(tokens, "-l");
    assert(logfile.values == 1);
    assert(logfile.argv.size() == 2 && logfile.argv[1] == "compile.log");

    // nested filelists are inlined, entries keep their own base dir
    bool nested_define = false, nested_source = false;
    for (const auto &tok : tokens) {
        if (tok.kind == token_kind::arg && tok.argv.front() == "+define+FROM_NESTED") { nested_define = true; }
        if (tok.kind == token_kind::path && tok.argv.front() == "rtl/b.sv") { nested_source = true; }
    }
    assert(nested_define);
    assert(nested_source);

    // filelist order is preserved: +define+WIDTH comes before rtl/a.v
    size_t define_index = tokens.size(), source_index = 0;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].argv.front() == "+define+WIDTH=32") { define_index = i; }
        if (tokens[i].argv.front() == "rtl/a.v") { source_index = i; }
    }
    assert(define_index < source_index);

    // comma separated CLI entries still work
    std::vector<token> cli_tokens;
    picker::filelist::expand({(base / "rtl" / "a.v").string() + "," + (base / "rtl" / "b.sv").string()}, cli_tokens);
    assert(cli_tokens.size() == 2);
    assert(cli_tokens[0].kind == token_kind::path);
    assert(cli_tokens[1].kind == token_kind::path);

    fs::remove_all(base);
    return 0;
}
