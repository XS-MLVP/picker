#include <cassert>
#include <filesystem>
#include <string>

#include "inja.hpp"

static std::string render_dut_template(const std::string &simulator)
{
    namespace fs = std::filesystem;

    inja::Environment env;
    nlohmann::json data;
    data["__TOP_MODULE_NAME__"] = "Adder";
    data["__SIMULATOR__"] = simulator;
    data["__XDATA_INIT__"] = "";
    data["__XDATA_BIND__"] = "";
    data["__XPORT_ADD__"] = "";
    data["__XPORT_CASCADED__"] = "";

    const auto template_path = fs::path(PICKER_PROJECT_DIR) / "template" / "python" / "dut.py";
    return env.render_file(template_path.string(), data);
}

int main()
{
    const auto vcs_render = render_dut_template("vcs");
    assert(vcs_render.find("if \"-no_save\" not in args:\n            args = args + (\"-no_save\",)") != std::string::npos);
    assert(vcs_render.find("self.dut = DutUnifiedBase(*args)") != std::string::npos);

    const auto verilator_render = render_dut_template("verilator");
    assert(verilator_render.find("-no_save") == std::string::npos);

    return 0;
}
