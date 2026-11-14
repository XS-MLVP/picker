#include <cassert>
#include <string>
#include <unordered_set>
#include <vector>

#include "rtl_config.hpp"

bool picker::is_debug = false;
char *picker::lib_random_hash = const_cast<char *>("H");

static void test_parse_defines()
{
    auto defines = picker::rtl::parse_defines({"W", "DEPTH=8", " SPACED = 3 "}, "--rtl-define");
    assert(defines.size() == 3);
    assert(defines[0].name == "W" && !defines[0].has_value);
    assert(defines[1].name == "DEPTH" && defines[1].has_value && defines[1].value == "8");
    assert(defines[2].name == "SPACED" && defines[2].value == "3");

    auto empty = picker::rtl::parse_defines({}, "--rtl-define");
    assert(empty.empty());
}

static void test_parse_params()
{
    auto params = picker::rtl::parse_params({"A=8", " B = $clog2(256) ", "MASK={1,2}"}, "--rtl-param");
    assert(params.size() == 3);
    assert(params[0].first == "A" && params[0].second == "8");
    // Values stay textual: the RTL frontend evaluates expressions.
    assert(params[1].first == "B" && params[1].second == "$clog2(256)");
    assert(params[2].first == "MASK" && params[2].second == "{1,2}");
}

static void test_argument_rendering()
{
    picker::rtl::macro_define bare{"W", "", false};
    picker::rtl::macro_define valued{"DEPTH", "8", true};
    assert(picker::rtl::slang_define_argument(bare) == "-DW");
    assert(picker::rtl::slang_define_argument(valued) == "-DDEPTH=8");
    // One syntax for every simulator and for filelists.
    assert(picker::rtl::simulator_define_argument(bare) == "+define+W");
    assert(picker::rtl::simulator_define_argument(valued) == "+define+DEPTH=8");
}

static void test_filter_filelist_defines()
{
    const std::unordered_set<std::string> overridden{"W", "DEPTH"};
    std::string replacement;

    // A plusarg whose only macro is overridden disappears entirely.
    replacement.clear();
    assert(!picker::rtl::filter_filelist_define("+define", {"+define+W=4"}, overridden, replacement));

    // A multi-macro plusarg keeps the macros the command line does not set.
    replacement.clear();
    assert(picker::rtl::filter_filelist_define("+define", {"+define+W=4+P=2"}, overridden, replacement));
    assert(replacement == "+define+P=2");

    // Single-macro -D forms, with and without a space.
    replacement.clear();
    assert(!picker::rtl::filter_filelist_define("-DW", {"-DW=4"}, overridden, replacement));
    replacement.clear();
    assert(!picker::rtl::filter_filelist_define("-D", {"-D", "DEPTH=9"}, overridden, replacement));

    // Untouched tokens survive verbatim.
    replacement.clear();
    assert(picker::rtl::filter_filelist_define("+define", {"+define+LOCAL=1"}, overridden, replacement));
    assert(replacement.empty());
    assert(picker::rtl::filter_filelist_define("-D", {"-D", "LOCAL=1"}, overridden, replacement));
    assert(picker::rtl::filter_filelist_define("+incdir", {"+incdir+rtl"}, overridden, replacement));
}

static void test_identifier_rules()
{
    assert(picker::rtl::is_identifier("A"));
    assert(picker::rtl::is_identifier("_WIDTH_2"));
    assert(!picker::rtl::is_identifier(""));
    assert(!picker::rtl::is_identifier("2W"));
    assert(!picker::rtl::is_identifier("A-B"));
    assert(!picker::rtl::is_identifier("A.B"));
}

int main()
{
    test_parse_defines();
    test_parse_params();
    test_argument_rendering();
    test_filter_filelist_defines();
    test_identifier_rules();
    return 0;
}
