#include "picker.hpp"
#include "parser/sv.hpp"
#include "parser/slang_uvm.hpp"
#include "parser/uvm.hpp"

namespace picker { namespace parser {

    uvm_transaction_define parse_sv(
        const std::string& filepath,
        const std::string& macro_path)
    {
        (void)macro_path;

        picker::pack_opts opts {};
        opts.files.push_back(filepath);

        std::vector<uvm_transaction_define> transactions;
        parse_sv_transactions(opts, transactions);
        if (transactions.size() != 1) {
            PK_FATAL("Expected exactly one transaction in %s", filepath.c_str());
        }
        return transactions.front();
    }

    void parse_sv_transactions(const pack_opts& opts, std::vector<uvm_transaction_define>& transactions)
    {
        parse_sv_transactions_with_slang(opts, transactions);
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
