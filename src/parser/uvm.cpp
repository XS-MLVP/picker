#include "picker.hpp"
#include "parser/sv.hpp"
#include "parser/uvm.hpp"
#include "parser/exprtk.hpp"
#include <climits>
#include <unordered_map>
#include <fstream>

namespace picker { namespace parser {

    /// Mapping from SystemVerilog basic types to (bit_count, byte_count)
    static const std::unordered_map<std::string, std::pair<int, int>> SV_TYPE_MAP = {
        {"byte",     {8,  1}},
        {"shortint", {16, 2}},
        {"int",      {32, 4}},
        {"longint",  {64, 8}}
    };

    /// Parse array declaration and extract bit width
    void handle_array(
        size_t& j,
        const nlohmann::json& module_token,
        uvm_parameter& parameter,
        const std::string& macro_path)
    {
        if (j + 2 >= module_token.size()) {
            PK_FATAL("Unexpected end of tokens while parsing array declaration");
        }

        std::string msb_str = module_token[j + 2][VeribleJson::TEXT].get<std::string>();

        // Check if it's a number (starts with digit)
        if (std::isdigit(msb_str[0])) {
            try {
                int msb_value = std::stoi(msb_str);
                parameter.bit_count = msb_value + 1;
                parameter.byte_count = bits_to_bytes(parameter.bit_count);
                parameter.is_macro = 0;
                parameter.macro_name = "";
            } catch (const std::exception& e) {
                PK_FATAL("Failed to parse array bound '%s': %s", msb_str.c_str(), e.what());
            }
        } else if (msb_str[0] == '`' || std::isalpha(msb_str[0]) || msb_str[0] == '_') {
            parameter.is_macro = 1;
            parameter.macro_name = msb_str;
            if (!macro_path.empty()) {
                PK_DEBUG("Array bound uses macro: %s (resolution not yet implemented)", msb_str.c_str());
            }
        } else {
            PK_FATAL("Unexpected array bound token: %s", msb_str.c_str());
        }

        // Skip to closing bracket ']'
        while (j + 1 < module_token.size() && module_token[j + 1][VeribleJson::TAG] != "]") {
            ++j;
        }

        if (j + 1 >= module_token.size()) {
            PK_FATAL("Missing closing bracket ']' in array declaration");
        }

        if (j + 2 >= module_token.size()) {
            PK_FATAL("Missing parameter name after array declaration");
        }

        j += 2;  // Skip ']' and get parameter name
        parameter.name = module_token[j][VeribleJson::TEXT].get<std::string>();
    }

    /// Parse class definition (name and members) from token stream
    static std::pair<std::string, std::vector<uvm_parameter>> parse_class_definition(
        const nlohmann::json& tokens,
        const std::string& filepath,
        const std::string& macro_path)
    {
        std::string class_name;
        std::vector<uvm_parameter> parameters;

        for (size_t j = 0; j < tokens.size(); j++) {
            const std::string& tag = tokens[j][VeribleJson::TAG].get_ref<const std::string&>();

            // Find class name
            if (tag == "class") {
                if (j + 1 >= tokens.size()) {
                    PK_FATAL("Class keyword found but no class name follows in %s", filepath.c_str());
                }
                class_name = tokens[j + 1][VeribleJson::TEXT].get<std::string>();
                continue;
            }

            // Support both 'rand type' and direct 'type' declarations
            bool has_rand = (tag == "rand");

            if (has_rand || tag == "bit" || tag == "logic" ||
                tag == "byte" || tag == "int" ||
                tag == "shortint" || tag == "longint") {

                // Use default constructor
                uvm_parameter parameter;

                std::string data_type; 
                if (has_rand) {
                     // Ensure bounds before accessing j+1
                     if (j + 1 >= tokens.size()) {
                         PK_FATAL("Unexpected end after 'rand'");
                     }
                     data_type = tokens[++j][VeribleJson::TAG].get<std::string>();
                } else {
                     data_type = tag;
                }

                // Handle basic types (int, shortint, longint, byte)
                auto it = SV_TYPE_MAP.find(data_type);
                if (it != SV_TYPE_MAP.end()) {
                    parameter.bit_count  = it->second.first;
                    parameter.byte_count = it->second.second;
                }

                // Parse array dimensions if present
                if (j + 1 < tokens.size() && tokens[j + 1][VeribleJson::TAG] == "[") {
                    handle_array(j, tokens, parameter, macro_path);
                } else {
                    // No array, just a simple variable
                    if (j + 1 >= tokens.size()) {
                        PK_FATAL("Missing parameter name after type declaration");
                    }
                    parameter.name = tokens[++j][VeribleJson::TEXT].get<std::string>();
                }

                parameters.push_back(parameter);
            }
        }

        if (class_name.empty()) {
            PK_FATAL("No class definition found in %s", filepath.c_str());
        }

        return {class_name, parameters};
    }

