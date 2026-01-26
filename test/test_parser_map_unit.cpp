#include <cassert>
#include <string>

#include "parser/parser_map.hpp"

int main()
{
    // Supported simulators should not throw
    try {
        auto f1 = picker::parser::GetReadVarDeclarations("verilator");
        auto f2 = picker::parser::GetReadVarDeclarations("gsim");
        (void)f1;
        (void)f2;
    } catch (...) {
        assert(false && "GetReadVarDeclarations threw for supported sim");
    }

    try {
        auto f1 = picker::parser::GetProcessDeclarations("verilator");
        auto f2 = picker::parser::GetProcessDeclarations("gsim");
        (void)f1;
        (void)f2;
    } catch (...) {
        assert(false && "GetProcessDeclarations threw for supported sim");
    }

    try {
        auto f1 = picker::parser::GetInputParser("verilator");
        auto f2 = picker::parser::GetInputParser("vcs");
        auto f3 = picker::parser::GetInputParser("gsim");
        (void)f1;
        (void)f2;
        (void)f3;
    } catch (...) {
        assert(false && "GetInputParser threw for supported sim");
    }

    // Unsupported sim should throw
    bool threw = false;
    try {
        picker::parser::GetReadVarDeclarations("nope");
    } catch (...) {
        threw = true;
    }
    assert(threw);

    threw = false;
    try {
        picker::parser::GetProcessDeclarations("nope");
    } catch (...) {
        threw = true;
    }
    assert(threw);

    threw = false;
    try {
        picker::parser::GetInputParser("nope");
    } catch (...) {
        threw = true;
    }
    assert(threw);

    return 0;
}
