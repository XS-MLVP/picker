#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

#include "codegen/lib.hpp"

static void write_text(const std::filesystem::path &p, const std::string &t)
{
    std::ofstream o(p, std::ios::trunc);
    o << t;
}

static std::string read_text(const std::filesystem::path &p)
{
    std::ifstream i(p);
    std::stringstream ss;
    ss << i.rdbuf();
    return ss.str();
}

int main()
{
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() / "picker_test_codegen_render";
    fs::remove_all(base);
    fs::create_directories(base / "src");
    fs::create_directories(base / "src" / "sub");

    write_text(base / "src" / "a.txt", "hello {{name}}\n");
    write_text(base / "src" / "sub" / "b.txt", "{{value}}\n");

    std::string src_dir = (base / "src").string();
    std::string dst_dir = (base / "dst").string();

    nlohmann::json data;
    data["name"] = "picker";
    data["value"] = 42;
    inja::Environment env;

    picker::codegen::recursive_render(src_dir, dst_dir, data, env);

    const auto a_out = read_text(base / "dst" / "a.txt");
    const auto b_out = read_text(base / "dst" / "sub" / "b.txt");
    assert(a_out == "hello picker\n");
    assert(b_out == "42\n");

    fs::remove_all(base);
    return 0;
}
