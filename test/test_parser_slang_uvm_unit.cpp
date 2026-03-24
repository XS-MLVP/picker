#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "parser/uvm.hpp"

namespace {

    void write_text(const std::filesystem::path &path, const std::string &text)
    {
        std::ofstream stream(path, std::ios::trunc);
        stream << text;
    }

    const picker::uvm_transaction_define &find_transaction(const std::vector<picker::uvm_transaction_define> &transactions,
                                                           const std::string &name)
    {
        for (const auto &transaction : transactions) {
            if (transaction.name == name) { return transaction; }
        }
        assert(false && "transaction not found");
        return transactions.front();
    }

    const picker::uvm_parameter &find_parameter(const picker::uvm_transaction_define &transaction,
                                                const std::string &name)
    {
        for (const auto &param : transaction.parameters) {
            if (param.name == name) { return param; }
        }
        assert(false && "parameter not found");
        return transaction.parameters.front();
    }

    void test_single_transaction_supports_grouped_fields_and_integer_types()
    {
        namespace fs = std::filesystem;
        const fs::path base = fs::temp_directory_path() / "picker_test_slang_uvm_single";
        fs::remove_all(base);
        fs::create_directories(base);

        write_text(base / "adder_trans.sv",
                   "class adder_trans extends uvm_sequence_item;\n"
                   "    rand bit [7:0] a, b;\n"
                   "    bit scalar;\n"
                   "    logic [2:0] flags;\n"
                   "    byte byte_v;\n"
                   "    shortint short_v;\n"
                   "    int int_v;\n"
                   "    longint long_v;\n"
                   "    `uvm_object_utils_begin(adder_trans)\n"
                   "        `uvm_field_int(a, UVM_ALL_ON)\n"
                   "        `uvm_field_int(b, UVM_ALL_ON)\n"
                   "    `uvm_object_utils_end\n"
                   "    function new(string name = \"adder_trans\");\n"
                   "        super.new(name);\n"
                   "    endfunction\n"
                   "endclass\n");

        auto transaction = picker::parser::parse_sv((base / "adder_trans").string(), "");

        assert(transaction.name == "adder_trans");
        assert(transaction.parameters.size() == 8);

        assert(find_parameter(transaction, "a").bit_count == 8);
        assert(find_parameter(transaction, "b").bit_count == 8);
        assert(find_parameter(transaction, "scalar").bit_count == 1);
        assert(find_parameter(transaction, "flags").bit_count == 3);
        assert(find_parameter(transaction, "byte_v").byte_count == 1);
        assert(find_parameter(transaction, "short_v").byte_count == 2);
        assert(find_parameter(transaction, "int_v").byte_count == 4);
        assert(find_parameter(transaction, "long_v").byte_count == 8);
        assert(find_parameter(transaction, "a").is_macro == 0);

        fs::remove_all(base);
    }

    void test_filelist_relative_include_define_and_multi_transaction()
    {
        namespace fs = std::filesystem;
        const fs::path base = fs::temp_directory_path() / "picker_test_slang_uvm_filelist";
        fs::remove_all(base);
        fs::create_directories(base / "list");
        fs::create_directories(base / "rtl" / "inc");

        write_text(base / "rtl" / "inc" / "defs.svh", "`define PAYLOAD_W ($clog2(33) + 4)\n");
        write_text(base / "rtl" / "op_trans.sv",
                   "`include \"inc/defs.svh\"\n"
                   "class op_trans extends uvm_sequence_item;\n"
                   "    rand bit [`PAYLOAD_W-1:0] payload;\n"
                   "    function new(string name = \"op_trans\");\n"
                   "        super.new(name);\n"
                   "    endfunction\n"
                   "endclass\n");
        write_text(base / "rtl" / "result_trans.sv",
                   "class result_trans extends uvm_sequence_item;\n"
                   "    bit [15:0] data;\n"
                   "    longint stamp;\n"
                   "    function new(string name = \"result_trans\");\n"
                   "        super.new(name);\n"
                   "    endfunction\n"
                   "endclass\n");
        write_text(base / "list" / "design.f",
                   "+incdir+../rtl/inc\n"
                   "../rtl/op_trans.sv\n"
                   "../rtl/result_trans.sv\n");

        picker::pack_opts opts {};
        opts.filelist = {(base / "list" / "design.f").string()};

        std::vector<picker::uvm_transaction_define> transactions;
        picker::parser::parse_sv_transactions(opts, transactions);

        assert(transactions.size() == 2);

        const auto &op = find_transaction(transactions, "op_trans");
        assert(op.parameters.size() == 1);
        assert(find_parameter(op, "payload").bit_count == 10);
        assert(find_parameter(op, "payload").byte_count == 2);

        const auto &result = find_transaction(transactions, "result_trans");
        assert(result.parameters.size() == 2);
        assert(find_parameter(result, "data").bit_count == 16);
        assert(find_parameter(result, "stamp").byte_count == 8);

        fs::remove_all(base);
    }

} // namespace

int main()
{
    test_single_transaction_supports_grouped_fields_and_integer_types();
    test_filelist_relative_include_define_and_multi_transaction();
    return 0;
}
