#pragma once
#include "picker.hpp"

namespace picker::codegen {

// Host-language XData IO type for a pin direction. Verilog has three
// directions, so a two-way test on the first character is not enough: an
// inout pin read back through an Input XData returns the value the testbench
// last wrote instead of the value the DUT drives.
//   languages "python", "java", "scala", "lua": In / Out / InOut
//   language  "cpp":                           Input / Output / InOut (xcomm::IOType)
//   language  "golang":                        IOType_Input / IOType_Output / IOType_InOut
std::string pin_direction(const std::string &verilog_direction, const std::string &language = "python");

// C++ spelling of ordinary Verilog identifiers in Verilator's generated root.
// This is separate from wrapper attribute allocation and never changes RTL names.
std::string verilator_identifier(const std::string &name);
std::string verilator_member_expression(const std::string &expression);

// One allocation for the whole wrapper, shared by leaves, views and subports.
// Only host-language attributes change; simulator handles and XPort keys do not.
class MemberNames {
    std::map<std::string, std::string> names_;
public:
    MemberNames(const std::vector<sv_signal_define> &external,
                const std::vector<sv_signal_define> &internal,
                const nlohmann::json &tree, const std::string &language,
                const std::string &module = "");
    const std::string &Pin(const std::string &name) const { return names_.at("pin:" + name); }
    const std::string &View(const std::string &name) const { return names_.at("view:" + name); }
    const std::string &Cascade(const std::string &name) const { return names_.at("cascade:" + name); }
};

} // namespace picker::codegen
