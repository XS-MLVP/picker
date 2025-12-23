#pragma once

#include "picker.hpp"
#include <vector>
#include <string>

namespace picker { namespace parser {

    // Template variable keys for JSON data
    namespace TemplateVars {
        constexpr const char* PACKAGE_NAME = "package_name";
        constexpr const char* CLASS_NAME = "class_name";
        constexpr const char* TRANS_CLASS_NAME = "trans_class_name";
        constexpr const char* MODULE_NAME = "module_name";
        constexpr const char* DATE_NOW = "date_now";
        constexpr const char* VERSION = "version";
        constexpr const char* FILEPATH = "filepath";
        constexpr const char* VARIABLES = "variables";
        constexpr const char* TRANSACTIONS = "transactions";
        constexpr const char* BYTE_STREAM_COUNT = "byte_stream_count";
        constexpr const char* TRANSACTION_COUNT = "transaction_count";
        constexpr const char* USE_TYPE = "use_type";
        constexpr const char* GENERATE_DUT = "generate_dut";
        constexpr const char* RTL_FILE_PATH = "rtl_file_path";
        constexpr const char* FROM_RTL = "from_rtl";
    }

    // Verible JSON output keys
    namespace VeribleJson {
        constexpr const char* TAG = "tag";
        constexpr const char* TEXT = "text";
        constexpr const char* TOKENS = "tokens";
    }

    // Convert bit count to byte count
    inline int bits_to_bytes(int bits) {
        return (bits + 7) / 8;
    }

    // Parse SV transaction class using Verible
    uvm_transaction_define parse_sv(const std::string& filepath, const std::string& macro_path);

    // RTL to UVM Transaction conversion
    uvm_transaction_define parse_rtl_file(
        const std::string& rtl_file_path,
        std::string& module_name,
        const std::string& target_module_name = "");

    uvm_transaction_define sv_module_to_uvm_transaction(const sv_module_define &module);
    uvm_parameter sv_signal_to_uvm_parameter(const sv_signal_define &signal);
    std::vector<uvm_parameter> sv_signals_to_uvm_parameters(const std::vector<sv_signal_define> &signals);

    // Template Data Preparation
    void enrich_field_metadata(inja::json& field, int& byte_offset);

    std::pair<inja::json, int> transaction_to_json(
        const uvm_transaction_define& trans,
        const std::string& trans_filename,
        bool is_multi_transaction);

    inja::json prepare_uvm_package_data(
        const std::vector<uvm_transaction_define>& transactions,
        const std::vector<std::string>& filenames,
        const std::string& package_name,
        bool generate_dut,
        const std::string& rtl_file_path = "");

    // Unified entry point for UVM package data preparation
    inja::json parse_and_prepare_package(const pack_opts& opts);

}; } // namespace picker::parser
