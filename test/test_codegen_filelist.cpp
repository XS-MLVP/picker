#include <cassert>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

namespace picker { namespace codegen {
    void gen_filelist(const std::vector<std::string> &source_file, const std::vector<std::string> &ifilelists,
                      std::string &ofilelist, std::vector<std::string> &incdirs);
    void append_incdirs_to_vflag(const std::string &simulator, const std::vector<std::string> &incdirs,
                                 std::string &vflag);
}} // namespace picker::codegen

static void write_text(const std::filesystem::path &p, const std::string &t)
{
    std::ofstream o(p, std::ios::trunc);
    o << t;
}

static bool contains_line(const std::string &haystack, const std::string &needle)
{
    return haystack.find(needle) != std::string::npos;
}

int main()
{
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() / "picker_test_filelist";
    fs::remove_all(base);
    fs::create_directories(base / "inc1");
    fs::create_directories(base / "inc2");
    fs::create_directories(base / "inc3");
    fs::create_directories(base / "inc4");
    fs::create_directories(base / "sub");
    fs::create_directories(base / "sub2");

    write_text(base / "rtl1.sv", "module m; endmodule\n");
    write_text(base / "sub" / "rtl2.v", "module n; endmodule\n");
    write_text(base / "sub2" / "ignore.txt", "noop\n");
    write_text(base / "sub2" / "rtl3.sv", "module p; endmodule\n");

    const fs::path filelist = base / "filelist.f";
    write_text(filelist,
               "# comment\n"
               "+incdir+inc1+inc2\n"
               "-I inc3\n"
               "-Iinc4\n"
               "rtl1.sv\n"
               "sub/\n");

    std::string ofilelist;
    std::vector<std::string> incdirs;
    picker::codegen::gen_filelist({}, {filelist.string()}, ofilelist, incdirs);

    std::set<std::string> incset(incdirs.begin(), incdirs.end());
    assert(incset.size() == 4);
    assert(incset.count((base / "inc1").string()) == 1);
    assert(incset.count((base / "inc2").string()) == 1);
    assert(incset.count((base / "inc3").string()) == 1);
    assert(incset.count((base / "inc4").string()) == 1);

    assert(contains_line(ofilelist, (base / "rtl1.sv").string()));
    assert(contains_line(ofilelist, (base / "sub" / "rtl2.v").string()));
    assert(!contains_line(ofilelist, "incdir"));

    std::string vflag;
    picker::codegen::append_incdirs_to_vflag("verilator", incdirs, vflag);
    assert(contains_line(vflag, "-I" + (base / "inc1").string()));

    std::string vflag_vcs;
    picker::codegen::append_incdirs_to_vflag("vcs", incdirs, vflag_vcs);
    assert(contains_line(vflag_vcs, "+incdir+" + (base / "inc1").string()));

    // second scenario: duplicates, comments, and source_file skip
    const fs::path filelist2 = base / "filelist2.f";
    write_text(filelist2,
               "+incdir+inc1+inc1\n"
               "  # full line comment\n"
               "rtl1.sv # trailing comment\n"
               "sub2/\n");

    std::string ofilelist2;
    std::vector<std::string> incdirs2;
    picker::codegen::gen_filelist({(base / "rtl1.sv").string()}, {filelist2.string()}, ofilelist2, incdirs2);
    // rtl1.sv should be skipped because it is in source_file
    assert(!contains_line(ofilelist2, (base / "rtl1.sv").string()));
    // sub2/ should include rtl3.sv but not ignore.txt
    assert(contains_line(ofilelist2, (base / "sub2" / "rtl3.sv").string()));
    assert(!contains_line(ofilelist2, (base / "sub2" / "ignore.txt").string()));
    // duplicate incdir should collapse
    std::set<std::string> incset2(incdirs2.begin(), incdirs2.end());
    assert(incset2.size() == 1);
    assert(incset2.count((base / "inc1").string()) == 1);

    fs::remove_all(base);
    return 0;
}
