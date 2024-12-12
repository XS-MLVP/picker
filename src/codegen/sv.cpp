#include <bits/stdc++.h>
#include "codegen/sv.hpp"

namespace picker { namespace codegen {

    namespace sv {
        static const std::string sub_module_template =
            " {{module_name}} {{module_name}}{{module_subfix}}(\n{{pin_connect}}\n );\n";
        static const std::string pin_connect_template           = "    .{{raw_pin}}({{logic_pin}}),\n";
        static const std::string logic_pin_declaration_template = "  logic {{logic_pin_length}} {{logic_pin}};\n";
        static const std::string wire_pin_declaration_template  = "  wire {{logic_pin_length}} {{logic_pin}};\n";

        static const std::string dpi_get_export_template =
            "  export \"DPI-C\" function get_{{pin_func_name}}xx{{__LIB_DPI_FUNC_NAME_HASH__}};\n";
        static const std::string dpi_set_export_template =
            "  export \"DPI-C\" function set_{{pin_func_name}}xx{{__LIB_DPI_FUNC_NAME_HASH__}};\n";

        static const std::string dpi_get_impl_template =
            "  function void get_{{pin_func_name}}xx{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
            "    output logic {{logic_pin_length}} value;\n"
            "    value={{logic_pin}};\n"
            "  endfunction\n\n";

        static const std::string dpi_set_impl_template =
            "  function void set_{{pin_func_name}}xx{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
            "    input logic {{logic_pin_length}} value;\n"
            "    {{logic_pin}}=value;\n"
            "  endfunction\n\n";

        static const std::string dpi_finish_sv_template =
            "  export \"DPI-C\" function finish_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
            "  function void finish_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
            "    $finish;\n"
            "  endfunction\n";

        /// @brief Export external pin for verilog render, contains pin connect,
        /// @param pin
        /// @param pin_connect
        /// @param logic
        /// @param wire
        /// @param dpi_export
        /// @param dpi_impl
        std::vector<picker::sv_signal_define>
        render_external_pin(std::vector<picker::sv_module_define> sv_module_result, std::string &inner_modules,
                            std::string &logic, std::string &wire, std::string &dpi_export, std::string &dpi_impl)
        {
            std::vector<picker::sv_signal_define> ret;
            inja::Environment env;
            auto module_diffs = sv_module_result.size();
            for (int diff_index = 0; diff_index < module_diffs; diff_index++) {
                auto &module = sv_module_result[diff_index];
                PK_DEBUG("module_name: %s, module_nums: %d", module.module_name.c_str(), module.module_nums);
                for (int count_index = 0; count_index < module.module_nums; count_index++) {
                    nlohmann::json data;
                    std::string pin_connect, pin_prefix;
                    auto &pin = module.pins;
                    if (module_diffs != 1 || module.module_nums != 1) pin_prefix = module.module_name + "_";
                    if (module.module_nums != 1) {
                        pin_prefix += std::to_string(count_index) + "_";
                        data["module_subfix"] = "_" + std::to_string(count_index);
                    } else
                        data["module_subfix"] = "";
                    data["module_name"] = module.module_name;
                    PK_DEBUG("module_subfix: %s, pin_prefix: %s", data["module_subfix"].dump().c_str(),
                             pin_prefix.c_str());
                    for (int i = 0; i < pin.size(); i++) {
                        picker::sv_signal_define temp_pin = pin[i];
                        temp_pin.logic_pin                = pin_prefix + pin[i].logic_pin;
                        ret.push_back(temp_pin); // pins need export
                        data["raw_pin"]                    = pin[i].logic_pin;
                        data["logic_pin"]                  = temp_pin.logic_pin;
                        data["logic_pin_type"]             = pin[i].logic_pin_type;
                        data["pin_func_name"]              = replace_all(temp_pin.logic_pin, ".", "_");
                        data["__LIB_DPI_FUNC_NAME_HASH__"] = std::string(lib_random_hash);
                        // Set empty or [hb:lb] for verilog render
                        data["logic_pin_length"] = pin[i].logic_pin_hb == -1 ?
                                                       "" :
                                                       "[" + std::to_string(pin[i].logic_pin_hb) + ":"
                                                           + std::to_string(pin[i].logic_pin_lb) + "]";
                        pin_connect += env.render(pin_connect_template, data);
                        logic += env.render(logic_pin_declaration_template, data);
                        wire += env.render(wire_pin_declaration_template, data);
                        dpi_export += env.render(dpi_get_export_template, data);
                        dpi_impl += env.render(dpi_get_impl_template, data);
                        if (pin[i].logic_pin_type != "output") {
                            dpi_export += env.render(dpi_set_export_template, data);
                            dpi_impl += env.render(dpi_set_impl_template, data);
                        }
                    }
                    if (pin_connect.length() == 0)
                        PK_FATAL(
                            "No port information of src_module was found in the specified file. \nPlease check whether the file name or source module name is correct.");
                    pin_connect.pop_back();
                    pin_connect.pop_back();
                    data["pin_connect"] = pin_connect;
                    // inner module
                    inner_modules = inner_modules + env.render(sub_module_template, data);
                }
            }
            return ret;
        }

