
class adder_trans extends uvm_sequence_item;
  rand bit [7:0] a;
  rand bit [7:0] b;
  
  `uvm_object_utils_begin(adder_trans)
    `uvm_field_int(a,UVM_ALL_ON)
    `uvm_field_int(b,UVM_ALL_ON)
  `uvm_component_utils_end

  function new(string name = "adder_trans");
    super.new(name);
  endfunction
  
  function void display();
    $display("Transaction: a=%d, b=%d", a, b);
  endfunction
endclass

