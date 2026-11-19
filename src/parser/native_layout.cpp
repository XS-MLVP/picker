#include "parser/native_layout.hpp"
#include "parser/verilator_root.hpp"
#include "verilator_root/common.hpp"

#include <fstream>
#include <functional>
#include <map>
#include <set>
#include <stdexcept>

namespace picker::parser {
namespace {
using J = nlohmann::json;
using Members = std::map<std::string, std::string>;

std::string logical_name(std::string name) {
    for (size_t pos = 0; (pos = name.find("__DOT__", pos)) != std::string::npos; ++pos)
        name.replace(pos, 7, ".");
    // Verilator emits generated array scopes as __BRA__N__KET__.
    static const std::regex index("__BRA__([0-9]+)__KET__");
    if (name.find("__BRA__") != std::string::npos) name = std::regex_replace(name, index, "[$1]");
    return name;
}

struct Binder {
    const J &types;
    // Struct definitions come from the same generator as the root header, so
    // one format covers every scanned type spelling.
    verilator::detail::root_format format;
    std::map<std::string, Members> structs;
    std::map<std::string, cpp_variableInfo> parsed;

    explicit Binder(const J &types, const std::string &header)
        : types(types), format(verilator::detail::detect_root_format(header)) {
        // Types are emitted in generated headers next to the root. Only simple
        // data-member declarations are accepted, never C++ method bodies.
        const std::regex start(R"(^\s*struct\s+([A-Za-z_]\w*)\s*\{\s*$)");
        const std::regex member(R"(^\s*(.+\S)\s+([A-Za-z_]\w*);\s*$)");
        for (const auto &entry : std::filesystem::directory_iterator(std::filesystem::path(header).parent_path())) {
            if (entry.path().extension() != ".h") continue;
            std::ifstream stream(entry.path());
            std::string line, name;
            bool members = false;
            while (std::getline(stream, line)) {
                std::smatch match;
                if (std::regex_match(line, match, start)) {
                    name = match[1]; members = true;
                    structs.try_emplace(name);
                } else if (line.find("};") != std::string::npos) {
                    name.clear(); members = false;
                } else if (!name.empty() && members) {
                    if (line.find('(') != std::string::npos && line.find("alignas") == std::string::npos) {
                        members = false;
                    } else if (std::regex_match(line, match, member)) {
                        structs[name].emplace(match[2], picker::trim(match[1]));
                    }
                }
            }
        }
    }

    const cpp_variableInfo &parse(const std::string &text) {
        auto [found, inserted] = parsed.try_emplace(text);
        if (inserted) found->second = verilator::detail::parse_root_type(text, format);
        return found->second;
    }

