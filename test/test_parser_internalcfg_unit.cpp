#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "parser/internalcfg.hpp"

static void write_text(const std::filesystem::path &p, const std::string &t)
{
    std::ofstream o(p, std::ios::trunc);
    o << t;
}

int main()
{
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() / "picker_test_internalcfg";
    fs::remove_all(base);
    fs::create_directories(base);

    const fs::path yaml = base / "pins.yaml";
    write_text(yaml,
               "top:\n"
               "  sub:\n"
               "    - \"input [3:0] a\"\n"
               "    - \"output b\"\n"
               "  sub2:\n"
               "    key: \"inout [7:0] c\"\n");

    auto pins = picker::parser::internal(yaml.string());
    assert(pins.size() == 3);

    bool found_a = false, found_b = false, found_c = false;
    for (const auto &p : pins) {
        if (p.logic_pin == "top.sub.a") {
            found_a = true;
            assert(p.logic_pin_type == "input");
            assert(p.logic_pin_hb == 3);
            assert(p.logic_pin_lb == 0);
        } else if (p.logic_pin == "top.sub.b") {
            found_b = true;
            assert(p.logic_pin_type == "output");
            assert(p.logic_pin_hb == -1);
        } else if (p.logic_pin == "top.sub2.c") {
            found_c = true;
            assert(p.logic_pin_type == "inout");
            assert(p.logic_pin_hb == 7);
            assert(p.logic_pin_lb == 0);
        }
    }
    assert(found_a && found_b && found_c);

    fs::remove_all(base);
    return 0;
}
