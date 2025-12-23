// ALU Result Transaction
// Captures the result of ALU operation

class alu_result extends uvm_sequence_item;
    `uvm_object_utils(alu_result)
    
    // Result value
    bit [15:0] result;
    
    // Status flags (bit 0: overflow, bit 1: zero, bit 2: negative)
    bit [2:0] status;
    
    function new(string name = "alu_result");
        super.new(name);
    endfunction
    
    function string convert2string();
        return $sformatf("ALU_RESULT[result=%0d, status=0x%0h]", result, status);
    endfunction
endclass
