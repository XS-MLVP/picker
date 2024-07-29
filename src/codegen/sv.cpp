#include <bits/stdc++.h>
#include "codegen/sv.hpp"

namespace picker { namespace codegen {

    namespace sv {
        static const std::string pin_connect_template =
            "    .{{logic_pin}}({{logic_pin}}),\n";
        static const std::string logic_pin_declaration_template =
            "  logic {{logic_pin_length}} {{logic_pin}};\n";
        static const std::string wire_pin_declaration_template =
            "  wire {{logic_pin_length}} {{logic_pin}};\n";

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

        /// @brief Export external pin for verilog render, contains pin connect,
        /// @param pin
        /// @param pin_connect
        /// @param logic
        /// @param wire
        /// @param dpi_export
        /// @param dpi_impl
        void render_external_pin(std::vector<picker::sv_signal_define> pin,
                                 std::string &pin_connect, std::string &logic,
                                 std::string &wire, std::string &dpi_export,
                                 std::string &dpi_impl)
        {
            inja::Environment env;
            nlohmann::json data;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"] = replace_all(pin[i].logic_pin, ".", "_");
                data["__LIB_DPI_FUNC_NAME_HASH__"] =
                    std::string(lib_random_hash);

                // Set empty or [hb:lb] for verilog render
                data["logic_pin_length"] =
                    pin[i].logic_pin_hb == -1 ?
                        "" :
                        "[" + std::to_string(pin[i].logic_pin_hb) + ":"
                            + std::to_string(pin[i].logic_pin_lb) + "]";

                pin_connect =
                    pin_connect + env.render(pin_connect_template, data);
                logic =
                    logic + env.render(logic_pin_declaration_template, data);
                wire = wire + env.render(wire_pin_declaration_template, data);

                dpi_export = dpi_export
                             + env.render(dpi_get_export_template, data)
                             + env.render(dpi_set_export_template, data);
                dpi_impl = dpi_impl + env.render(dpi_get_impl_template, data)
                           + env.render(dpi_set_impl_template, data);
            }
            if (pin_connect.length() == 0)
                PK_FATAL(
                    "No port information of src_module was found in the specified file. \nPlease check whether the file name or source module name is correct.");
            pin_connect.pop_back();
            pin_connect.pop_back();
        }

        /// @brief Export internal signal for verilog render, only contains dpi
        /// get function
        /// @param pin
        /// @param dpi_export
        /// @param dpi_impl
        void render_internal_signal(std::vector<picker::sv_signal_define> pin,
                                    std::string &dpi_export,
                                    std::string &dpi_impl)
        {
            inja::Environment env;
            nlohmann::json data;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"] = replace_all(pin[i].logic_pin, ".", "_");
                data["__LIB_DPI_FUNC_NAME_HASH__"] =
                    std::string(lib_random_hash);

                // Set empty or [hb:lb] for verilog render
                data["logic_pin_length"] =
                    pin[i].logic_pin_hb == -1 ?
                        "" :
                        "[" + std::to_string(pin[i].logic_pin_hb) + ":"
                            + std::to_string(pin[i].logic_pin_lb) + "]";

                dpi_export =
                    dpi_export + env.render(dpi_get_export_template, data);
                dpi_impl = dpi_impl + env.render(dpi_get_impl_template, data);
            }
        };

        void render_sv_waveform(const std::string &simulator,
                                const std::string &wave_file_name,
                                nlohmann::json &data)
        {
            inja::Environment env;
            std::string sv_dump_wave, trace;
            data["__WAVE_FILE_NAME__"] = wave_file_name;
            if (simulator == "verilator") {
                if (wave_file_name.length() > 0) {
                    if (wave_file_name.ends_with(".vcd")
                        || wave_file_name.ends_with(".fst"))
                        sv_dump_wave = env.render(
                            "initial begin\n"
                            "    $dumpfile(\"{{__WAVE_FILE_NAME__}}\");\n"
                            "    $dumpvars(0, {{__TOP_MODULE_NAME__}}_top);\n"
                            " end ",
                            data);
                    else
                        PK_FATAL(
                            "Verilator trace file must be .vcd or .fst format.\n");
                }
            } else if (simulator == "vcs") {
                if (wave_file_name.length() > 0) {
                    if (wave_file_name.ends_with(".fsdb") == false)
                        PK_FATAL("VCS trace file must be .fsdb format.\n");
                    sv_dump_wave = env.render(
                        "initial begin\n"
                        "    $fsdbDumpfile(\"{{__WAVE_FILE_NAME__}}\");\n"
                        "    $fsdbDumpvars(0, {{__TOP_MODULE_NAME__}}_top);\n"
                        " end ",
                        data);
                }
            } else {
                PK_FATAL("Unsupported simulator: %s\n", simulator.c_str());
            }

            data["__TRACE__"] =
                sv_dump_wave.length() > 0 ?
                    wave_file_name.substr(wave_file_name.find_last_of(".") + 1,
                                          wave_file_name.length()) :
                    "OFF";
            data["__SV_DUMP_WAVE__"] = sv_dump_wave;
        }

