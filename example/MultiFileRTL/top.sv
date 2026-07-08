`include "outer.svh"

module MultiFileRTLTop(
    input logic [`MULTI_FILE_RTL_OUT_W-1:0] a,
    output logic [`MULTI_FILE_RTL_OUT_W-1:0] b
);
    MultiFileRTLLeaf #(
        .WIDTH(`MULTI_FILE_RTL_OUT_W)
    ) u_leaf (
        .a(a),
        .b(b)
    );
endmodule
