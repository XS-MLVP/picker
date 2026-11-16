#include "codegen/mem_direct.hpp"
#include "codegen/render_tags.hpp"
#include "codegen/lib.hpp"

#define MAX_VAR_PERFILE 5000

namespace picker { namespace codegen {

    static const std::string variable_info_template =
        "    cpp_variableInfo {{var_hash}} = {\"{{var_name}}\", \"{{var_type}}\", {{var_width}}, {{var_array_size}}, (uint64_t)&(base.{{var_raw_name}}) - (uint64_t)&base, {{var_dimensions}}, {{var_strides}}, {{var_mem_bytes}}, {{var_layout}}, {{var_error}}, {{var_groups}} };\n"
        "    all_vars.push_back({{var_hash}});\n";

    std::string replace_dot(const std::string &varName)
    {
        // __DOT__ -> .  , fast mode
        std::string out;
        out.reserve(varName.size());
        for (size_t i = 0; i < varName.size(); i++) {
            if (i + 6 < varName.size() && varName[i] == '_' && varName[i + 1] == '_' && varName[i + 2] == 'D'
                && varName[i + 3] == 'O' && varName[i + 4] == 'T' && varName[i + 5] == '_' && varName[i + 6] == '_') {
                out.push_back('.');
                i += 6;
            } else {
                out.push_back(varName[i]);
            }
        }
        return out;
    }

    void generate_offset(const std::vector<cpp_variableInfo> &variables, std::vector<std::string> &offset_array,
                         std::string &offset_funcs, std::string &offset_funcs_define)
    {
        inja::Environment env;
        nlohmann::json data;
        int var_count = 0, file_count = 0;
        offset_array.push_back("");

        for (auto &var : variables) {
            // calc hash from name
            data["var_hash"]       = "hash_" + std::to_string(std::hash<std::string>{}(var.public_name.empty() ? var.name : var.public_name));
            data["var_raw_name"]   = var.name;
            data["var_name"]       = var.public_name.empty() ? replace_dot(var.name) : var.public_name;
            data["var_type"]       = var.type;
            data["var_width"]      = var.width;
            data["var_layout"] = nlohmann::json(var.layout).dump();
            data["var_error"] = nlohmann::json(var.view_error).dump();
            std::string groups = "{";
            auto first_member = var.name;
            for (size_t d = 0; d < var.array_dimensions.size(); ++d) first_member += "[0]";
            const bool reuse_storage = var.native_groups.size() == 1 && var.width > 0 &&
                var.native_groups.front().member == first_member &&
                var.native_groups.front().width == var.width &&
                var.native_groups.front().dimensions == var.array_dimensions;
            for (const auto &group : var.native_groups) {
                if (reuse_storage) break; // Only a type reference is new for ordinary native values.
                if (groups.size() > 1) groups += ", ";
                groups += "{" + nlohmann::json(group.path).dump() + ", " + std::to_string(group.width) +
                    ", sizeof(base." + group.member + "), (uint64_t)&(base." + group.member + ") - (uint64_t)&base, {";
                for (size_t i = 0; i < group.dimensions.size(); ++i) {
                    if (i) groups += ", ";
                    groups += std::to_string(group.dimensions[i]);
                }
                groups += "}, {";
                for (size_t i = 0; i < group.strides.size(); ++i) {
                    if (i) groups += ", ";
                    groups += "sizeof(base." + group.strides[i] + ")";
                }
                groups += "}}";
            }
            data["var_groups"] = groups + "}";
            data["var_array_size"] = var.array_size;
            std::string dimensions = "{", strides = "{", member = "base." + var.name;
            for (size_t dimension = 0; dimension < var.array_dimensions.size(); ++dimension) {
                if (dimension) { dimensions += ", "; strides += ", "; }
                dimensions += std::to_string(var.array_dimensions[dimension]);
                member += "[0]";
                strides += "sizeof(" + member + ")";
            }
            data["var_dimensions"] = dimensions + "}";
            data["var_strides"] = strides + "}";
            data["var_mem_bytes"] = "sizeof(" + member + ")";
            offset_array[file_count] += env.render(variable_info_template, data);
            if (++var_count >= MAX_VAR_PERFILE) {
                var_count = 0;
                file_count++;
                offset_array.push_back("");
            }
        }
        for (int i = 0; i < offset_array.size(); i++) {
            offset_funcs += "    render_variable_info_" + std::to_string(i) + "(base);\n";
            offset_funcs_define += "void render_variable_info_" + std::to_string(i) + "(BaseType &base);\n";
        }
    }
    

    void write_offset_files(const std::vector<std::string> &offset_array, const std::string &dst_dir)
    {
        // Each file is emitted directly: a two-pass regex/inja expansion made
        // the generated function name impossible to follow from the template.
        // Write into the destination, after recursive_render: the emitted C++
        // contains nested braces such as {{".field", 1, ...}} for struct
        // layouts, which inja would otherwise parse as a template expression.
        for (size_t i = 0; i < offset_array.size(); ++i) {
            const auto path = dst_dir + "/gen_addr_sub_" + std::to_string(i) + ".cpp";
            write_file(path, "#include \"gen_addr.hpp\"\n\nvoid render_variable_info_" + std::to_string(i) +
                                 "(BaseType &base)\n{\n" + offset_array[i] + "\n    return;\n}\n");
        }
    }

    void render_md_addr_generator(const std::vector<cpp_variableInfo> &variables, picker::export_opts &opts,
                                  const nlohmann::json &types)
    {
        if (variables.size() == 0) { return; }
        inja::Environment env;
        nlohmann::json data;
        std::string erro_message;
        std::vector<std::string> offset_array;
        std::string offset_funcs, offset_funcs_define; // Calls and declarations for the generated offset functions.
        std::string src_dir = opts.source_dir; // "mem_direct" folder has been added in the upper level
        std::string dst_dir = opts.target_dir;
        std::string dst_dir_tmp = std::filesystem::path(dst_dir).parent_path().string() + "/mem_direct_tmp";
        // copy files
        std::filesystem::create_directories(dst_dir_tmp);
        std::filesystem::copy(src_dir + "/gen_addr.hpp", dst_dir_tmp + "/gen_addr.hpp", std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy(src_dir + "/gen_addr.cpp", dst_dir_tmp + "/gen_addr.cpp", std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy(src_dir + "/Makefile", dst_dir_tmp + "/Makefile", std::filesystem::copy_options::overwrite_existing);
        generate_offset(variables, offset_array, offset_funcs, offset_funcs_define);
        data[tags::top_module_name] = opts.source_module_name_list[0];
        data[tags::native_type_ids] = nlohmann::json::array();
        for (const auto &[id, type] : types.items()) data[tags::native_type_ids].push_back(nlohmann::json(id).dump());
        data[tags::render_variable_info_sub]        = "    // generated by codegen\n" + offset_funcs;
        data[tags::render_variable_info_sub_define] = offset_funcs_define;
        auto cpplib_location = picker::get_xcomm_lib("lib", erro_message);
        if (cpplib_location.empty()) { PK_FATAL("%s", erro_message.c_str()); }
        data[tags::xspcomm_lib]     = cpplib_location;
        data[tags::xspcomm_include] = picker::get_xcomm_lib("include", erro_message);
        data[tags::simulator]       = opts.sim;
        recursive_render(dst_dir_tmp, dst_dir, data, env);
        // After rendering: these files are final C++, not templates.
        write_offset_files(offset_array, dst_dir);
    }

}}; // namespace picker::codegen