        void render_signal_tree(
            const std::vector<picker::sv_signal_define> &external_pin,
            const std::vector<picker::sv_signal_define> &internal_signal,
            std::string &signal_tree, nlohmann::json &signal_tree_json)
        {
            // iterate the signal pin and generate the signal tree by TRIE tree
            // get all the signal name and sort them
            std::vector<picker::sv_signal_define> signal_list;
            for (auto pin : external_pin) { signal_list.push_back(pin); }
            for (auto pin : internal_signal) { signal_list.push_back(pin); }

            // sort the signal list for handle the signal tree
            std::sort(
                signal_list.begin(), signal_list.end(),
                [](auto &a, auto &b) { return a.logic_pin < b.logic_pin; });

            // split the string by delimiter, only considering the last one in
            // consecutive delimiters
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
                            tokens.push_back(
                                ss.str().substr(0, ss.str().size() - 1));
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
                std::vector<std::string> signal_split =
                    split(replace_all(signal.logic_pin, ".", "_"), '_');
                TrieNode *current = root;
                for (auto part_name : signal_split) {
                    if (current->children.find(part_name)
                        == current->children.end()) {
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

            // Auto merge words with '_', if one node has only one child, merge
            // them
            std::function<void(TrieNode *)> merge_trie = [&](TrieNode *node) {
                while (node->children.size() == 1) {
                    auto child = node->children.begin();
                    node->part_name += "_" + child->second->part_name;
                    TrieNode *temp = child->second;
                    node->children.clear();
                    for (auto &schild : temp->children) {
                        node->children[schild.first] = schild.second;
                    }
                    if (temp->isEndOfWord) {
                        node->isEndOfWord = true;
                        node->high_bit    = temp->high_bit;
                        node->low_bit     = temp->low_bit;
                        node->pin_type    = temp->pin_type;
                    }
                    delete temp;
                    printf("merge %s\n", node->part_name.c_str());
                }
                for (auto &child : node->children) { merge_trie(child.second); }
            };
            merge_trie(root);

            // convert the TRIE to json
            std::function<void(TrieNode *, nlohmann::json &)> trie_to_json =
                [&](TrieNode *node, nlohmann::json &json) {
                    if (node->isEndOfWord) {
                        json["Pin"]  = node->pin_type;
                        json["High"] = node->high_bit;
                        json["Low"]  = node->low_bit;
                        json["_"]    = true;
                    }
                    for (auto &child : node->children) {
                        if (child.first != child.second->part_name) {
                            node->children.erase(child.first);
                            node->children[child.second->part_name] = child.second;
                        }
                    }
                    for (auto &child : node->children) {
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
                for (auto &child : node->children) {
                    delete_trie(child.second);
                }
                delete node;
            };
            delete_trie(root);
        }

    } // namespace sv
    /// @brief generate system verilog wrapper file contains
    /// __PIN_CONNECT__ , __LOGIC_PIN_DECLARATION__ , __WIRE_PIN_DECLARATION__
    /// __DPI_FUNCTION_EXPORT__ and __DPI_FUNCTION_IMPLEMENT__
    /// @param global_render_data
    /// @param external_pin
    /// @param internal_signal
    void
    gen_sv_param(nlohmann::json &global_render_data,
                 const std::vector<picker::sv_signal_define> &external_pin,
                 const std::vector<picker::sv_signal_define> &internal_signal,
                 nlohmann::json &signal_tree_json,
                 const std::string &wave_file_name,
                 const std::string &simulator)
    {
        std::string pin_connect, logic, wire, dpi_export, dpi_impl, signal_tree;

        sv::render_external_pin(external_pin, pin_connect, logic, wire,
                                dpi_export, dpi_impl);
        sv::render_internal_signal(internal_signal, dpi_export, dpi_impl);
        sv::render_sv_waveform(simulator, wave_file_name, global_render_data);
        sv::render_signal_tree(external_pin, internal_signal, signal_tree,
                               signal_tree_json);

        global_render_data["__LOGIC_PIN_DECLARATION__"]  = logic;
        global_render_data["__WIRE_PIN_DECLARATION__"]   = wire;
        global_render_data["__PIN_CONNECT__"]            = pin_connect;
        global_render_data["__DPI_FUNCTION_EXPORT__"]    = dpi_export;
        global_render_data["__DPI_FUNCTION_IMPLEMENT__"] = dpi_impl;
        global_render_data["__SIGNAL_TREE__"]            = signal_tree;
    }

}} // namespace picker::codegen