    /// Parse SystemVerilog transaction class file (runs verible and parses result)
    uvm_transaction_define parse_sv(
        const std::string& filepath,
        const std::string& macro_path)
    {
        namespace fs = std::filesystem;

        // Generate JSON using verible (reusing export's approach with /tmp/)
        std::string filename = fs::path(filepath).stem().string();
        std::string json_path = "/tmp/" + filename + "_" + std::string(lib_random_hash)
                              + picker::get_node_uuid() + ".json";

        std::string command =
            std::string(appimage::is_running_as_appimage() ? appimage::get_temporary_path() + "/usr/bin/" : "")
            + "verible-verilog-syntax --export_json --printtokens "
            + fs::absolute(filepath).string() + " > " + json_path;

        PK_DEBUG("Running verible command: %s", command.c_str());

        if (std::system(command.c_str()) != 0) {
            PK_FATAL("Failed to parse %s with verible-verilog-syntax. "
                     "Ensure verible-verilog-syntax is installed and the file is valid SystemVerilog.",
                     filename.c_str());
        }

        PK_MESSAGE("Parsed %s.sv successfully", filename.c_str());

        // Read and parse JSON
        auto json_result = read_file(json_path);
        nlohmann::json module_json;
        try {
            module_json = nlohmann::json::parse(json_result);
        } catch (const std::exception& e) {
            std::remove(json_path.c_str());  // Cleanup on error
            PK_FATAL("Failed to parse JSON from verible output: %s", e.what());
        }

        // Cleanup temp file
        std::remove(json_path.c_str());

        uvm_transaction_define transaction;

        // Convert filepath to absolute path to match JSON keys from verible-verilog-syntax
        std::string abs_filepath = fs::absolute(filepath).string();

        if (!module_json.contains(abs_filepath)) {
            PK_FATAL("JSON output does not contain key for file: %s", abs_filepath.c_str());
        }

        auto module_token = module_json[abs_filepath][VeribleJson::TOKENS];

        // Set basic metadata
        transaction.filepath = filepath;
        transaction.version  = version();
        transaction.data_now = fmtnow();

        // Extract class name and members
        auto [class_name, parameters] = parse_class_definition(module_token, filepath, macro_path);
        transaction.name = class_name;
        transaction.parameters = parameters;

        return transaction;
    }