        /// @brief Export internal signal for verilog render, only contains dpi
        /// get function
        /// @param pin
        /// @param dpi_export
        /// @param dpi_impl
        void render_internal_signal(std::vector<picker::sv_signal_define> pin, std::string &dpi_export,
                                    std::string &dpi_impl)
        {
            inja::Environment env;
            nlohmann::json data;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]                  = pin[i].logic_pin;
                data["logic_pin_type"]             = pin[i].logic_pin_type;
                data["pin_func_name"]              = replace_all(pin[i].logic_pin, ".", "_");
                data["__LIB_DPI_FUNC_NAME_HASH__"] = std::string(lib_random_hash);

                // Set empty or [hb:lb] for verilog render
                data["logic_pin_length"] = pin[i].logic_pin_hb == -1 ? "" :
                                                                       "[" + std::to_string(pin[i].logic_pin_hb) + ":"
                                                                           + std::to_string(pin[i].logic_pin_lb) + "]";

                dpi_export = dpi_export + env.render(dpi_get_export_template, data);
                dpi_impl   = dpi_impl + env.render(dpi_get_impl_template, data);
            }
        };

        void render_sv_waveform(const std::string &simulator, const std::string &wave_file_name, nlohmann::json &data)
        {
            inja::Environment env;
            std::string sv_dump_wave, trace, dum_var_options;
            dum_var_options = picker::get_env("DUMPVARS_OPTION", "");
            if (!dum_var_options.empty()) {
                PK_DEBUG("Find DUMPVARS_OPTION=%s", dum_var_options.c_str());
                dum_var_options = ", \"" + dum_var_options + "\"";
            }
            data["__WAVE_FILE_NAME__"]   = wave_file_name;
            data["__DUMP_VAR_OPTIONS__"] = dum_var_options;
            if (simulator == "verilator") {
                if (wave_file_name.length() > 0) {
                    if (wave_file_name.ends_with(".vcd") || wave_file_name.ends_with(".fst"))
                        sv_dump_wave =
                            env.render("  initial begin\n"
                                       "    $dumpfile(\"{{__WAVE_FILE_NAME__}}\");\n"
                                       "    $dumpvars(0, {{__TOP_MODULE_NAME__}}_top{{__DUMP_VAR_OPTIONS__}});\n"
                                       "  end",
                                       data);
                    else
                        PK_FATAL("Verilator trace file must be .vcd or .fst format.\n");
                }
            } else if (simulator == "vcs") {
                if (wave_file_name.length() > 0) {
                    if (wave_file_name.ends_with(".fsdb") == false) PK_FATAL("VCS trace file must be .fsdb format.\n");
                    sv_dump_wave =
                        env.render("  initial begin\n"
                                   "    $fsdbDumpfile(\"{{__WAVE_FILE_NAME__}}\");\n"
                                   "    $fsdbDumpvars(0, {{__TOP_MODULE_NAME__}}_top{{__DUMP_VAR_OPTIONS__}});\n"
                                   "  end",
                                   data);
                }
            } else {
                PK_FATAL("Unsupported simulator: %s\n", simulator.c_str());
            }

            data["__TRACE__"] = sv_dump_wave.length() > 0 ? wave_file_name.substr(wave_file_name.find_last_of(".") + 1,
                                                                                  wave_file_name.length()) :
                                                            "OFF";
            data["__SV_DUMP_WAVE__"] = sv_dump_wave;
        }

