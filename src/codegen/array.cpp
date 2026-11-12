#include "codegen/lib.hpp"

namespace picker { namespace codegen {

    std::string signal_name(const picker::sv_signal_define &pin)
    {
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
                            std::string &declarations, std::string &initialization)
    {
        std::map<std::string, std::vector<const picker::sv_signal_define *>> arrays;
        for (const auto &pin : pins) {
            if (!pin.array_name.empty()) { arrays[pin.array_name].push_back(&pin); }
        }
        auto pin_map = picker::get_default_conflict_map();
        for (const auto &[name, elements] : arrays) {
            const auto &bounds = elements.front()->unpacked_dimensions;
            auto attribute = picker::fix_conflict_pin_name(str_replace_all(name, ".", "_"), pin_map,
                                                          language == "golang");
            if (language == "python") { attribute = str_replace_all(attribute, "$", "_"); }

            std::function<std::string(size_t)> array_type = [&](size_t rank) -> std::string {
                if (rank == 0) {
                    return language == "cpp" ? "XData*" : language == "golang" ? "xspcomm.XData" : "XData";
                }
                auto child = array_type(rank - 1);
                if (language == "cpp") { return "PickerSignalArray<" + child + ">"; }
                if (language == "java") { return "Map<Integer, " + child + ">"; }
                if (language == "scala") { return "Map[Int, " + child + "]"; }
                return "map[int]" + child;
            };
            size_t position = 0;
            std::function<std::string(size_t)> initializer = [&](size_t dimension) -> std::string {
                if (dimension == bounds.size()) {
                    auto pin = elements.at(position++);
                    auto prefix = language == "cpp" ? "&this->" :
                                  language == "java" || language == "scala" ? "this." : "self.";
                    return prefix + pin->logic_pin;
                }
                const auto [left, right] = bounds[dimension];
                const int step = left <= right ? 1 : -1;
                std::string children;
                for (int64_t index = left;; index += step) {
                    if (!children.empty()) { children += ", "; }
                    if (language == "golang") { children += std::to_string(index) + ": "; }
                    if (language == "scala") { children += std::to_string(index) + " -> "; }
                    if (language == "lua") { children += "[" + std::to_string(index) + "] = "; }
                    children += initializer(dimension + 1);
                    if (index == right) { break; }
                }
                if (language == "python") {
                    std::string ranges;
                    for (size_t d = dimension; d < bounds.size(); ++d) {
                        ranges += "(" + std::to_string(bounds[d].first) + ", "
                                      + std::to_string(bounds[d].second) + "),";
                    }
                    return "_SignalArray((" + ranges + "), (" + children + ",))";
                }
                if (language == "cpp") {
                    return array_type(bounds.size() - dimension) + "{" + std::to_string(left) + ", "
                           + std::to_string(right) + ", {" + children + "}}";
                }
                if (language == "java") {
                    return "signalArray(" + std::to_string(left) + ", " + std::to_string(right) + ", " + children + ")";
                }
                if (language == "scala") { return "scala.collection.immutable.ListMap(" + children + ")"; }
                return (language == "golang" ? array_type(bounds.size() - dimension) : "") + "{" + children + "}";
            };
            const auto value = initializer(0);
            const auto type = array_type(bounds.size());
            if (language == "cpp") {
                declarations += "    " + type + " " + attribute + ";\n";
                initialization += "    this->" + attribute + " = " + value + ";\n";
            } else if (language == "java") {
                declarations += "    public " + type + " " + attribute + ";\n";
                initialization += "        this." + attribute + " = " + value + ";\n";
            } else if (language == "golang") {
                declarations += "    " + attribute + " " + type + "\n";
                initialization += "    self." + attribute + " = " + value + "\n";
            } else if (language == "scala") {
                initialization += "    val " + attribute + ": " + type + " = " + value + "\n";
            } else {
                initialization += (language == "python" ? "        " : "    ") + std::string("self.")
                                  + attribute + " = " + value + "\n";
                if (language == "python") {
                    initialization += "        self._picker_arrays[" + nlohmann::json(name).dump()
                                      + "] = self." + attribute + "\n";
                }
            }
        }
        return !arrays.empty();
    }

}} // namespace picker::codegen
