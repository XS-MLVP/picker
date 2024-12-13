
module multi_clock (
    input wire clk1,
    input wire clk2,
    input wire clk3,
    input wire clk4,
    input wire clk5,
    input wire clk6,
    output reg [31:0] reg1,
    output reg [31:0] reg2,
    output reg [31:0] reg3,
    output reg [31:0] reg4,
    output reg [31:0] reg5,
    output reg [31:0] reg6
);
    initial begin
        reg1 = 32'b0;
        reg2 = 32'b0;
        reg3 = 32'b0;
        reg4 = 32'b0;
        reg5 = 32'b0;
        reg6 = 32'b0;
    end
    always @(posedge clk1) begin
        reg1 <= reg1 + 1;
    end
    always @(posedge clk2) begin
        reg2 <= reg2 + 1;
    end
    always @(posedge clk3) begin
        reg3 <= reg3 + 1;
    end
    always @(posedge clk4) begin
        reg4 <= reg4 + 1;
    end
    always @(posedge clk5) begin
        reg5 <= reg5 + 1;
    end
    always @(posedge clk6) begin
        reg6 <= reg6 + 1;
    end
endmodule
