module dpi_dollar(
  input logic clock,
  input logic reset,
  input logic in$1,
  output logic out$1
);
  logic sig$1;

  assign out$1 = in$1;

  initial begin
    sig$1 = 1'b1;
  end
endmodule
