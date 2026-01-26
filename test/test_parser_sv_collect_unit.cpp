#include <cassert>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include "parser/sv.hpp"

static void write_text(const std::filesystem::path &p, const std::string &t)
{
    std::ofstream o(p, std::ios::trunc);
    o << t;
}

int main()
{
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() / "picker_test_sv_collect";
    fs::remove_all(base);
    fs::create_directories(base / "rtl");
    fs::create_directories(base / "rtl" / "sub");

    write_text(base / "rtl" / "a.sv", "module a; endmodule\n");
    write_text(base / "rtl" / "b.v", "module b; endmodule\n");
    write_text(base / "rtl" / "sub" / "c.sv", "module c; endmodule\n");
    write_text(base / "rtl" / "sub" / "note.txt", "ignore\n");

    const fs::path filelist = base / "files.f";
    write_text(filelist,
               "# comment\n"
               "rtl/a.sv\n"
               "rtl/sub/\n"
               "missing.sv\n");

    std::vector<std::string> out;
    std::vector<std::string> inputs = {filelist.string(), "rtl/b.v"};
    picker::parser::collect_verilog_from_filelists(inputs, out);

    std::set<std::string> outs(out.begin(), out.end());
    assert(outs.count(fs::absolute(base / "rtl" / "a.sv").string()) == 1);
    assert(outs.count(fs::absolute(base / "rtl" / "b.v").string()) == 1);
    assert(outs.count(fs::absolute(base / "rtl" / "sub" / "c.sv").string()) == 1);
    assert(outs.count(fs::absolute(base / "rtl" / "sub" / "note.txt").string()) == 0);

    fs::remove_all(base);
    return 0;
}
