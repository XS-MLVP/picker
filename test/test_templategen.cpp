
#include <mcv/mcv.hpp>

int main(int argc, char **argv)
{
    auto ctx = std::map<std::string, std::string>{
        {"dut_name", "dut"},
        {"mode_name","test_name"},
        {"body","<this is body>"},
    };
    MESSAGE("render result:\n%s",template_rander("template/python/pybind11.cpp", ctx).c_str());
    return 0;
}