        void render_signal_tree(const std::vector<picker::sv_signal_define> &external_pin,
                                const std::vector<picker::sv_signal_define> &internal_signal, std::string &signal_tree,
                                nlohmann::json &signal_tree_json)
        {
            // iterate the signal pin and generate the signal tree by TRIE tree
            // get all the signal name and sort them
            std::vector<picker::sv_signal_define> signal_list;
            for (auto pin : external_pin) { signal_list.push_back(pin); }
            for (auto pin : internal_signal) { signal_list.push_back(pin); }

            // sort the signal list for handle the signal tree
            std::sort(signal_list.begin(), signal_list.end(),
                      [](auto &a, auto &b) { return a.logic_pin < b.logic_pin; });

            // split the string by delimiter, only considering the last one in
            // consecutive delimiters  AAA__1_BBB_C => (AAA_, 1, BBB, C)
            auto split = [](const std::string &str, char delimiter) {
                std::vector<std::string> tokens;
                std::stringstream ss;
                bool last_was_delimiter = false;
                for (char ch : str) {
                    if (ch == delimiter) {
                        ss << ch;
                        last_was_delimiter = true;
                    } else {
                        if (last_was_delimiter) {
                            tokens.push_back(ss.str().substr(0, ss.str().size() - 1));
                            ss.str("");
                        }
                        ss << ch;
                        last_was_delimiter = false;
                    }
                }
                if (!ss.str().empty()) { tokens.push_back(ss.str()); }
                return tokens;
            };

            // Creat trie node typedef for signal tree
            struct TrieNode {
                std::map<std::string, TrieNode *> children;
                bool isEndOfWord = false;
                std::string part_name, pin_type;
                int high_bit = -1, low_bit = -1;
            };

            // Build the TRIE
            TrieNode *root = new TrieNode();
            for (auto signal : signal_list) {
                std::vector<std::string> signal_split = split(replace_all(signal.logic_pin, ".", "_"), '_');
                TrieNode *current                     = root;
                for (auto part_name : signal_split) {
                    if (current->children.find(part_name) == current->children.end()) {
                        TrieNode *new_node           = new TrieNode();
                        new_node->part_name          = part_name;
                        current->children[part_name] = new_node;
                    }
                    current = current->children[part_name];
                }
                current->isEndOfWord = true;
                current->high_bit    = signal.logic_pin_hb;
                current->low_bit     = signal.logic_pin_lb;
                current->pin_type    = signal.logic_pin_type;
            }

            // Auto merge words with '_', if one node has only one child and not
            // number, merge them, (AAA_, 1, BBB, C) => (AAA_, 1, BBB_C)
            auto is_number = [](const std::string &s) {
                return !s.empty()
                       && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
            };

            std::function<void(TrieNode *)> merge_trie = [&](TrieNode *node) {
                while (node->children.size() == 1 && !is_number(node->part_name)
                       && !is_number(node->children.begin()->first)) {
                    auto child = node->children.begin();
                    if (node->part_name.size() > 0) { node->part_name += "_"; }
                    node->part_name += child->second->part_name;
                    TrieNode *temp = child->second;
                    node->children.clear();
                    for (auto &schild : temp->children) {
                        node->children[schild.first] = schild.second;
                        PK_DEBUG("transfer son %s", schild.first.c_str());
                    }
                    if (temp->isEndOfWord) {
                        node->isEndOfWord = true;
                        node->high_bit    = temp->high_bit;
                        node->low_bit     = temp->low_bit;
                        node->pin_type    = temp->pin_type;
                    }
                    delete temp;
                    PK_DEBUG("merge %s", node->part_name.c_str());
                }
                for (auto &child : node->children) { merge_trie(child.second); }
            };
            merge_trie(root);
            if (root->part_name != "") {
                TrieNode *new_root                  = new TrieNode();
                new_root->children[root->part_name] = root;
                root                                = new_root;
            } // if root is not empty, json generator may lost the root part

            // convert the TRIE to json
            std::function<void(TrieNode *, nlohmann::json &)> trie_to_json = [&](TrieNode *node, nlohmann::json &json) {
                if (node->isEndOfWord) {
                    json["Pin"]  = node->pin_type;
                    json["High"] = node->high_bit;
                    json["Low"]  = node->low_bit;
                    json["_"]    = true;
                }
                std::vector<std::pair<std::string, std::string>> rename_keys;
                for (const auto &child : node->children) {
                    if (child.first != child.second->part_name) {
                        PK_DEBUG("JSON child rename %s to %s", child.first.c_str(), child.second->part_name.c_str());
                        rename_keys.emplace_back(child.first, child.second->part_name);
                    }
                }
                for (const auto &rename : rename_keys) {
                    node->children[rename.second] = node->children[rename.first];
                    node->children.erase(rename.first);
                }
                for (auto &child : node->children) {
                    PK_DEBUG("JSON child %s", child.first.c_str());
                    nlohmann::json child_json;
                    trie_to_json(child.second, child_json);
                    json[child.first] = child_json;
                }
            };

            // json2string
            nlohmann::json &tries = signal_tree_json;
            trie_to_json(root, tries);
            signal_tree = tries.dump(4);

            // free the memory
            std::function<void(TrieNode *)> delete_trie = [&](TrieNode *node) {
                for (auto &child : node->children) { delete_trie(child.second); }
                delete node;
            };
            delete_trie(root);
        }

