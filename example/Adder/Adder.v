// A verilog 128-bit full adder with carry in and carry out

module Adder #(
    parameter WIDTH = 128
) (
    input [WIDTH-1:0] a,
    input [WIDTH-1:0] b,
    input cin,
    output [WIDTH-1:0] sum,
    output cout
);

assign {cout, sum}  = a + b + cin;

endmodule

module Adder_128 (
    input [127:0] a,
    input [127:0] b,
    input cin,
    output [127:0] sum,
    output cout
);

  Adder #(
      .WIDTH(128)
  ) adder (
      .a(a),
      .b(b),
      .cin(cin),
      .sum(sum),
      .cout(cout)
  );

endmodule