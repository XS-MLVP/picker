#pragma once
#include "picker.hpp"

namespace picker::codegen {

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