        void render_extend_sv(nlohmann::json &global_data, std::string &extend_sv)
        {
            inja::Environment env;
            nlohmann::json data;
            data["__LIB_DPI_FUNC_NAME_HASH__"] = std::string(lib_random_hash);
            extend_sv                          = env.render(dpi_finish_sv_template, data);
        }

    } // namespace sv
    /// @brief generate system verilog wrapper file contains
    /// __PIN_CONNECT__ , __LOGIC_PIN_DECLARATION__ , __WIRE_PIN_DECLARATION__
    /// __DPI_FUNCTION_EXPORT__ and __DPI_FUNCTION_IMPLEMENT__
    /// @param global_render_data
    /// @param external_pin
    /// @param internal_signal
    std::vector<picker::sv_signal_define> gen_sv_param(nlohmann::json &global_render_data,
                                                       const std::vector<picker::sv_module_define> &sv_module_result,
                                                       const std::vector<picker::sv_signal_define> &internal_signal,
                                                       nlohmann::json &signal_tree_json,
                                                       const std::string &wave_file_name, const std::string &simulator)
    {
        std::string inner_modules, logic, wire, dpi_export, dpi_impl, signal_tree, extend_sv;

        auto external_pin = sv::render_external_pin(sv_module_result, inner_modules, logic, wire, dpi_export, dpi_impl);
        sv::render_internal_signal(internal_signal, dpi_export, dpi_impl);
        sv::render_sv_waveform(simulator, wave_file_name, global_render_data);
        sv::render_signal_tree(external_pin, internal_signal, signal_tree, signal_tree_json);
        sv::render_extend_sv(global_render_data, extend_sv);

        global_render_data["__LOGIC_PIN_DECLARATION__"]  = logic;
        global_render_data["__WIRE_PIN_DECLARATION__"]   = wire;
        global_render_data["__INNER_MODULES__"]          = inner_modules;
        global_render_data["__DPI_FUNCTION_EXPORT__"]    = dpi_export;
        global_render_data["__DPI_FUNCTION_IMPLEMENT__"] = dpi_impl;
        global_render_data["__EXTEND_SV__"]              = extend_sv;
        global_render_data["__LIB_DPI_FUNC_NAME_HASH__"] = std::string(lib_random_hash);
        global_render_data["__SIGNAL_TREE__"]            = signal_tree;
        return external_pin;
    }

}} // namespace picker::codegen
