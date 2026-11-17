#include "codegen/lib.hpp"
#include <optional>

namespace picker { namespace codegen {

    std::string signal_name(const picker::sv_signal_define &pin)
    {
        if (!pin.signal_path.empty()) { return pin.signal_path; }
        if (pin.array_name.empty()) { return pin.logic_pin; }
        auto name = pin.array_name;
        for (int index : pin.array_indices) { name += "[" + std::to_string(index) + "]"; }
        return name;
    }

    std::string render_array_binding(const picker::sv_signal_define &pin, const std::string &language,
                                     const std::string &element)
    {
        int64_t index = 0;
        for (size_t d = 0; d < pin.unpacked_dimensions.size(); ++d) {
            const auto [left, right] = pin.unpacked_dimensions[d];
            index = index * (std::abs(int64_t(left) - right) + 1)
                    + (int64_t(pin.array_indices[d]) - left) * (left <= right ? 1 : -1);
        }
        const auto name = nlohmann::json(str_replace_all(pin.array_name, ".", "_")).dump();
        const auto self = language == "cpp" ? "this->" :
                          language == "java" || language == "scala" ? "this." : "self.";
        const auto call = language == "cpp" ? "dut->" : language == "lua" ? "dut:" :
                          language == "golang" ? "Dut." : "dut.";
        const auto args = std::string(self) + element + (language == "lua" ? ":" : ".")
                          + "CSelf(), " + name + ", " + std::to_string(index);
        return std::string(language == "python" || language == "java" ? "        " : "    ")
               + self + call + "BindArrayElement(" + args + ")"
               + (language == "cpp" || language == "java" ? ";\n" : "\n");
    }

    bool render_array_ports(const std::vector<picker::sv_signal_define> &pins, const std::string &language,
                            std::string &declarations, std::string &initialization, const MemberNames *names)
    {
        std::optional<MemberNames> local_names;
        if (!names) { local_names.emplace(pins, std::vector<picker::sv_signal_define>{}, nlohmann::json::object(), language); names = &*local_names; }
        if (language == "cpp") initialization += "    this->xcfg->BindDataLayouts(this->xport);\n";
        else if (language == "python") initialization += "        self.xcfg.BindDataLayouts(self.xport)\n";
        else if (language == "java") initialization += "        this.xcfg.BindDataLayouts(this.xport);\n";
        else if (language == "scala") initialization += "    this.xcfg.BindDataLayouts(this.xport)\n";
        else if (language == "golang") initialization += "    self.Xcfg.BindDataLayouts(self.Xport)\n";
        else initialization += "    self.xcfg:BindDataLayouts(self.xport)\n";
        auto emit = [&](const std::string &path, const nlohmann::json &layout,
                        bool ordinary) {
            const auto &attribute = names->View(path);
            const auto name = nlohmann::json(path).dump();
            const std::string type = layout["kind"] == "struct" ? "XStruct" : "XArray";
            const std::string method = layout["kind"] == "struct" ? "Struct" : "Array";
            if (language == "cpp") {
                declarations += "    std::shared_ptr<" + type + "> " + attribute + ";\n";
                initialization += "    this->" + attribute + " = this->xcfg->NewViewFromPort(this->xport, " + name + ");\n";
            } else if (language == "python") {
                initialization += "        self." + attribute + " = self.xcfg.NewViewFromPort(self.xport, " + name + ")." + method + "()\n";
                const auto registry = ordinary ? "_picker_arrays" : "_picker_views";
                const auto key = name;
                initialization += "        self." + std::string(registry) + "[" + key + "] = self." + attribute + "\n";
            } else if (language == "java") {
                declarations += "    public " + type + " " + attribute + ";\n";
                initialization += "        this." + attribute + " = this.xcfg.NewViewFromPort(this.xport, " + name + ")." + method + "();\n";
            } else if (language == "scala") {
                initialization += "    val " + attribute + ": " + type + " = this.xcfg.NewViewFromPort(this.xport, " + name + ")." + method + "()\n";
            } else if (language == "golang") {
                declarations += "    " + attribute + " xspcomm." + type + "\n";
                initialization += "    {\n        view := self.Xcfg.NewViewFromPort(self.Xport, " + name + ")\n        self." + attribute + " = view." + method +
                                  "()\n        xspcomm.DeleteXView(view)\n    }\n";
            } else {
                initialization += "    self." + attribute + " = self.xcfg:NewViewFromPort(self.xport, " + name + "):" + method + "()\n";
            }
        };

        std::map<std::string, const picker::sv_signal_define *> arrays;
        for (const auto &pin : pins) {
            if (pin.aggregate_name.empty() && !pin.array_name.empty()) {
                arrays.emplace(pin.array_name, &pin);
            }
        }
        for (const auto &[name, pin] : arrays) {
            nlohmann::json layout{{"kind", "bits"}, {"packed", true},
                {"width", pin->logic_pin_hb == -1 ? 0 : pin->logic_pin_hb - pin->logic_pin_lb + 1},
                {"signed", pin->unpacked_element_type.find("signed") != std::string::npos},
                {"four_state", !pin->unpacked_element_type.starts_with("bit")}};
            for (auto it = pin->unpacked_dimensions.rbegin(); it != pin->unpacked_dimensions.rend(); ++it) {
                layout = {{"kind", "array"}, {"packed", false}, {"width", 0},
                          {"left", it->first}, {"right", it->second}, {"element", std::move(layout)}};
            }
            emit(name, layout, true);
        }
        std::set<std::string> seen;
        for (const auto &pin : pins) {
            if (pin.aggregate_name.empty() || !seen.insert(pin.aggregate_name).second) continue;
            const auto layout = nlohmann::json::parse(*pin.type_layout);
            if (layout["kind"] == "bits" || layout["packed"].get<bool>()) continue;
            emit(pin.aggregate_name, layout, false);
        }
        return !arrays.empty();
    }

}} // namespace picker::codegen