    void group(const std::string &id, std::string native, std::string path,
               std::string cpp_type, std::vector<int> dimensions,
               std::vector<std::string> strides, std::vector<cpp_native_group> &groups,
               std::set<std::string> &active) {
        if (active.size() >= 128 || !active.insert(id).second)
            throw std::invalid_argument("recursive or excessively deep native type");
        const auto &node = types.at(id);
        static const std::regex unpacked(R"(^VlUnpacked<(.+),\s*(\d+)>$)");
        cpp_type = verilator::detail::normalize_root_type(std::move(cpp_type), format);
        const bool packed = node.at("packed").get<bool>();
        if (packed) {
            const auto &storage = parse(cpp_type);
            const auto width = node.at("width").get<int>();
            if (storage.type.empty() || storage.array_size || storage.width != (width ? width : 1))
                throw std::invalid_argument("packed RTL/native width or representation mismatch");
            groups.push_back({path, native, storage.width, dimensions, strides});
        } else if (node.at("kind") == "array") {
            std::smatch match;
            if (!std::regex_match(cpp_type, match, unpacked))
                throw std::invalid_argument("unpacked RTL array has no VlUnpacked storage");
            const auto left = node.at("left").get<int64_t>(), right = node.at("right").get<int64_t>();
            const auto size = std::abs(left - right) + 1;
            if (size > INT32_MAX || std::stoll(match[2]) != size)
                throw std::invalid_argument("unpacked RTL/native dimension mismatch");
            dimensions.push_back(int(size));
            native += "[0]";
            strides.push_back(native);
            group(node.at("element"), native, path + "[]", picker::trim(match[1]),
                  dimensions, strides, groups, active);
        } else if (node.at("kind") == "struct") {
            auto found = structs.find(cpp_type);
            if (found == structs.end()) throw std::invalid_argument("native struct definition is unavailable");
            if (found->second.size() != node.at("fields").size())
                throw std::invalid_argument("RTL/native struct member count mismatch");
            for (const auto &field : node.at("fields")) {
                const auto name = field.at("name").get<std::string>();
                auto member = found->second.find("__PVT__" + name);
                if (member == found->second.end()) member = found->second.find(name);
                if (member == found->second.end()) throw std::invalid_argument("native struct member not found: " + name);
                group(field.at("type"), native + "." + member->first, path + "." + name,
                      member->second, dimensions, strides, groups, active);
            }
        } else throw std::invalid_argument("unsupported unpacked native type");
        active.erase(id);
    }
};
}

nlohmann::json bind_native_layouts(const std::string &header, const J &metadata,
                                  std::vector<cpp_variableInfo> &variables,
                                  const std::vector<trace_signal_info> &aliases) {
    if (metadata.value("version", 0) != 1 || !metadata.at("types").is_object() ||
        !metadata.at("signals").is_object()) throw std::invalid_argument("Invalid native semantic metadata");
    const auto &types = metadata.at("types"), &signals = metadata.at("signals");
    Binder binder(types, header);
    std::map<std::string, std::pair<std::string, std::string>> declarations;
    const std::regex declaration(R"(^(.+\S)\s+([A-Za-z_]\w*);$)");
    for (const auto &line : verilator::readVarDeclarations(header)) {
        std::smatch match;
        if (std::regex_match(line, match, declaration)) {
            const auto raw = match[2].str();
            declarations.emplace(logical_name(raw), std::make_pair(raw, match[1].str()));
        }
    }
    std::map<std::string, size_t> existing;
    for (size_t i = 0; i < variables.size(); ++i) existing.emplace(variables[i].name, i);
    // A whole-value trace alias proves a name relation, not a new address.
    // Re-check the alias's own Slang type against that same C++ declaration.
    for (const auto &alias : aliases) {
        if (alias.kind != "direct" || declarations.count(alias.name)) continue;
        const auto source = declarations.find(alias.root_name);
        if (source == declarations.end()) continue;
        const auto &storage = binder.parse(source->second.second);
        if (!storage.type.empty() && storage.width == alias.width)
            declarations.emplace(alias.name, source->second);
    }
    J used = J::object();
    std::function<void(const std::string &)> retain = [&](const std::string &id) {
        if (used.contains(id)) return;
        const auto &node = types.at(id);
        used[id] = node;
        if (node.contains("element")) retain(node.at("element"));
        if (node.contains("fields")) for (const auto &field : node.at("fields")) retain(field.at("type"));
    };
    size_t matched = 0, rejected = 0;
    for (const auto &[name, id_node] : signals.items()) {
        auto found = declarations.find(name);
        if (found == declarations.end()) continue; // optimized, relocated, or not retained
        const auto &[raw, cpp_type] = found->second;
        const bool alias = logical_name(raw) != name;
        const auto key = alias ? name : raw;
        const auto id = id_node.get<std::string>();
        std::vector<cpp_native_group> groups;
        std::set<std::string> active;
        try {
            binder.group(id, raw, "", cpp_type, {}, {}, groups, active);
            size_t index;
            if (auto it = existing.find(key); it != existing.end()) index = it->second;
            else {
                index = variables.size();
                cpp_variableInfo variable;
                variable.name = raw;
                const auto &storage = binder.parse(cpp_type);
                variable.type = storage.type.empty() ? cpp_type : storage.type;
                variable.width = storage.width; variable.array_size = storage.array_size;
                variable.array_dimensions = storage.array_dimensions;
                if (alias) variable.public_name = name;
                variables.push_back(std::move(variable));
                existing.emplace(key, index);
            }
            variables[index].layout = id;
            variables[index].native_groups = std::move(groups);
            retain(id);
            ++matched;
        } catch (const std::exception &error) {
            ++rejected;
            if (auto it = existing.find(key); it != existing.end()) variables[it->second].view_error = error.what();
        }
    }
    PK_MESSAGE("Native RTL alignment: %zu verified, %zu incompatible, %zu without matching root storage; %zu retained types",
               matched, rejected, signals.size() - matched - rejected, used.size());
    return used;
}
}