    /// Convert transaction definition to enriched JSON with metadata
    std::pair<inja::json, int> transaction_to_json(
        const uvm_transaction_define& trans,
        const std::string& trans_filename,
        bool is_multi_transaction)
    {
        bool is_rtl = trans.filepath.empty();

        inja::json trans_data;
        trans_data["name"] = trans.name;
        trans_data["filename"] = trans_filename;
        trans_data[TemplateVars::FILEPATH] = is_rtl ? trans.name + "_trans.sv" : trans.filepath;
        trans_data[TemplateVars::CLASS_NAME] = is_rtl ? trans.name + "_trans" : trans.name;
        trans_data[TemplateVars::FROM_RTL] = is_rtl;
        trans_data[TemplateVars::VARIABLES] = inja::json::array();

        int byte_offset = 0;
        int total_bytes = 0;

        // Convert each parameter to enriched field data
        for (const auto& param : trans.parameters) {
            inja::json field;
            field["name"] = param.name;
            field["byte_count"] = param.byte_count;
            field["bit_count"] = param.bit_count;
            field["macro"] = param.is_macro;
            field["macro_name"] = param.macro_name;

            // Add transaction qualifier for multi-transaction mode
            if (is_multi_transaction) {
                field["transaction_name"] = trans.name;
            }

            // Compute serialization metadata
            field["byte_offset"] = byte_offset;
            switch(param.byte_count) {
                case 1: field["struct_fmt"] = "B"; field["is_standard_aligned"] = true; break;
                case 2: field["struct_fmt"] = "H"; field["is_standard_aligned"] = true; break;
                case 4: field["struct_fmt"] = "I"; field["is_standard_aligned"] = true; break;
                case 8: field["struct_fmt"] = "Q"; field["is_standard_aligned"] = true; break;
                default: field["struct_fmt"] = ""; field["is_standard_aligned"] = false; break;
            }
            byte_offset += param.byte_count;

            trans_data[TemplateVars::VARIABLES].push_back(field);
            total_bytes += param.byte_count;
        }

        trans_data[TemplateVars::BYTE_STREAM_COUNT] = total_bytes;
        return {trans_data, total_bytes};
    }

    /// Enrich template data with shortcuts for single-transaction mode
    static void enrich_single_transaction_template_data(inja::json& data, const uvm_transaction_define& trans)
    {
        if (trans.filepath.empty()) {
            // RTL-generated transaction
            data[TemplateVars::FILEPATH] = trans.name + "_trans.sv";
            data[TemplateVars::MODULE_NAME] = trans.name;
            data[TemplateVars::TRANS_CLASS_NAME] = trans.name + "_trans";
        } else {
            // User-provided transaction
            data[TemplateVars::FILEPATH] = trans.filepath;
            data[TemplateVars::MODULE_NAME] = trans.name;
            data[TemplateVars::TRANS_CLASS_NAME] = trans.name;
        }

        // Add template shortcuts (so templates can use both data.variables and data.transactions[0].variables)
        data["trans"] = data[TemplateVars::TRANSACTIONS][0];
        data["variables"] = data[TemplateVars::TRANSACTIONS][0]["variables"];
        data["trans_class_name"] = data[TemplateVars::TRANS_CLASS_NAME];
        data["byte_stream_count"] = data[TemplateVars::BYTE_STREAM_COUNT];
    }

    /// Prepare complete UVM package template data
    inja::json prepare_uvm_package_data(
        const std::vector<uvm_transaction_define>& transactions,
        const std::vector<std::string>& filenames,
        const std::string& package_name,
        bool generate_dut,
        const std::string& rtl_file_path)
    {
        inja::json data;
        // Basic package info
        data[TemplateVars::PACKAGE_NAME] = package_name;
        data[TemplateVars::VERSION] = transactions[0].version;
        data[TemplateVars::DATE_NOW] = transactions[0].data_now;
        data[TemplateVars::USE_TYPE] = 1;
        data[TemplateVars::GENERATE_DUT] = generate_dut;

        // Setup RTL file path
        if (!rtl_file_path.empty()) {
            std::filesystem::path rtl_abs = std::filesystem::absolute(rtl_file_path);
            std::filesystem::path uvmpy_abs = std::filesystem::absolute("uvmpy");
            std::filesystem::path rtl_relative = std::filesystem::relative(rtl_abs, uvmpy_abs);
            data[TemplateVars::RTL_FILE_PATH] = rtl_relative.string();
        } else {
            data[TemplateVars::RTL_FILE_PATH] = "";
        }

        // Initialize arrays
        data[TemplateVars::TRANSACTIONS] = inja::json::array();
        data[TemplateVars::VARIABLES] = inja::json::array();

        // Process all transactions (Unified Loop)
        int total_byte_count = 0;
        for (size_t i = 0; i < transactions.size(); i++) {
            // Always treat as part of a list for the codegen logic
            auto [trans_data, trans_bytes] = transaction_to_json(
                transactions[i],
                filenames[i],
                true // Always use multi-transaction style metadata
            );

            for (const auto& var : trans_data[TemplateVars::VARIABLES]) {
                data[TemplateVars::VARIABLES].push_back(var);
            }

            data[TemplateVars::TRANSACTIONS].push_back(trans_data);
            total_byte_count += trans_bytes;
        }

        data[TemplateVars::BYTE_STREAM_COUNT] = total_byte_count;
        data[TemplateVars::TRANSACTION_COUNT] = transactions.size();

        // Add template shortcuts for single-transaction mode
        if (transactions.size() == 1) {
            enrich_single_transaction_template_data(data, transactions[0]);
        }

        return data;
    }

