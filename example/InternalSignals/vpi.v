module vpi(input wire clk,
           output wire v1,
           output wire [7:0] v2,
           output wire [31:0] v3,
           output wire [63:0] v4
           );
    reg _v1;
    reg [7:0] _v2;
    reg [31:0] _v3;
    reg [63:0] _v4;
    reg _v1_base;
    reg [7:0] _v2_base;
    reg [31:0] _v3_base;
    reg [63:0] _v4_base;
    assign v1 = _v1;
    assign v2 = _v2;
    assign v3 = _v3;
    assign v4 = _v4;
    always @(posedge clk) begin
        _v1 <= _v1 + 1 + _v1_base;
        _v2 <= _v2 + 1 + _v2_base;
        _v3 <= _v3 + 1 + _v3_base;
        _v4 <= _v4 + 1 + _v4_base;
    end
    initial begin
        _v1 = 1'b0;
        _v2 = 8'b0;
        _v3 = 32'b0;
        _v4 = 64'b0;
        _v2_base = 8'b0;
        _v3_base = 32'b0;
        _v4_base = 64'b0;
    end
endmodule
