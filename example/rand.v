module rand_num_generator
#(
    parameter N = 3
)

(
    input wire clk, reset, 
    output wire [N:0] q
);

reg [N:0] r_reg;
wire [N:0] r_next;
wire feedback_value;
                        
always @(posedge clk, posedge reset)
begin 
    if (reset)
        begin
        // set initial value to 1
        r_reg <= 1;  // use this or uncomment below two line
        
//      r_reg[0] <= 1'b1; // 0th bit = 1
//      r_reg[N:1] <= 0;  // other bits are zero
        
        
        end     
    else if (clk == 1'b1)
        r_reg <= r_next;
end

//// N = 3
//// Feedback polynomial : x^3 + x^2 + 1
////total sequences (maximum) : 2^3 - 1 = 7
assign feedback_value = r_reg[3] ^ r_reg[2] ^ r_reg[0];

//// N = 4
//assign feedback_value = r_reg[4] ^ r_reg[3] ^ r_reg[0];

// N = 5, maximum length = 28 (not 31)
//assign feedback_value = r_reg[5] ^ r_reg[3] ^ r_reg[0];

//// N = 9
//assign feedback_value = r_reg[9] ^ r_reg[5] ^ r_reg[0];


assign r_next = {feedback_value, r_reg[N:1]};
assign q = r_reg;
endmodule

