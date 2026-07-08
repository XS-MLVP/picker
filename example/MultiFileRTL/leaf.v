module MultiFileRTLLeaf #(
    parameter WIDTH = 8
) (
    input  [WIDTH-1:0] a,
    output [WIDTH-1:0] b
);
    assign b = a;
endmodule