    /// Convert a single RTL port definition to UVM transaction parameter
    uvm_parameter sv_signal_to_uvm_parameter(const sv_signal_define &signal)
    {
        uvm_parameter param;
        param.name = signal.logic_pin;
        param.is_macro = 0;
        param.macro_name = "";
        param.current_index = "0";

        // Calculate bit width
        if (signal.logic_pin_hb == -1) {
            // Single-bit signal
            param.bit_count = 1;
            param.byte_count = 1;
        } else {
            // Multi-bit signal: [hb:lb]
            param.bit_count = signal.logic_pin_hb - signal.logic_pin_lb + 1;
            param.byte_count = bits_to_bytes(param.bit_count);
        }

        return param;
    }

    /// Convert RTL module definition to UVM transaction definition
    uvm_transaction_define sv_module_to_uvm_transaction(const sv_module_define &module)
    {
        uvm_transaction_define transaction;

        // Module name becomes transaction name prefix
        // For RTL-generated transactions: Adder -> Adder_trans
        transaction.name = module.module_name;
        transaction.version = picker::version();
        transaction.data_now = picker::fmtnow();

        // Empty filepath indicates RTL-generated transaction
        // Will be set to {module_name}_trans.sv by codegen
        transaction.filepath = "";

        // Convert all module ports to transaction parameters
        transaction.parameters.reserve(module.pins.size());
        for (const auto &signal : module.pins) {
            transaction.parameters.push_back(sv_signal_to_uvm_parameter(signal));
        }

        return transaction;
    }

    /// Parse RTL file and convert to UVM transaction
    uvm_transaction_define parse_rtl_file(
        const std::string& rtl_file_path, 
        std::string& module_name,
        const std::string& target_module_name)
    {
        PK_MESSAGE("RTL mode: generating transaction from %s", rtl_file_path.c_str());

        // Prepare export_opts for RTL parsing (reuse export's parser::sv)
        picker::export_opts rtl_opts;
        rtl_opts.file.push_back(rtl_file_path);
        
        // If target_module_name is provided, use it to filter during RTL parsing
        if (!target_module_name.empty()) {
            rtl_opts.source_module_name_list.push_back(target_module_name);
        }

        // Parse RTL file using export's sv parser (it will handle module search and errors)
        std::vector<picker::sv_module_define> sv_module_result;
        try {
            sv(rtl_opts, sv_module_result);
        } catch (const std::exception &e) {
            PK_FATAL("Failed to parse RTL file: %s", e.what());
        }

        // sv() function ensures sv_module_result is not empty if target was specified
        const auto &target_module = sv_module_result[0];
        module_name = target_module.module_name;

        PK_MESSAGE("Using module: %s with %d ports",
                   target_module.module_name.c_str(), (int)target_module.pins.size());

        if (target_module.pins.empty()) {
            PK_MESSAGE("Warning: Module %s has no ports", target_module.module_name.c_str());
        }

        // Convert RTL to transaction definition
        return sv_module_to_uvm_transaction(target_module);
    }

}} // namespace picker::parser
