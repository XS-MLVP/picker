// ALU Operation Transaction
// Defines the operation to be performed by the ALU

class alu_op extends uvm_sequence_item;
    `uvm_object_utils(alu_op)
    
    // Operation code (0=ADD, 1=SUB, 2=MUL, 3=DIV)
    rand bit [1:0] opcode;
    
    // Operands
    rand bit [7:0] operand_a;
    rand bit [7:0] operand_b;
    
    function new(string name = "alu_op");
        super.new(name);
    endfunction
    
    function string convert2string();
        return $sformatf("ALU_OP[opcode=%0d, a=%0d, b=%0d]", opcode, operand_a, operand_b);
    endfunction
endclass
