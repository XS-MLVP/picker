package AggregateTypes;
    typedef struct packed {
        logic signed [0:4] tag;
        bit valid;
    } header_t;

    typedef struct {
        header_t header;
        logic [64:0] payload [3:2];
        logic flag;
    } entry_t;

    typedef struct {
        logic enabled;
        entry_t entries [-1:0];
    } record_t;
endpackage

module AggregatePorts (
    input AggregateTypes::header_t packed_in,
    output AggregateTypes::header_t packed_out,
    input logic signed [1:0][-2:1] matrix,
    output logic signed [1:0][-2:1] matrix_out,
    input var AggregateTypes::record_t record_in,
    output var AggregateTypes::record_t record_out,
    input AggregateTypes::header_t packed_records [2:1],
    output AggregateTypes::header_t packed_records_out [2:1]
);
    assign packed_out = packed_in;
    assign matrix_out = matrix;
    assign record_out = record_in;
    assign packed_records_out = packed_records;
endmodule
