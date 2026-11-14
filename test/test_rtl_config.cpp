#include <cassert>
#include <string>
#include <vector>

#include "rtl_config.hpp"

bool picker::is_debug = false;
char *picker::lib_random_hash = const_cast<char *>("H");

static void test_parse_params()
{
    auto params = picker::rtl::parse_params({"A=8", " B = $clog2(256) ", "MASK={1,2}"}, "--rtl-param");
    assert(params.size() == 3);
    assert(params[0].first == "A" && params[0].second == "8");
    // Values stay textual: the RTL frontend evaluates expressions.
    assert(params[1].first == "B" && params[1].second == "$clog2(256)");
    assert(params[2].first == "MASK" && params[2].second == "{1,2}");

    auto empty = picker::rtl::parse_params({}, "--rtl-param");
    assert(empty.empty());
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
    test_parse_params();
    test_identifier_rules();
    return 0;
}
