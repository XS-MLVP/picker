#include "codegen/names.hpp"
#include <sstream>

namespace picker::codegen {

std::string pin_direction(const std::string &verilog_direction, const std::string &language)
{
    const bool inout = verilog_direction == "inout";
    const bool input = verilog_direction == "input";
    if (language == "golang") {
        return inout ? "IOType_InOut" : input ? "IOType_Input" : "IOType_Output";
    }
    // xcomm spells the C++ enumerators Input/Output, unlike the SWIG names.
    if (language == "cpp") {
        return inout ? "InOut" : input ? "Input" : "Output";
    }
    return inout ? "InOut" : input ? "In" : "Out";
}

std::string verilator_identifier(const std::string &name)
{
    // Verilator encodes the second underscore of each pair, and dollar signs.
    // Apply underscore escaping first so generated escape markers stay intact.
    return str_replace_all(str_replace_all(name, "__", "___05F"), "$", "__024");
}

std::string verilator_member_expression(const std::string &expression)
{
    static const std::regex identifier(R"([A-Za-z_][A-Za-z0-9_$]*)");
    std::string result;
    size_t end = 0;
    for (auto it = std::sregex_iterator(expression.begin(), expression.end(), identifier);
         it != std::sregex_iterator(); ++it) {
        result.append(expression, end, it->position() - end);
        const auto token = it->str();
        // __PVT__ is added by aggregate lowering, not part of the RTL field.
        const bool member = it->position() > 0 && expression[it->position() - 1] == '.';
        result += member && token.starts_with("__PVT__") ? "__PVT__" + verilator_identifier(token.substr(7)) :
                                                           verilator_identifier(token);
        end = it->position() + it->length();
    }
    result.append(expression, end, expression.size() - end);
    return result;
}

MemberNames::MemberNames(const std::vector<sv_signal_define> &external,
                         const std::vector<sv_signal_define> &internal,
                         const nlohmann::json &tree, const std::string &language,
                         const std::string &module)
{
    auto reserved = get_default_conflict_map();
    // Reserve the complete wrapper surface, even if a feature is disabled.
    reserved[lower_case("UT" + module)] = "pin_";
    reserved[lower_case("UT_" + module)] = "pin_";
    const std::map<std::string, std::string> keywords{
        {"python", "False None True and as assert async await break class continue def del elif else except finally for from global if import in is lambda nonlocal not or pass raise return try while with yield __class__ __dict__ __weakref__"},
        {"cpp", "alignas alignof and and_eq asm auto bitand bitor bool break case catch char char8_t char16_t char32_t class compl concept const consteval constexpr constinit const_cast continue co_await co_return co_yield decltype default delete do double dynamic_cast else enum explicit export extern false float for friend goto if inline int long mutable namespace new noexcept not not_eq nullptr operator or or_eq private protected public register reinterpret_cast requires return short signed sizeof static static_assert static_cast struct switch template this thread_local throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while xor xor_eq"},
        {"java", "abstract assert boolean break byte case catch char class const continue default do double else enum extends final finally float for goto if implements import instanceof int interface long native new package private protected public return short static strictfp super switch synchronized this throw throws transient try void volatile while true false null _ getClass hashCode equals clone toString notify notifyAll wait finalize"},
        {"scala", "abstract case catch class def do else extends false final finally for forSome if implicit import lazy match new null object override package private protected return sealed super this throw trait try true type val var while with yield enum export given then end inline opaque open transparent using extension derives as infix erased equals hashCode toString getClass notify notifyAll wait finalize clone"},
        {"golang", "break default func interface select case defer go map struct chan else goto package switch const fallthrough if range type continue for import return var"},
        {"lua", "and break do else elseif end false for function goto if in local nil not or repeat return then true until while"}};
    std::istringstream words(keywords.at(language));
    for (std::string word; words >> word;) reserved[lower_case(word)] = "pin_";
    auto normalize = [&](std::string name) {
        if (language == "python") name = str_replace_all(name, "$$", "_");
        for (char &c : name) if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                                  (c >= '0' && c <= '9') || c == '_')) c = '_';
        if (name.empty() || (name.front() >= '0' && name.front() <= '9')) name = "pin_" + name;
        return language == "golang" ? capitalize_first_letter(name) : name;
    };
    std::map<std::string, std::string> requested;
    std::set<std::string> original;
    for (const auto *pins : {&external, &internal}) for (const auto &pin : *pins) {
        requested["pin:" + pin.logic_pin] = normalize(pin.logic_pin);
        original.insert(normalize(pin.logic_pin));
        if (!pin.aggregate_name.empty()) original.insert(normalize(pin.aggregate_name));
        else if (!pin.array_name.empty()) original.insert(normalize(pin.array_name));
    }
    std::map<std::string, const sv_signal_define *> aggregates;
    for (const auto &pin : external) {
        if (!pin.aggregate_name.empty()) {
            aggregates.emplace(pin.aggregate_name, &pin);
        } else if (!pin.array_name.empty()) {
            requested["view:" + pin.array_name] = normalize(pin.array_name);
        }
    }
    // Sort before choosing packed-view suffixes as well as before final
    // allocation: distinct RTL names can normalize to the same attribute.
    for (const auto &[name, pin] : aggregates) {
        const auto layout = nlohmann::json::parse(*pin->type_layout);
        if (layout["kind"] == "bits") continue;
        auto attribute = normalize(name);
        if (layout["packed"].get<bool>()) {
            attribute += "_view";
            while (original.count(attribute)) attribute += "_";
        }
        original.insert(attribute);
        requested["view:" + name] = attribute;
    }
    std::function<void(const std::string &, const nlohmann::json &)> cascade;
    cascade = [&](const std::string &prefix, const nlohmann::json &node) {
        for (const auto &[key, port] : node.items()) {
            if (port.contains("_")) continue;
            const auto path = prefix.empty() ? key : prefix + "_" + key;
            requested["cascade:" + path] = normalize(path);
            cascade(path, port);
        }
    };
    cascade("", tree);

    std::set<std::string> used;
    // Give all unchanged, non-reserved names priority. A renamed `dut` must not
    // steal a real `pin_dut` port, regardless of declaration/traversal order.
    for (const auto &[key, name] : requested) {
        if (!reserved.count(lower_case(name)) && used.insert(name).second) names_[key] = name;
    }
    for (const auto &[key, name] : requested) {
        if (names_.count(key)) continue;
        auto base = reserved.count(lower_case(name)) ? normalize("pin_" + name) : name;
        auto candidate = base;
        for (unsigned suffix = 2; reserved.count(lower_case(candidate)) || used.count(candidate); ++suffix)
            candidate = base + "_" + std::to_string(suffix);
        used.insert(candidate);
        names_[key] = candidate;
    }
}

} // namespace picker::codegen
