// Transaction for Adder DUT
// 128-bit adder with carry in/out

class adder_trans extends uvm_sequence_item;
    // Input signals
    rand bit [127:0] a;
    rand bit [127:0] b;
    rand bit cin;
    
    // Output signals
    bit [127:0] sum;
    bit cout;
    
    `uvm_object_utils_begin(adder_trans)
        `uvm_field_int(a, UVM_ALL_ON)
        `uvm_field_int(b, UVM_ALL_ON)
        `uvm_field_int(cin, UVM_ALL_ON)
        `uvm_field_int(sum, UVM_ALL_ON)
        `uvm_field_int(cout, UVM_ALL_ON)
    `uvm_object_utils_end
    
    function new(string name = "adder_trans");
        super.new(name);
    endfunction
endclass
