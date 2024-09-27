
class adder_trans extends uvm_sequence_item;
  rand bit [7:0] a;
  rand bit [7:0] b;
  rand bit [7:0] sum;
  
  `uvm_object_utils(adder_trans)
  
  function new(string name = "adder_trans");
    super.new(name);
  endfunction
  
  function void display();
    $display("Transaction: a=%d, b=%d, sum=%d", a, b, sum);
  endfunction
endclass

