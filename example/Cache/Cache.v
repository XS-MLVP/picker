module CacheStage1(
  output        io_in_ready,
  input         io_in_valid,
  input  [31:0] io_in_bits_addr,
  input  [2:0]  io_in_bits_size,
  input  [3:0]  io_in_bits_cmd,
  input  [7:0]  io_in_bits_wmask,
  input  [63:0] io_in_bits_wdata,
  input  [15:0] io_in_bits_user,
  input         io_out_ready,
  output        io_out_valid,
  output [31:0] io_out_bits_req_addr,
  output [2:0]  io_out_bits_req_size,
  output [3:0]  io_out_bits_req_cmd,
  output [7:0]  io_out_bits_req_wmask,
  output [63:0] io_out_bits_req_wdata,
  output [15:0] io_out_bits_req_user,
  input         io_metaReadBus_req_ready,
  output        io_metaReadBus_req_valid,
  output [6:0]  io_metaReadBus_req_bits_setIdx,
  input  [18:0] io_metaReadBus_resp_data_0_tag,
  input         io_metaReadBus_resp_data_0_valid,
  input         io_metaReadBus_resp_data_0_dirty,
  input  [18:0] io_metaReadBus_resp_data_1_tag,
  input         io_metaReadBus_resp_data_1_valid,
  input         io_metaReadBus_resp_data_1_dirty,
  input  [18:0] io_metaReadBus_resp_data_2_tag,
  input         io_metaReadBus_resp_data_2_valid,
  input         io_metaReadBus_resp_data_2_dirty,
  input  [18:0] io_metaReadBus_resp_data_3_tag,
  input         io_metaReadBus_resp_data_3_valid,
  input         io_metaReadBus_resp_data_3_dirty,
  input         io_dataReadBus_req_ready,
  output        io_dataReadBus_req_valid,
  output [9:0]  io_dataReadBus_req_bits_setIdx,
  input  [63:0] io_dataReadBus_resp_data_0_data,
  input  [63:0] io_dataReadBus_resp_data_1_data,
  input  [63:0] io_dataReadBus_resp_data_2_data,
  input  [63:0] io_dataReadBus_resp_data_3_data
);
  wire  _io_in_ready_T_1 = io_out_ready & io_out_valid; // @[Decoupled.scala 51:35]
  assign io_in_ready = (~io_in_valid | _io_in_ready_T_1) & io_metaReadBus_req_ready & io_dataReadBus_req_ready; // @[Cache.scala 145:76]
  assign io_out_valid = io_in_valid & io_metaReadBus_req_ready & io_dataReadBus_req_ready; // @[Cache.scala 144:59]
  assign io_out_bits_req_addr = io_in_bits_addr; // @[Cache.scala 143:19]
  assign io_out_bits_req_size = io_in_bits_size; // @[Cache.scala 143:19]
  assign io_out_bits_req_cmd = io_in_bits_cmd; // @[Cache.scala 143:19]
  assign io_out_bits_req_wmask = io_in_bits_wmask; // @[Cache.scala 143:19]
  assign io_out_bits_req_wdata = io_in_bits_wdata; // @[Cache.scala 143:19]
  assign io_out_bits_req_user = io_in_bits_user; // @[Cache.scala 143:19]
  assign io_metaReadBus_req_valid = io_in_valid & io_out_ready; // @[Cache.scala 139:34]
  assign io_metaReadBus_req_bits_setIdx = io_in_bits_addr[12:6]; // @[Cache.scala 77:45]
  assign io_dataReadBus_req_valid = io_in_valid & io_out_ready; // @[Cache.scala 139:34]
  assign io_dataReadBus_req_bits_setIdx = {io_in_bits_addr[12:6],io_in_bits_addr[5:3]}; // @[Cat.scala 33:92]
endmodule
module CacheStage2(
  input         clock,
  input         reset,
  output        io_in_ready,
  input         io_in_valid,
  input  [31:0] io_in_bits_req_addr,
  input  [2:0]  io_in_bits_req_size,
  input  [3:0]  io_in_bits_req_cmd,
  input  [7:0]  io_in_bits_req_wmask,
  input  [63:0] io_in_bits_req_wdata,
  input  [15:0] io_in_bits_req_user,
  input         io_out_ready,
  output        io_out_valid,
  output [31:0] io_out_bits_req_addr,
  output [2:0]  io_out_bits_req_size,
  output [3:0]  io_out_bits_req_cmd,
  output [7:0]  io_out_bits_req_wmask,
  output [63:0] io_out_bits_req_wdata,
  output [15:0] io_out_bits_req_user,
  output [18:0] io_out_bits_metas_0_tag,
  output        io_out_bits_metas_0_dirty,
  output [18:0] io_out_bits_metas_1_tag,
  output        io_out_bits_metas_1_dirty,
  output [18:0] io_out_bits_metas_2_tag,
  output        io_out_bits_metas_2_dirty,
  output [18:0] io_out_bits_metas_3_tag,
  output        io_out_bits_metas_3_dirty,
  output [63:0] io_out_bits_datas_0_data,
  output [63:0] io_out_bits_datas_1_data,
  output [63:0] io_out_bits_datas_2_data,
  output [63:0] io_out_bits_datas_3_data,
  output        io_out_bits_hit,
  output [3:0]  io_out_bits_waymask,
  output        io_out_bits_mmio,
  output        io_out_bits_isForwardData,
  output [63:0] io_out_bits_forwardData_data_data,
  output [3:0]  io_out_bits_forwardData_waymask,
  input  [18:0] io_metaReadResp_0_tag,
  input         io_metaReadResp_0_valid,
  input         io_metaReadResp_0_dirty,
  input  [18:0] io_metaReadResp_1_tag,
  input         io_metaReadResp_1_valid,
  input         io_metaReadResp_1_dirty,
  input  [18:0] io_metaReadResp_2_tag,
  input         io_metaReadResp_2_valid,
  input         io_metaReadResp_2_dirty,
  input  [18:0] io_metaReadResp_3_tag,
  input         io_metaReadResp_3_valid,
  input         io_metaReadResp_3_dirty,
  input  [63:0] io_dataReadResp_0_data,
  input  [63:0] io_dataReadResp_1_data,
  input  [63:0] io_dataReadResp_2_data,
  input  [63:0] io_dataReadResp_3_data,
  input         io_metaWriteBus_req_valid,
  input  [6:0]  io_metaWriteBus_req_bits_setIdx,
  input  [18:0] io_metaWriteBus_req_bits_data_tag,
  input         io_metaWriteBus_req_bits_data_dirty,
  input  [3:0]  io_metaWriteBus_req_bits_waymask,
  input         io_dataWriteBus_req_valid,
  input  [9:0]  io_dataWriteBus_req_bits_setIdx,
  input  [63:0] io_dataWriteBus_req_bits_data_data,
  input  [3:0]  io_dataWriteBus_req_bits_waymask
);
`ifdef RANDOMIZE_REG_INIT
  reg [31:0] _RAND_0;
  reg [31:0] _RAND_1;
  reg [31:0] _RAND_2;
  reg [31:0] _RAND_3;
  reg [63:0] _RAND_4;
  reg [31:0] _RAND_5;
  reg [63:0] _RAND_6;
  reg [31:0] _RAND_7;
`endif // RANDOMIZE_REG_INIT
  wire [2:0] addr_wordIndex = io_in_bits_req_addr[5:3]; // @[Cache.scala 174:31]
  wire [6:0] addr_index = io_in_bits_req_addr[12:6]; // @[Cache.scala 174:31]
  wire [18:0] addr_tag = io_in_bits_req_addr[31:13]; // @[Cache.scala 174:31]
  wire  isForwardMeta = io_in_valid & io_metaWriteBus_req_valid & io_metaWriteBus_req_bits_setIdx == addr_index; // @[Cache.scala 176:64]
  reg  isForwardMetaReg; // @[Cache.scala 177:33]
  wire  _GEN_0 = isForwardMeta | isForwardMetaReg; // @[Cache.scala 178:24 177:33 178:43]
  wire  _T = io_in_ready & io_in_valid; // @[Decoupled.scala 51:35]
  wire  _T_1 = ~io_in_valid; // @[Cache.scala 179:23]
  wire  _T_2 = _T | ~io_in_valid; // @[Cache.scala 179:20]
  reg [18:0] forwardMetaReg_data_tag; // @[Reg.scala 19:16]
  reg  forwardMetaReg_data_dirty; // @[Reg.scala 19:16]
  reg [3:0] forwardMetaReg_waymask; // @[Reg.scala 19:16]
  wire [18:0] _GEN_3 = isForwardMeta ? io_metaWriteBus_req_bits_data_tag : forwardMetaReg_data_tag; // @[Reg.scala 19:16 20:{18,22}]
  wire  _GEN_5 = isForwardMeta ? io_metaWriteBus_req_bits_data_dirty : forwardMetaReg_data_dirty; // @[Reg.scala 19:16 20:{18,22}]
  wire [3:0] _GEN_6 = isForwardMeta ? io_metaWriteBus_req_bits_waymask : forwardMetaReg_waymask; // @[Reg.scala 19:16 20:{18,22}]
  wire  pickForwardMeta = isForwardMetaReg | isForwardMeta; // @[Cache.scala 183:42]
  wire  forwardWaymask_0 = _GEN_6[0]; // @[Cache.scala 185:61]
  wire  forwardWaymask_1 = _GEN_6[1]; // @[Cache.scala 185:61]
  wire  forwardWaymask_2 = _GEN_6[2]; // @[Cache.scala 185:61]
  wire  forwardWaymask_3 = _GEN_6[3]; // @[Cache.scala 185:61]
  wire [18:0] metaWay_0_tag = pickForwardMeta & forwardWaymask_0 ? _GEN_3 : io_metaReadResp_0_tag; // @[Cache.scala 187:22]
  wire  metaWay_0_valid = pickForwardMeta & forwardWaymask_0 | io_metaReadResp_0_valid; // @[Cache.scala 187:22]
  wire [18:0] metaWay_1_tag = pickForwardMeta & forwardWaymask_1 ? _GEN_3 : io_metaReadResp_1_tag; // @[Cache.scala 187:22]
  wire  metaWay_1_valid = pickForwardMeta & forwardWaymask_1 | io_metaReadResp_1_valid; // @[Cache.scala 187:22]
  wire [18:0] metaWay_2_tag = pickForwardMeta & forwardWaymask_2 ? _GEN_3 : io_metaReadResp_2_tag; // @[Cache.scala 187:22]
  wire  metaWay_2_valid = pickForwardMeta & forwardWaymask_2 | io_metaReadResp_2_valid; // @[Cache.scala 187:22]
  wire [18:0] metaWay_3_tag = pickForwardMeta & forwardWaymask_3 ? _GEN_3 : io_metaReadResp_3_tag; // @[Cache.scala 187:22]
  wire  metaWay_3_valid = pickForwardMeta & forwardWaymask_3 | io_metaReadResp_3_valid; // @[Cache.scala 187:22]
  wire  _hitVec_T_2 = metaWay_0_valid & metaWay_0_tag == addr_tag & io_in_valid; // @[Cache.scala 190:73]
  wire  _hitVec_T_5 = metaWay_1_valid & metaWay_1_tag == addr_tag & io_in_valid; // @[Cache.scala 190:73]
  wire  _hitVec_T_8 = metaWay_2_valid & metaWay_2_tag == addr_tag & io_in_valid; // @[Cache.scala 190:73]
  wire  _hitVec_T_11 = metaWay_3_valid & metaWay_3_tag == addr_tag & io_in_valid; // @[Cache.scala 190:73]
  wire [3:0] hitVec = {_hitVec_T_11,_hitVec_T_8,_hitVec_T_5,_hitVec_T_2}; // @[Cache.scala 190:90]
  reg [63:0] victimWaymask_lfsr; // @[LFSR64.scala 25:23]
  wire  victimWaymask_xor = victimWaymask_lfsr[0] ^ victimWaymask_lfsr[1] ^ victimWaymask_lfsr[3] ^ victimWaymask_lfsr[4
    ]; // @[LFSR64.scala 26:43]
  wire [63:0] _victimWaymask_lfsr_T_2 = {victimWaymask_xor,victimWaymask_lfsr[63:1]}; // @[Cat.scala 33:92]
  wire [3:0] victimWaymask = 4'h1 << victimWaymask_lfsr[1:0]; // @[Cache.scala 191:42]
  wire  _invalidVec_T = ~metaWay_0_valid; // @[Cache.scala 193:45]
  wire  _invalidVec_T_1 = ~metaWay_1_valid; // @[Cache.scala 193:45]
  wire  _invalidVec_T_2 = ~metaWay_2_valid; // @[Cache.scala 193:45]
  wire  _invalidVec_T_3 = ~metaWay_3_valid; // @[Cache.scala 193:45]
  wire [3:0] invalidVec = {_invalidVec_T_3,_invalidVec_T_2,_invalidVec_T_1,_invalidVec_T}; // @[Cache.scala 193:56]
  wire  hasInvalidWay = |invalidVec; // @[Cache.scala 194:34]
  wire [1:0] _refillInvalidWaymask_T_3 = invalidVec >= 4'h2 ? 2'h2 : 2'h1; // @[Cache.scala 197:8]
  wire [2:0] _refillInvalidWaymask_T_4 = invalidVec >= 4'h4 ? 3'h4 : {{1'd0}, _refillInvalidWaymask_T_3}; // @[Cache.scala 196:8]
  wire [3:0] refillInvalidWaymask = invalidVec >= 4'h8 ? 4'h8 : {{1'd0}, _refillInvalidWaymask_T_4}; // @[Cache.scala 195:33]
  wire [3:0] _waymask_T = hasInvalidWay ? refillInvalidWaymask : victimWaymask; // @[Cache.scala 200:49]
  wire [3:0] waymask = io_out_bits_hit ? hitVec : _waymask_T; // @[Cache.scala 200:20]
  wire [1:0] _T_7 = waymask[0] + waymask[1]; // @[Bitwise.scala 51:90]
  wire [1:0] _T_9 = waymask[2] + waymask[3]; // @[Bitwise.scala 51:90]
  wire [2:0] _T_11 = _T_7 + _T_9; // @[Bitwise.scala 51:90]
  wire  _T_13 = _T_11 > 3'h1; // @[Cache.scala 201:26]
  wire  _T_16 = ~reset; // @[Debug.scala 56:24]
  wire [31:0] _io_out_bits_mmio_T = io_in_bits_req_addr ^ 32'h30000000; // @[NutCore.scala 86:11]
  wire  _io_out_bits_mmio_T_2 = _io_out_bits_mmio_T[31:28] == 4'h0; // @[NutCore.scala 86:44]
  wire [31:0] _io_out_bits_mmio_T_3 = io_in_bits_req_addr ^ 32'h40000000; // @[NutCore.scala 86:11]
  wire  _io_out_bits_mmio_T_5 = _io_out_bits_mmio_T_3[31:30] == 2'h0; // @[NutCore.scala 86:44]
  wire [9:0] _isForwardData_T_8 = {addr_index,addr_wordIndex}; // @[Cat.scala 33:92]
  wire  _isForwardData_T_10 = io_dataWriteBus_req_valid & io_dataWriteBus_req_bits_setIdx == _isForwardData_T_8; // @[Cache.scala 217:13]
  wire  isForwardData = io_in_valid & _isForwardData_T_10; // @[Cache.scala 216:35]
  reg  isForwardDataReg; // @[Cache.scala 219:33]
  wire  _GEN_8 = isForwardData | isForwardDataReg; // @[Cache.scala 220:24 219:33 220:43]
  reg [63:0] forwardDataReg_data_data; // @[Reg.scala 19:16]
  reg [3:0] forwardDataReg_waymask; // @[Reg.scala 19:16]
  wire  _io_in_ready_T_1 = io_out_ready & io_out_valid; // @[Decoupled.scala 51:35]
  assign io_in_ready = _T_1 | _io_in_ready_T_1; // @[Cache.scala 228:31]
  assign io_out_valid = io_in_valid; // @[Cache.scala 227:16]
  assign io_out_bits_req_addr = io_in_bits_req_addr; // @[Cache.scala 226:19]
  assign io_out_bits_req_size = io_in_bits_req_size; // @[Cache.scala 226:19]
  assign io_out_bits_req_cmd = io_in_bits_req_cmd; // @[Cache.scala 226:19]
  assign io_out_bits_req_wmask = io_in_bits_req_wmask; // @[Cache.scala 226:19]
  assign io_out_bits_req_wdata = io_in_bits_req_wdata; // @[Cache.scala 226:19]
  assign io_out_bits_req_user = io_in_bits_req_user; // @[Cache.scala 226:19]
  assign io_out_bits_metas_0_tag = pickForwardMeta & forwardWaymask_0 ? _GEN_3 : io_metaReadResp_0_tag; // @[Cache.scala 187:22]
  assign io_out_bits_metas_0_dirty = pickForwardMeta & forwardWaymask_0 ? _GEN_5 : io_metaReadResp_0_dirty; // @[Cache.scala 187:22]
  assign io_out_bits_metas_1_tag = pickForwardMeta & forwardWaymask_1 ? _GEN_3 : io_metaReadResp_1_tag; // @[Cache.scala 187:22]
  assign io_out_bits_metas_1_dirty = pickForwardMeta & forwardWaymask_1 ? _GEN_5 : io_metaReadResp_1_dirty; // @[Cache.scala 187:22]
  assign io_out_bits_metas_2_tag = pickForwardMeta & forwardWaymask_2 ? _GEN_3 : io_metaReadResp_2_tag; // @[Cache.scala 187:22]
  assign io_out_bits_metas_2_dirty = pickForwardMeta & forwardWaymask_2 ? _GEN_5 : io_metaReadResp_2_dirty; // @[Cache.scala 187:22]
  assign io_out_bits_metas_3_tag = pickForwardMeta & forwardWaymask_3 ? _GEN_3 : io_metaReadResp_3_tag; // @[Cache.scala 187:22]
  assign io_out_bits_metas_3_dirty = pickForwardMeta & forwardWaymask_3 ? _GEN_5 : io_metaReadResp_3_dirty; // @[Cache.scala 187:22]
  assign io_out_bits_datas_0_data = io_dataReadResp_0_data; // @[Cache.scala 213:21]
  assign io_out_bits_datas_1_data = io_dataReadResp_1_data; // @[Cache.scala 213:21]
  assign io_out_bits_datas_2_data = io_dataReadResp_2_data; // @[Cache.scala 213:21]
  assign io_out_bits_datas_3_data = io_dataReadResp_3_data; // @[Cache.scala 213:21]
  assign io_out_bits_hit = io_in_valid & |hitVec; // @[Cache.scala 211:34]
  assign io_out_bits_waymask = io_out_bits_hit ? hitVec : _waymask_T; // @[Cache.scala 200:20]
  assign io_out_bits_mmio = _io_out_bits_mmio_T_2 | _io_out_bits_mmio_T_5; // @[NutCore.scala 87:15]
  assign io_out_bits_isForwardData = isForwardDataReg | isForwardData; // @[Cache.scala 223:49]
  assign io_out_bits_forwardData_data_data = isForwardData ? io_dataWriteBus_req_bits_data_data :
    forwardDataReg_data_data; // @[Cache.scala 224:33]
  assign io_out_bits_forwardData_waymask = isForwardData ? io_dataWriteBus_req_bits_waymask : forwardDataReg_waymask; // @[Cache.scala 224:33]
  always @(posedge clock) begin
    if (reset) begin // @[Cache.scala 177:33]
      isForwardMetaReg <= 1'h0; // @[Cache.scala 177:33]
    end else if (_T | ~io_in_valid) begin // @[Cache.scala 179:37]
      isForwardMetaReg <= 1'h0; // @[Cache.scala 179:56]
    end else begin
      isForwardMetaReg <= _GEN_0;
    end
    if (isForwardMeta) begin // @[Reg.scala 20:18]
      forwardMetaReg_data_tag <= io_metaWriteBus_req_bits_data_tag; // @[Reg.scala 20:22]
    end
    if (isForwardMeta) begin // @[Reg.scala 20:18]
      forwardMetaReg_data_dirty <= io_metaWriteBus_req_bits_data_dirty; // @[Reg.scala 20:22]
    end
    if (isForwardMeta) begin // @[Reg.scala 20:18]
      forwardMetaReg_waymask <= io_metaWriteBus_req_bits_waymask; // @[Reg.scala 20:22]
    end
    if (reset) begin // @[LFSR64.scala 25:23]
      victimWaymask_lfsr <= 64'h1234567887654321; // @[LFSR64.scala 25:23]
    end else if (victimWaymask_lfsr == 64'h0) begin // @[LFSR64.scala 28:18]
      victimWaymask_lfsr <= 64'h1;
    end else begin
      victimWaymask_lfsr <= _victimWaymask_lfsr_T_2;
    end
    if (reset) begin // @[Cache.scala 219:33]
      isForwardDataReg <= 1'h0; // @[Cache.scala 219:33]
    end else if (_T_2) begin // @[Cache.scala 221:37]
      isForwardDataReg <= 1'h0; // @[Cache.scala 221:56]
    end else begin
      isForwardDataReg <= _GEN_8;
    end
    if (isForwardData) begin // @[Reg.scala 20:18]
      forwardDataReg_data_data <= io_dataWriteBus_req_bits_data_data; // @[Reg.scala 20:22]
    end
    if (isForwardData) begin // @[Reg.scala 20:18]
      forwardDataReg_waymask <= io_dataWriteBus_req_bits_waymask; // @[Reg.scala 20:22]
    end
    `ifndef SYNTHESIS
    `ifdef PRINTF_COND
      if (`PRINTF_COND) begin
    `endif
        if (_T_16 & ~(~(io_in_valid & _T_13))) begin
          $fwrite(32'h80000002,
            "Assertion failed\n    at Cache.scala:208 assert(!(io.in.valid && PopCount(waymask) > 1.U))\n"); // @[Cache.scala 208:9]
        end
    `ifdef PRINTF_COND
      end
    `endif
    `endif // SYNTHESIS
    `ifndef SYNTHESIS
    `ifdef STOP_COND
      if (`STOP_COND) begin
    `endif
        if (~(~(io_in_valid & _T_13)) & _T_16) begin
          $fatal; // @[Cache.scala 208:9]
        end
    `ifdef STOP_COND
      end
    `endif
    `endif // SYNTHESIS
  end
// Register and memory initialization
`ifdef RANDOMIZE_GARBAGE_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_INVALID_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_REG_INIT
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_MEM_INIT
`define RANDOMIZE
`endif
`ifndef RANDOM
`define RANDOM $random
`endif
`ifdef RANDOMIZE_MEM_INIT
  integer initvar;
`endif
`ifndef SYNTHESIS
`ifdef FIRRTL_BEFORE_INITIAL
`FIRRTL_BEFORE_INITIAL
`endif
initial begin
  `ifdef RANDOMIZE
    `ifdef INIT_RANDOM
      `INIT_RANDOM
    `endif
    `ifndef VERILATOR
      `ifdef RANDOMIZE_DELAY
        #`RANDOMIZE_DELAY begin end
      `else
        #0.002 begin end
      `endif
    `endif
`ifdef RANDOMIZE_REG_INIT
  _RAND_0 = {1{`RANDOM}};
  isForwardMetaReg = _RAND_0[0:0];
  _RAND_1 = {1{`RANDOM}};
  forwardMetaReg_data_tag = _RAND_1[18:0];
  _RAND_2 = {1{`RANDOM}};
  forwardMetaReg_data_dirty = _RAND_2[0:0];
  _RAND_3 = {1{`RANDOM}};
  forwardMetaReg_waymask = _RAND_3[3:0];
  _RAND_4 = {2{`RANDOM}};
  victimWaymask_lfsr = _RAND_4[63:0];
  _RAND_5 = {1{`RANDOM}};
  isForwardDataReg = _RAND_5[0:0];
  _RAND_6 = {2{`RANDOM}};
  forwardDataReg_data_data = _RAND_6[63:0];
  _RAND_7 = {1{`RANDOM}};
  forwardDataReg_waymask = _RAND_7[3:0];
`endif // RANDOMIZE_REG_INIT
  `endif // RANDOMIZE
end // initial
`ifdef FIRRTL_AFTER_INITIAL
`FIRRTL_AFTER_INITIAL
`endif
`endif // SYNTHESIS
endmodule
module Arbiter(
  input         io_in_0_valid,
  input  [6:0]  io_in_0_bits_setIdx,
  input  [18:0] io_in_0_bits_data_tag,
  input  [3:0]  io_in_0_bits_waymask,
  input         io_in_1_valid,
  input  [6:0]  io_in_1_bits_setIdx,
  input  [18:0] io_in_1_bits_data_tag,
  input         io_in_1_bits_data_dirty,
  input  [3:0]  io_in_1_bits_waymask,
  output        io_out_valid,
  output [6:0]  io_out_bits_setIdx,
  output [18:0] io_out_bits_data_tag,
  output        io_out_bits_data_dirty,
  output [3:0]  io_out_bits_waymask
);
  wire  grant_1 = ~io_in_0_valid; // @[Arbiter.scala 45:78]
  assign io_out_valid = ~grant_1 | io_in_1_valid; // @[Arbiter.scala 147:31]
  assign io_out_bits_setIdx = io_in_0_valid ? io_in_0_bits_setIdx : io_in_1_bits_setIdx; // @[Arbiter.scala 136:15 138:26 140:19]
  assign io_out_bits_data_tag = io_in_0_valid ? io_in_0_bits_data_tag : io_in_1_bits_data_tag; // @[Arbiter.scala 136:15 138:26 140:19]
  assign io_out_bits_data_dirty = io_in_0_valid | io_in_1_bits_data_dirty; // @[Arbiter.scala 136:15 138:26 140:19]
  assign io_out_bits_waymask = io_in_0_valid ? io_in_0_bits_waymask : io_in_1_bits_waymask; // @[Arbiter.scala 136:15 138:26 140:19]
endmodule
module Arbiter_1(
  input         io_in_0_valid,
  input  [9:0]  io_in_0_bits_setIdx,
  input  [63:0] io_in_0_bits_data_data,
  input  [3:0]  io_in_0_bits_waymask,
  input         io_in_1_valid,
  input  [9:0]  io_in_1_bits_setIdx,
  input  [63:0] io_in_1_bits_data_data,
  input  [3:0]  io_in_1_bits_waymask,
  output        io_out_valid,
  output [9:0]  io_out_bits_setIdx,
  output [63:0] io_out_bits_data_data,
  output [3:0]  io_out_bits_waymask
);
  wire  grant_1 = ~io_in_0_valid; // @[Arbiter.scala 45:78]
  assign io_out_valid = ~grant_1 | io_in_1_valid; // @[Arbiter.scala 147:31]
  assign io_out_bits_setIdx = io_in_0_valid ? io_in_0_bits_setIdx : io_in_1_bits_setIdx; // @[Arbiter.scala 136:15 138:26 140:19]
  assign io_out_bits_data_data = io_in_0_valid ? io_in_0_bits_data_data : io_in_1_bits_data_data; // @[Arbiter.scala 136:15 138:26 140:19]
  assign io_out_bits_waymask = io_in_0_valid ? io_in_0_bits_waymask : io_in_1_bits_waymask; // @[Arbiter.scala 136:15 138:26 140:19]
endmodule
module CacheStage3(
  input         clock,
  input         reset,
  output        io_in_ready,
  input         io_in_valid,
  input  [31:0] io_in_bits_req_addr,
  input  [2:0]  io_in_bits_req_size,
  input  [3:0]  io_in_bits_req_cmd,
  input  [7:0]  io_in_bits_req_wmask,
  input  [63:0] io_in_bits_req_wdata,
  input  [15:0] io_in_bits_req_user,
  input  [18:0] io_in_bits_metas_0_tag,
  input         io_in_bits_metas_0_dirty,
  input  [18:0] io_in_bits_metas_1_tag,
  input         io_in_bits_metas_1_dirty,
  input  [18:0] io_in_bits_metas_2_tag,
  input         io_in_bits_metas_2_dirty,
  input  [18:0] io_in_bits_metas_3_tag,
  input         io_in_bits_metas_3_dirty,
  input  [63:0] io_in_bits_datas_0_data,
  input  [63:0] io_in_bits_datas_1_data,
  input  [63:0] io_in_bits_datas_2_data,
  input  [63:0] io_in_bits_datas_3_data,
  input         io_in_bits_hit,
  input  [3:0]  io_in_bits_waymask,
  input         io_in_bits_mmio,
  input         io_in_bits_isForwardData,
  input  [63:0] io_in_bits_forwardData_data_data,
  input  [3:0]  io_in_bits_forwardData_waymask,
  input         io_out_ready,
  output        io_out_valid,
  output [3:0]  io_out_bits_cmd,
  output [63:0] io_out_bits_rdata,
  output [15:0] io_out_bits_user,
  output        io_isFinish,
  input         io_flush,
  input         io_dataReadBus_req_ready,
  output        io_dataReadBus_req_valid,
  output [9:0]  io_dataReadBus_req_bits_setIdx,
  input  [63:0] io_dataReadBus_resp_data_0_data,
  input  [63:0] io_dataReadBus_resp_data_1_data,
  input  [63:0] io_dataReadBus_resp_data_2_data,
  input  [63:0] io_dataReadBus_resp_data_3_data,
  output        io_dataWriteBus_req_valid,
  output [9:0]  io_dataWriteBus_req_bits_setIdx,
  output [63:0] io_dataWriteBus_req_bits_data_data,
  output [3:0]  io_dataWriteBus_req_bits_waymask,
  output        io_metaWriteBus_req_valid,
  output [6:0]  io_metaWriteBus_req_bits_setIdx,
  output [18:0] io_metaWriteBus_req_bits_data_tag,
  output        io_metaWriteBus_req_bits_data_dirty,
  output [3:0]  io_metaWriteBus_req_bits_waymask,
  input         io_mem_req_ready,
  output        io_mem_req_valid,
  output [31:0] io_mem_req_bits_addr,
  output [3:0]  io_mem_req_bits_cmd,
  output [63:0] io_mem_req_bits_wdata,
  output        io_mem_resp_ready,
  input         io_mem_resp_valid,
  input  [3:0]  io_mem_resp_bits_cmd,
  input  [63:0] io_mem_resp_bits_rdata,
  input         io_mmio_req_ready,
  output        io_mmio_req_valid,
  output [31:0] io_mmio_req_bits_addr,
  output [2:0]  io_mmio_req_bits_size,
  output [3:0]  io_mmio_req_bits_cmd,
  output [7:0]  io_mmio_req_bits_wmask,
  output [63:0] io_mmio_req_bits_wdata,
  output        io_mmio_resp_ready,
  input         io_mmio_resp_valid,
  input  [63:0] io_mmio_resp_bits_rdata,
  input         io_cohResp_ready,
  output        io_cohResp_valid,
  output [3:0]  io_cohResp_bits_cmd,
  output [63:0] io_cohResp_bits_rdata,
  output        io_dataReadRespToL1
);
`ifdef RANDOMIZE_REG_INIT
  reg [31:0] _RAND_0;
  reg [31:0] _RAND_1;
  reg [31:0] _RAND_2;
  reg [31:0] _RAND_3;
  reg [31:0] _RAND_4;
  reg [31:0] _RAND_5;
  reg [63:0] _RAND_6;
  reg [63:0] _RAND_7;
  reg [63:0] _RAND_8;
  reg [63:0] _RAND_9;
  reg [31:0] _RAND_10;
  reg [31:0] _RAND_11;
  reg [63:0] _RAND_12;
  reg [31:0] _RAND_13;
  reg [31:0] _RAND_14;
`endif // RANDOMIZE_REG_INIT
  wire  metaWriteArb_io_in_0_valid; // @[Cache.scala 254:28]
  wire [6:0] metaWriteArb_io_in_0_bits_setIdx; // @[Cache.scala 254:28]
  wire [18:0] metaWriteArb_io_in_0_bits_data_tag; // @[Cache.scala 254:28]
  wire [3:0] metaWriteArb_io_in_0_bits_waymask; // @[Cache.scala 254:28]
  wire  metaWriteArb_io_in_1_valid; // @[Cache.scala 254:28]
  wire [6:0] metaWriteArb_io_in_1_bits_setIdx; // @[Cache.scala 254:28]
  wire [18:0] metaWriteArb_io_in_1_bits_data_tag; // @[Cache.scala 254:28]
  wire  metaWriteArb_io_in_1_bits_data_dirty; // @[Cache.scala 254:28]
  wire [3:0] metaWriteArb_io_in_1_bits_waymask; // @[Cache.scala 254:28]
  wire  metaWriteArb_io_out_valid; // @[Cache.scala 254:28]
  wire [6:0] metaWriteArb_io_out_bits_setIdx; // @[Cache.scala 254:28]
  wire [18:0] metaWriteArb_io_out_bits_data_tag; // @[Cache.scala 254:28]
  wire  metaWriteArb_io_out_bits_data_dirty; // @[Cache.scala 254:28]
  wire [3:0] metaWriteArb_io_out_bits_waymask; // @[Cache.scala 254:28]
  wire  dataWriteArb_io_in_0_valid; // @[Cache.scala 255:28]
  wire [9:0] dataWriteArb_io_in_0_bits_setIdx; // @[Cache.scala 255:28]
  wire [63:0] dataWriteArb_io_in_0_bits_data_data; // @[Cache.scala 255:28]
  wire [3:0] dataWriteArb_io_in_0_bits_waymask; // @[Cache.scala 255:28]
  wire  dataWriteArb_io_in_1_valid; // @[Cache.scala 255:28]
  wire [9:0] dataWriteArb_io_in_1_bits_setIdx; // @[Cache.scala 255:28]
  wire [63:0] dataWriteArb_io_in_1_bits_data_data; // @[Cache.scala 255:28]
  wire [3:0] dataWriteArb_io_in_1_bits_waymask; // @[Cache.scala 255:28]
  wire  dataWriteArb_io_out_valid; // @[Cache.scala 255:28]
  wire [9:0] dataWriteArb_io_out_bits_setIdx; // @[Cache.scala 255:28]
  wire [63:0] dataWriteArb_io_out_bits_data_data; // @[Cache.scala 255:28]
  wire [3:0] dataWriteArb_io_out_bits_waymask; // @[Cache.scala 255:28]
  wire [2:0] addr_wordIndex = io_in_bits_req_addr[5:3]; // @[Cache.scala 258:31]
  wire [6:0] addr_index = io_in_bits_req_addr[12:6]; // @[Cache.scala 258:31]
  wire  mmio = io_in_valid & io_in_bits_mmio; // @[Cache.scala 259:26]
  wire  hit = io_in_valid & io_in_bits_hit; // @[Cache.scala 260:25]
  wire  miss = io_in_valid & ~io_in_bits_hit; // @[Cache.scala 261:26]
  wire  _probe_T_1 = io_in_bits_req_cmd == 4'h8; // @[SimpleBus.scala 79:23]
  wire  probe = io_in_valid & _probe_T_1; // @[Cache.scala 262:39]
  wire  _hitReadBurst_T = io_in_bits_req_cmd == 4'h2; // @[SimpleBus.scala 76:27]
  wire  hitReadBurst = hit & _hitReadBurst_T; // @[Cache.scala 263:26]
  wire  meta_dirty = io_in_bits_waymask[0] & io_in_bits_metas_0_dirty | io_in_bits_waymask[1] & io_in_bits_metas_1_dirty
     | io_in_bits_waymask[2] & io_in_bits_metas_2_dirty | io_in_bits_waymask[3] & io_in_bits_metas_3_dirty; // @[Mux.scala 27:73]
  wire [18:0] _meta_T_18 = io_in_bits_waymask[0] ? io_in_bits_metas_0_tag : 19'h0; // @[Mux.scala 27:73]
  wire [18:0] _meta_T_19 = io_in_bits_waymask[1] ? io_in_bits_metas_1_tag : 19'h0; // @[Mux.scala 27:73]
  wire [18:0] _meta_T_20 = io_in_bits_waymask[2] ? io_in_bits_metas_2_tag : 19'h0; // @[Mux.scala 27:73]
  wire [18:0] _meta_T_21 = io_in_bits_waymask[3] ? io_in_bits_metas_3_tag : 19'h0; // @[Mux.scala 27:73]
  wire [18:0] _meta_T_22 = _meta_T_18 | _meta_T_19; // @[Mux.scala 27:73]
  wire [18:0] _meta_T_23 = _meta_T_22 | _meta_T_20; // @[Mux.scala 27:73]
  wire [18:0] meta_tag = _meta_T_23 | _meta_T_21; // @[Mux.scala 27:73]
  wire  _T_3 = ~reset; // @[Cache.scala 265:9]
  wire  useForwardData = io_in_bits_isForwardData & io_in_bits_waymask == io_in_bits_forwardData_waymask; // @[Cache.scala 273:49]
  wire [63:0] _dataReadArray_T_4 = io_in_bits_waymask[0] ? io_in_bits_datas_0_data : 64'h0; // @[Mux.scala 27:73]
  wire [63:0] _dataReadArray_T_5 = io_in_bits_waymask[1] ? io_in_bits_datas_1_data : 64'h0; // @[Mux.scala 27:73]
  wire [63:0] _dataReadArray_T_6 = io_in_bits_waymask[2] ? io_in_bits_datas_2_data : 64'h0; // @[Mux.scala 27:73]
  wire [63:0] _dataReadArray_T_7 = io_in_bits_waymask[3] ? io_in_bits_datas_3_data : 64'h0; // @[Mux.scala 27:73]
  wire [63:0] _dataReadArray_T_8 = _dataReadArray_T_4 | _dataReadArray_T_5; // @[Mux.scala 27:73]
  wire [63:0] _dataReadArray_T_9 = _dataReadArray_T_8 | _dataReadArray_T_6; // @[Mux.scala 27:73]
  wire [63:0] _dataReadArray_T_10 = _dataReadArray_T_9 | _dataReadArray_T_7; // @[Mux.scala 27:73]
  wire [63:0] dataRead = useForwardData ? io_in_bits_forwardData_data_data : _dataReadArray_T_10; // @[Cache.scala 275:21]
  wire [7:0] _wordMask_T_12 = io_in_bits_req_wmask[0] ? 8'hff : 8'h0; // @[Bitwise.scala 77:12]
  wire [7:0] _wordMask_T_14 = io_in_bits_req_wmask[1] ? 8'hff : 8'h0; // @[Bitwise.scala 77:12]
  wire [7:0] _wordMask_T_16 = io_in_bits_req_wmask[2] ? 8'hff : 8'h0; // @[Bitwise.scala 77:12]
  wire [7:0] _wordMask_T_18 = io_in_bits_req_wmask[3] ? 8'hff : 8'h0; // @[Bitwise.scala 77:12]
  wire [7:0] _wordMask_T_20 = io_in_bits_req_wmask[4] ? 8'hff : 8'h0; // @[Bitwise.scala 77:12]
  wire [7:0] _wordMask_T_22 = io_in_bits_req_wmask[5] ? 8'hff : 8'h0; // @[Bitwise.scala 77:12]
  wire [7:0] _wordMask_T_24 = io_in_bits_req_wmask[6] ? 8'hff : 8'h0; // @[Bitwise.scala 77:12]
  wire [7:0] _wordMask_T_26 = io_in_bits_req_wmask[7] ? 8'hff : 8'h0; // @[Bitwise.scala 77:12]
  wire [63:0] _wordMask_T_27 = {_wordMask_T_26,_wordMask_T_24,_wordMask_T_22,_wordMask_T_20,_wordMask_T_18,
    _wordMask_T_16,_wordMask_T_14,_wordMask_T_12}; // @[Cat.scala 33:92]
  wire [63:0] wordMask = io_in_bits_req_cmd[0] ? _wordMask_T_27 : 64'h0; // @[Cache.scala 276:21]
  reg [2:0] writeL2BeatCnt_value; // @[Counter.scala 61:40]
  wire  _T_5 = io_out_ready & io_out_valid; // @[Decoupled.scala 51:35]
  wire  _T_6 = io_in_bits_req_cmd == 4'h3; // @[Cache.scala 279:32]
  wire  _T_7 = io_in_bits_req_cmd == 4'h7; // @[SimpleBus.scala 78:27]
  wire  _T_8 = io_in_bits_req_cmd == 4'h3 | _T_7; // @[Cache.scala 279:60]
  wire [2:0] _value_T_1 = writeL2BeatCnt_value + 3'h1; // @[Counter.scala 77:24]
  wire [2:0] _GEN_0 = _T_5 & (io_in_bits_req_cmd == 4'h3 | _T_7) ? _value_T_1 : writeL2BeatCnt_value; // @[Cache.scala 279:83 Counter.scala 77:15 61:40]
  wire  hitWrite = hit & io_in_bits_req_cmd[0]; // @[Cache.scala 283:22]
  wire [63:0] _dataHitWriteBus_x1_T = io_in_bits_req_wdata & wordMask; // @[BitUtils.scala 34:14]
  wire [63:0] _dataHitWriteBus_x1_T_1 = ~wordMask; // @[BitUtils.scala 34:39]
  wire [63:0] _dataHitWriteBus_x1_T_2 = dataRead & _dataHitWriteBus_x1_T_1; // @[BitUtils.scala 34:37]
  wire [2:0] _dataHitWriteBus_x3_T_3 = _T_8 ? writeL2BeatCnt_value : addr_wordIndex; // @[Cache.scala 286:51]
  wire  metaHitWriteBus_x5 = hitWrite & ~meta_dirty; // @[Cache.scala 289:22]
  reg [3:0] state; // @[Cache.scala 294:22]
  reg  needFlush; // @[Cache.scala 295:26]
  wire  _GEN_1 = io_flush & state != 4'h0 | needFlush; // @[Cache.scala 295:26 297:{41,53}]
  reg [2:0] readBeatCnt_value; // @[Counter.scala 61:40]
  reg [2:0] writeBeatCnt_value; // @[Counter.scala 61:40]
  reg [1:0] state2; // @[Cache.scala 304:23]
  wire  _T_14 = state == 4'h3; // @[Cache.scala 306:39]
  wire  _T_15 = state == 4'h8; // @[Cache.scala 306:66]
  wire [2:0] _T_20 = _T_15 ? readBeatCnt_value : writeBeatCnt_value; // @[Cache.scala 307:33]
  wire  _dataWay_T = state2 == 2'h1; // @[Cache.scala 308:60]
  reg [63:0] dataWay_0_data; // @[Reg.scala 19:16]
  reg [63:0] dataWay_1_data; // @[Reg.scala 19:16]
  reg [63:0] dataWay_2_data; // @[Reg.scala 19:16]
  reg [63:0] dataWay_3_data; // @[Reg.scala 19:16]
  wire [63:0] _dataHitWay_T_4 = io_in_bits_waymask[0] ? dataWay_0_data : 64'h0; // @[Mux.scala 27:73]
  wire [63:0] _dataHitWay_T_5 = io_in_bits_waymask[1] ? dataWay_1_data : 64'h0; // @[Mux.scala 27:73]
  wire [63:0] _dataHitWay_T_6 = io_in_bits_waymask[2] ? dataWay_2_data : 64'h0; // @[Mux.scala 27:73]
  wire [63:0] _dataHitWay_T_7 = io_in_bits_waymask[3] ? dataWay_3_data : 64'h0; // @[Mux.scala 27:73]
  wire [63:0] _dataHitWay_T_8 = _dataHitWay_T_4 | _dataHitWay_T_5; // @[Mux.scala 27:73]
  wire [63:0] _dataHitWay_T_9 = _dataHitWay_T_8 | _dataHitWay_T_6; // @[Mux.scala 27:73]
  wire  _T_23 = io_dataReadBus_req_ready & io_dataReadBus_req_valid; // @[Decoupled.scala 51:35]
  wire  _T_26 = io_mem_req_ready & io_mem_req_valid; // @[Decoupled.scala 51:35]
  wire  _T_27 = io_cohResp_ready & io_cohResp_valid; // @[Decoupled.scala 51:35]
  wire  _T_29 = hitReadBurst & io_out_ready; // @[Cache.scala 314:79]
  wire [1:0] _GEN_8 = _T_26 | _T_27 | hitReadBurst & io_out_ready ? 2'h0 : state2; // @[Cache.scala 314:105 304:23 314:96]
  wire [31:0] raddr = {io_in_bits_req_addr[31:3],3'h0}; // @[Cat.scala 33:92]
  wire [31:0] waddr = {meta_tag,addr_index,6'h0}; // @[Cat.scala 33:92]
  wire  _cmd_T = state == 4'h1; // @[Cache.scala 322:23]
  wire [2:0] _cmd_T_2 = writeBeatCnt_value == 3'h7 ? 3'h7 : 3'h3; // @[Cache.scala 323:8]
  wire [2:0] cmd = state == 4'h1 ? 3'h2 : _cmd_T_2; // @[Cache.scala 322:16]
  wire  _io_mem_req_valid_T_2 = state2 == 2'h2; // @[Cache.scala 329:89]
  reg  afterFirstRead; // @[Cache.scala 336:31]
  reg  alreadyOutFire; // @[Reg.scala 35:20]
  wire  _GEN_12 = _T_5 | alreadyOutFire; // @[Reg.scala 36:18 35:20 36:22]
  wire  _readingFirst_T_1 = io_mem_resp_ready & io_mem_resp_valid; // @[Decoupled.scala 51:35]
  wire  _readingFirst_T_3 = state == 4'h2; // @[Cache.scala 338:68]
  wire  readingFirst = ~afterFirstRead & _readingFirst_T_1 & state == 4'h2; // @[Cache.scala 338:58]
  wire  _inRdataRegDemand_T_2 = mmio ? state == 4'h6 : readingFirst; // @[Cache.scala 340:39]
  reg [63:0] inRdataRegDemand; // @[Reg.scala 19:16]
  wire  _io_cohResp_valid_T = state == 4'h0; // @[Cache.scala 343:31]
  wire  _io_cohResp_valid_T_4 = _T_15 & _io_mem_req_valid_T_2; // @[Cache.scala 344:46]
  wire  _releaseLast_T_2 = _T_15 & _T_27; // @[Cache.scala 346:49]
  reg [2:0] releaseLast_c_value; // @[Counter.scala 61:40]
  wire  releaseLast_wrap_wrap = releaseLast_c_value == 3'h7; // @[Counter.scala 73:24]
  wire [2:0] _releaseLast_wrap_value_T_1 = releaseLast_c_value + 3'h1; // @[Counter.scala 77:24]
  wire  releaseLast = _releaseLast_T_2 & releaseLast_wrap_wrap; // @[Counter.scala 118:{16,23} 117:24]
  wire [2:0] _io_cohResp_bits_cmd_T_1 = releaseLast ? 3'h6 : 3'h0; // @[Cache.scala 347:54]
  wire [3:0] _io_cohResp_bits_cmd_T_2 = hit ? 4'hc : 4'h8; // @[Cache.scala 348:8]
  wire  respToL1Fire = _T_29 & _io_mem_req_valid_T_2; // @[Cache.scala 350:51]
  wire  _respToL1Last_T_6 = (_io_cohResp_valid_T | _io_cohResp_valid_T_4) & hitReadBurst & io_out_ready; // @[Cache.scala 351:112]
  reg [2:0] respToL1Last_c_value; // @[Counter.scala 61:40]
  wire  respToL1Last_wrap_wrap = respToL1Last_c_value == 3'h7; // @[Counter.scala 73:24]
  wire [2:0] _respToL1Last_wrap_value_T_1 = respToL1Last_c_value + 3'h1; // @[Counter.scala 77:24]
  wire  respToL1Last = _respToL1Last_T_6 & respToL1Last_wrap_wrap; // @[Counter.scala 118:{16,23} 117:24]
  wire [3:0] _state_T = hit ? 4'h8 : 4'h0; // @[Cache.scala 360:23]
  wire [2:0] _value_T_4 = addr_wordIndex + 3'h1; // @[Cache.scala 365:93]
  wire [2:0] _value_T_5 = addr_wordIndex == 3'h7 ? 3'h0 : _value_T_4; // @[Cache.scala 365:33]
  wire  _T_38 = ~io_flush; // @[Cache.scala 366:38]
  wire [3:0] _state_T_3 = meta_dirty ? 4'h3 : 4'h1; // @[Cache.scala 367:42]
  wire [3:0] _state_T_4 = mmio ? 4'h5 : _state_T_3; // @[Cache.scala 367:21]
  wire [3:0] _GEN_20 = (miss | mmio) & ~io_flush ? _state_T_4 : state; // @[Cache.scala 366:49 367:15 294:22]
  wire  _T_41 = io_mmio_req_ready & io_mmio_req_valid; // @[Decoupled.scala 51:35]
  wire  _T_43 = io_mmio_resp_ready & io_mmio_resp_valid; // @[Decoupled.scala 51:35]
  wire [3:0] _GEN_26 = _T_43 ? 4'h7 : state; // @[Cache.scala 294:22 372:{48,56}]
  wire [2:0] _value_T_7 = readBeatCnt_value + 3'h1; // @[Counter.scala 77:24]
  wire [2:0] _GEN_27 = _T_27 | respToL1Fire ? _value_T_7 : readBeatCnt_value; // @[Cache.scala 375:46 Counter.scala 77:15 61:40]
  wire [3:0] _GEN_28 = probe & _T_27 & releaseLast | respToL1Fire & respToL1Last ? 4'h0 : state; // @[Cache.scala 294:22 376:{86,94}]
  wire [3:0] _GEN_29 = _T_26 ? 4'h2 : state; // @[Cache.scala 379:48 380:13 294:22]
  wire [2:0] _GEN_30 = _T_26 ? addr_wordIndex : readBeatCnt_value; // @[Cache.scala 379:48 381:25 Counter.scala 61:40]
  wire [2:0] _GEN_31 = _T_6 ? 3'h0 : _GEN_0; // @[Cache.scala 388:{52,75}]
  wire  _T_57 = io_mem_resp_bits_cmd == 4'h6; // @[SimpleBus.scala 91:24]
  wire [3:0] _GEN_32 = _T_57 ? 4'h7 : state; // @[Cache.scala 294:22 389:{44,52}]
  wire  _GEN_33 = _readingFirst_T_1 | afterFirstRead; // @[Cache.scala 385:31 386:24 336:31]
  wire [2:0] _GEN_34 = _readingFirst_T_1 ? _value_T_7 : readBeatCnt_value; // @[Cache.scala 385:31 Counter.scala 77:15 61:40]
  wire [2:0] _GEN_35 = _readingFirst_T_1 ? _GEN_31 : _GEN_0; // @[Cache.scala 385:31]
  wire [3:0] _GEN_36 = _readingFirst_T_1 ? _GEN_32 : state; // @[Cache.scala 294:22 385:31]
  wire [2:0] _value_T_11 = writeBeatCnt_value + 3'h1; // @[Counter.scala 77:24]
  wire [2:0] _GEN_37 = _T_26 ? _value_T_11 : writeBeatCnt_value; // @[Cache.scala 394:30 Counter.scala 77:15 61:40]
  wire  _T_60 = io_mem_req_bits_cmd == 4'h7; // @[SimpleBus.scala 78:27]
  wire [3:0] _GEN_38 = _T_60 & _T_26 ? 4'h4 : state; // @[Cache.scala 294:22 395:{63,71}]
  wire [3:0] _GEN_39 = _readingFirst_T_1 ? 4'h1 : state; // @[Cache.scala 294:22 398:{51,59}]
  wire [3:0] _GEN_40 = _T_5 | needFlush | alreadyOutFire ? 4'h0 : state; // @[Cache.scala 294:22 399:{74,82}]
  wire [3:0] _GEN_41 = 4'h7 == state ? _GEN_40 : state; // @[Cache.scala 353:18 294:22]
  wire [3:0] _GEN_42 = 4'h4 == state ? _GEN_39 : _GEN_41; // @[Cache.scala 353:18]
  wire [2:0] _GEN_43 = 4'h3 == state ? _GEN_37 : writeBeatCnt_value; // @[Cache.scala 353:18 Counter.scala 61:40]
  wire [3:0] _GEN_44 = 4'h3 == state ? _GEN_38 : _GEN_42; // @[Cache.scala 353:18]
  wire  _GEN_45 = 4'h2 == state ? _GEN_33 : afterFirstRead; // @[Cache.scala 353:18 336:31]
  wire [2:0] _GEN_46 = 4'h2 == state ? _GEN_34 : readBeatCnt_value; // @[Cache.scala 353:18 Counter.scala 61:40]
  wire [2:0] _GEN_47 = 4'h2 == state ? _GEN_35 : _GEN_0; // @[Cache.scala 353:18]
  wire [3:0] _GEN_48 = 4'h2 == state ? _GEN_36 : _GEN_44; // @[Cache.scala 353:18]
  wire [2:0] _GEN_49 = 4'h2 == state ? writeBeatCnt_value : _GEN_43; // @[Cache.scala 353:18 Counter.scala 61:40]
  wire [3:0] _GEN_50 = 4'h1 == state ? _GEN_29 : _GEN_48; // @[Cache.scala 353:18]
  wire [2:0] _GEN_51 = 4'h1 == state ? _GEN_30 : _GEN_46; // @[Cache.scala 353:18]
  wire  _GEN_52 = 4'h1 == state ? afterFirstRead : _GEN_45; // @[Cache.scala 353:18 336:31]
  wire [2:0] _GEN_53 = 4'h1 == state ? _GEN_0 : _GEN_47; // @[Cache.scala 353:18]
  wire [2:0] _GEN_54 = 4'h1 == state ? writeBeatCnt_value : _GEN_49; // @[Cache.scala 353:18 Counter.scala 61:40]
  wire [2:0] _GEN_55 = 4'h8 == state ? _GEN_27 : _GEN_51; // @[Cache.scala 353:18]
  wire [3:0] _GEN_56 = 4'h8 == state ? _GEN_28 : _GEN_50; // @[Cache.scala 353:18]
  wire  _GEN_57 = 4'h8 == state ? afterFirstRead : _GEN_52; // @[Cache.scala 353:18 336:31]
  wire [2:0] _GEN_58 = 4'h8 == state ? _GEN_0 : _GEN_53; // @[Cache.scala 353:18]
  wire [2:0] _GEN_59 = 4'h8 == state ? writeBeatCnt_value : _GEN_54; // @[Cache.scala 353:18 Counter.scala 61:40]
  wire [63:0] _dataRefill_T = readingFirst ? wordMask : 64'h0; // @[Cache.scala 402:67]
  wire [63:0] _dataRefill_T_1 = io_in_bits_req_wdata & _dataRefill_T; // @[BitUtils.scala 34:14]
  wire [63:0] _dataRefill_T_2 = ~_dataRefill_T; // @[BitUtils.scala 34:39]
  wire [63:0] _dataRefill_T_3 = io_mem_resp_bits_rdata & _dataRefill_T_2; // @[BitUtils.scala 34:37]
  wire  dataRefillWriteBus_x9 = _readingFirst_T_3 & _readingFirst_T_1; // @[Cache.scala 404:39]
  wire  metaRefillWriteBus_req_valid = dataRefillWriteBus_x9 & _T_57; // @[Cache.scala 412:59]
  wire  _io_out_bits_cmd_T_4 = ~io_in_bits_req_cmd[0] & ~io_in_bits_req_cmd[3]; // @[SimpleBus.scala 73:26]
  wire [2:0] _io_out_bits_cmd_T_6 = io_in_bits_req_cmd[0] ? 3'h5 : 3'h0; // @[Cache.scala 440:79]
  wire [2:0] _io_out_bits_cmd_T_7 = _io_out_bits_cmd_T_4 ? 3'h6 : _io_out_bits_cmd_T_6; // @[Cache.scala 440:27]
  wire  _io_out_valid_T_4 = state == 4'h7; // @[Cache.scala 446:48]
  wire  _io_out_valid_T_23 = io_in_bits_req_cmd[0] | mmio ? _io_out_valid_T_4 : afterFirstRead & ~alreadyOutFire; // @[Cache.scala 447:45]
  wire  _io_out_valid_T_25 = probe ? 1'h0 : hit | _io_out_valid_T_23; // @[Cache.scala 447:8]
  wire  _io_isFinish_T_4 = miss ? _io_cohResp_valid_T : _T_15 & releaseLast; // @[Cache.scala 454:51]
  wire  _io_isFinish_T_13 = hit | io_in_bits_req_cmd[0] ? _T_5 : _io_out_valid_T_4 & _GEN_12; // @[Cache.scala 455:8]
  Arbiter metaWriteArb ( // @[Cache.scala 254:28]
    .io_in_0_valid(metaWriteArb_io_in_0_valid),
    .io_in_0_bits_setIdx(metaWriteArb_io_in_0_bits_setIdx),
    .io_in_0_bits_data_tag(metaWriteArb_io_in_0_bits_data_tag),
    .io_in_0_bits_waymask(metaWriteArb_io_in_0_bits_waymask),
    .io_in_1_valid(metaWriteArb_io_in_1_valid),
    .io_in_1_bits_setIdx(metaWriteArb_io_in_1_bits_setIdx),
    .io_in_1_bits_data_tag(metaWriteArb_io_in_1_bits_data_tag),
    .io_in_1_bits_data_dirty(metaWriteArb_io_in_1_bits_data_dirty),
    .io_in_1_bits_waymask(metaWriteArb_io_in_1_bits_waymask),
    .io_out_valid(metaWriteArb_io_out_valid),
    .io_out_bits_setIdx(metaWriteArb_io_out_bits_setIdx),
    .io_out_bits_data_tag(metaWriteArb_io_out_bits_data_tag),
    .io_out_bits_data_dirty(metaWriteArb_io_out_bits_data_dirty),
    .io_out_bits_waymask(metaWriteArb_io_out_bits_waymask)
  );
  Arbiter_1 dataWriteArb ( // @[Cache.scala 255:28]
    .io_in_0_valid(dataWriteArb_io_in_0_valid),
    .io_in_0_bits_setIdx(dataWriteArb_io_in_0_bits_setIdx),
    .io_in_0_bits_data_data(dataWriteArb_io_in_0_bits_data_data),
    .io_in_0_bits_waymask(dataWriteArb_io_in_0_bits_waymask),
    .io_in_1_valid(dataWriteArb_io_in_1_valid),
    .io_in_1_bits_setIdx(dataWriteArb_io_in_1_bits_setIdx),
    .io_in_1_bits_data_data(dataWriteArb_io_in_1_bits_data_data),
    .io_in_1_bits_waymask(dataWriteArb_io_in_1_bits_waymask),
    .io_out_valid(dataWriteArb_io_out_valid),
    .io_out_bits_setIdx(dataWriteArb_io_out_bits_setIdx),
    .io_out_bits_data_data(dataWriteArb_io_out_bits_data_data),
    .io_out_bits_waymask(dataWriteArb_io_out_bits_waymask)
  );
  assign io_in_ready = io_out_ready & (_io_cohResp_valid_T & ~hitReadBurst) & ~miss & ~probe; // @[Cache.scala 458:79]
  assign io_out_valid = io_in_valid & _io_out_valid_T_25; // @[Cache.scala 445:31]
  assign io_out_bits_cmd = {{1'd0}, _io_out_bits_cmd_T_7}; // @[Cache.scala 440:21]
  assign io_out_bits_rdata = hit ? dataRead : inRdataRegDemand; // @[Cache.scala 439:29]
  assign io_out_bits_user = io_in_bits_req_user; // @[Cache.scala 442:56]
  assign io_isFinish = probe ? _T_27 & _io_isFinish_T_4 : _io_isFinish_T_13; // @[Cache.scala 454:21]
  assign io_dataReadBus_req_valid = (state == 4'h3 | state == 4'h8) & state2 == 2'h0; // @[Cache.scala 306:81]
  assign io_dataReadBus_req_bits_setIdx = {addr_index,_T_20}; // @[Cat.scala 33:92]
  assign io_dataWriteBus_req_valid = dataWriteArb_io_out_valid; // @[Cache.scala 409:23]
  assign io_dataWriteBus_req_bits_setIdx = dataWriteArb_io_out_bits_setIdx; // @[Cache.scala 409:23]
  assign io_dataWriteBus_req_bits_data_data = dataWriteArb_io_out_bits_data_data; // @[Cache.scala 409:23]
  assign io_dataWriteBus_req_bits_waymask = dataWriteArb_io_out_bits_waymask; // @[Cache.scala 409:23]
  assign io_metaWriteBus_req_valid = metaWriteArb_io_out_valid; // @[Cache.scala 419:23]
  assign io_metaWriteBus_req_bits_setIdx = metaWriteArb_io_out_bits_setIdx; // @[Cache.scala 419:23]
  assign io_metaWriteBus_req_bits_data_tag = metaWriteArb_io_out_bits_data_tag; // @[Cache.scala 419:23]
  assign io_metaWriteBus_req_bits_data_dirty = metaWriteArb_io_out_bits_data_dirty; // @[Cache.scala 419:23]
  assign io_metaWriteBus_req_bits_waymask = metaWriteArb_io_out_bits_waymask; // @[Cache.scala 419:23]
  assign io_mem_req_valid = _cmd_T | _T_14 & state2 == 2'h2; // @[Cache.scala 329:48]
  assign io_mem_req_bits_addr = _cmd_T ? raddr : waddr; // @[Cache.scala 324:35]
  assign io_mem_req_bits_cmd = {{1'd0}, cmd}; // @[SimpleBus.scala 65:14]
  assign io_mem_req_bits_wdata = _dataHitWay_T_9 | _dataHitWay_T_7; // @[Mux.scala 27:73]
  assign io_mem_resp_ready = 1'h1; // @[Cache.scala 328:21]
  assign io_mmio_req_valid = state == 4'h5; // @[Cache.scala 334:31]
  assign io_mmio_req_bits_addr = io_in_bits_req_addr; // @[Cache.scala 332:20]
  assign io_mmio_req_bits_size = io_in_bits_req_size; // @[Cache.scala 332:20]
  assign io_mmio_req_bits_cmd = io_in_bits_req_cmd; // @[Cache.scala 332:20]
  assign io_mmio_req_bits_wmask = io_in_bits_req_wmask; // @[Cache.scala 332:20]
  assign io_mmio_req_bits_wdata = io_in_bits_req_wdata; // @[Cache.scala 332:20]
  assign io_mmio_resp_ready = 1'h1; // @[Cache.scala 333:22]
  assign io_cohResp_valid = state == 4'h0 & probe | _io_cohResp_valid_T_4; // @[Cache.scala 343:53]
  assign io_cohResp_bits_cmd = _T_15 ? {{1'd0}, _io_cohResp_bits_cmd_T_1} : _io_cohResp_bits_cmd_T_2; // @[Cache.scala 347:29]
  assign io_cohResp_bits_rdata = _dataHitWay_T_9 | _dataHitWay_T_7; // @[Mux.scala 27:73]
  assign io_dataReadRespToL1 = hitReadBurst & (_io_cohResp_valid_T & io_out_ready | _io_cohResp_valid_T_4); // @[Cache.scala 459:39]
  assign metaWriteArb_io_in_0_valid = hitWrite & ~meta_dirty; // @[Cache.scala 289:22]
  assign metaWriteArb_io_in_0_bits_setIdx = io_in_bits_req_addr[12:6]; // @[Cache.scala 77:45]
  assign metaWriteArb_io_in_0_bits_data_tag = _meta_T_23 | _meta_T_21; // @[Mux.scala 27:73]
  assign metaWriteArb_io_in_0_bits_waymask = io_in_bits_waymask; // @[Cache.scala 288:29 SRAMTemplate.scala 38:24]
  assign metaWriteArb_io_in_1_valid = dataRefillWriteBus_x9 & _T_57; // @[Cache.scala 412:59]
  assign metaWriteArb_io_in_1_bits_setIdx = io_in_bits_req_addr[12:6]; // @[Cache.scala 77:45]
  assign metaWriteArb_io_in_1_bits_data_tag = io_in_bits_req_addr[31:13]; // @[Cache.scala 258:31]
  assign metaWriteArb_io_in_1_bits_data_dirty = io_in_bits_req_cmd[0]; // @[SimpleBus.scala 74:22]
  assign metaWriteArb_io_in_1_bits_waymask = io_in_bits_waymask; // @[Cache.scala 411:32 SRAMTemplate.scala 38:24]
  assign dataWriteArb_io_in_0_valid = hit & io_in_bits_req_cmd[0]; // @[Cache.scala 283:22]
  assign dataWriteArb_io_in_0_bits_setIdx = {addr_index,_dataHitWriteBus_x3_T_3}; // @[Cat.scala 33:92]
  assign dataWriteArb_io_in_0_bits_data_data = _dataHitWriteBus_x1_T | _dataHitWriteBus_x1_T_2; // @[BitUtils.scala 34:26]
  assign dataWriteArb_io_in_0_bits_waymask = io_in_bits_waymask; // @[Cache.scala 284:29 SRAMTemplate.scala 38:24]
  assign dataWriteArb_io_in_1_valid = _readingFirst_T_3 & _readingFirst_T_1; // @[Cache.scala 404:39]
  assign dataWriteArb_io_in_1_bits_setIdx = {addr_index,readBeatCnt_value}; // @[Cat.scala 33:92]
  assign dataWriteArb_io_in_1_bits_data_data = _dataRefill_T_1 | _dataRefill_T_3; // @[BitUtils.scala 34:26]
  assign dataWriteArb_io_in_1_bits_waymask = io_in_bits_waymask; // @[Cache.scala 403:32 SRAMTemplate.scala 38:24]
  always @(posedge clock) begin
    if (reset) begin // @[Counter.scala 61:40]
      writeL2BeatCnt_value <= 3'h0; // @[Counter.scala 61:40]
    end else if (4'h0 == state) begin // @[Cache.scala 353:18]
      writeL2BeatCnt_value <= _GEN_0;
    end else if (4'h5 == state) begin // @[Cache.scala 353:18]
      writeL2BeatCnt_value <= _GEN_0;
    end else if (4'h6 == state) begin // @[Cache.scala 353:18]
      writeL2BeatCnt_value <= _GEN_0;
    end else begin
      writeL2BeatCnt_value <= _GEN_58;
    end
    if (reset) begin // @[Cache.scala 294:22]
      state <= 4'h0; // @[Cache.scala 294:22]
    end else if (4'h0 == state) begin // @[Cache.scala 353:18]
      if (probe) begin // @[Cache.scala 358:20]
        if (_T_27) begin // @[Cache.scala 359:32]
          state <= _state_T; // @[Cache.scala 360:17]
        end
      end else if (_T_29) begin // @[Cache.scala 363:50]
        state <= 4'h8; // @[Cache.scala 364:15]
      end else begin
        state <= _GEN_20;
      end
    end else if (4'h5 == state) begin // @[Cache.scala 353:18]
      if (_T_41) begin // @[Cache.scala 371:46]
        state <= 4'h6; // @[Cache.scala 371:54]
      end
    end else if (4'h6 == state) begin // @[Cache.scala 353:18]
      state <= _GEN_26;
    end else begin
      state <= _GEN_56;
    end
    if (reset) begin // @[Cache.scala 295:26]
      needFlush <= 1'h0; // @[Cache.scala 295:26]
    end else if (_T_5 & needFlush) begin // @[Cache.scala 298:35]
      needFlush <= 1'h0; // @[Cache.scala 298:47]
    end else begin
      needFlush <= _GEN_1;
    end
    if (reset) begin // @[Counter.scala 61:40]
      readBeatCnt_value <= 3'h0; // @[Counter.scala 61:40]
    end else if (4'h0 == state) begin // @[Cache.scala 353:18]
      if (probe) begin // @[Cache.scala 358:20]
        if (_T_27) begin // @[Cache.scala 359:32]
          readBeatCnt_value <= addr_wordIndex; // @[Cache.scala 361:29]
        end
      end else if (_T_29) begin // @[Cache.scala 363:50]
        readBeatCnt_value <= _value_T_5; // @[Cache.scala 365:27]
      end
    end else if (!(4'h5 == state)) begin // @[Cache.scala 353:18]
      if (!(4'h6 == state)) begin // @[Cache.scala 353:18]
        readBeatCnt_value <= _GEN_55;
      end
    end
    if (reset) begin // @[Counter.scala 61:40]
      writeBeatCnt_value <= 3'h0; // @[Counter.scala 61:40]
    end else if (!(4'h0 == state)) begin // @[Cache.scala 353:18]
      if (!(4'h5 == state)) begin // @[Cache.scala 353:18]
        if (!(4'h6 == state)) begin // @[Cache.scala 353:18]
          writeBeatCnt_value <= _GEN_59;
        end
      end
    end
    if (reset) begin // @[Cache.scala 304:23]
      state2 <= 2'h0; // @[Cache.scala 304:23]
    end else if (2'h0 == state2) begin // @[Cache.scala 311:19]
      if (_T_23) begin // @[Cache.scala 312:51]
        state2 <= 2'h1; // @[Cache.scala 312:60]
      end
    end else if (2'h1 == state2) begin // @[Cache.scala 311:19]
      state2 <= 2'h2; // @[Cache.scala 313:35]
    end else if (2'h2 == state2) begin // @[Cache.scala 311:19]
      state2 <= _GEN_8;
    end
    if (_dataWay_T) begin // @[Reg.scala 20:18]
      dataWay_0_data <= io_dataReadBus_resp_data_0_data; // @[Reg.scala 20:22]
    end
    if (_dataWay_T) begin // @[Reg.scala 20:18]
      dataWay_1_data <= io_dataReadBus_resp_data_1_data; // @[Reg.scala 20:22]
    end
    if (_dataWay_T) begin // @[Reg.scala 20:18]
      dataWay_2_data <= io_dataReadBus_resp_data_2_data; // @[Reg.scala 20:22]
    end
    if (_dataWay_T) begin // @[Reg.scala 20:18]
      dataWay_3_data <= io_dataReadBus_resp_data_3_data; // @[Reg.scala 20:22]
    end
    if (reset) begin // @[Cache.scala 336:31]
      afterFirstRead <= 1'h0; // @[Cache.scala 336:31]
    end else if (4'h0 == state) begin // @[Cache.scala 353:18]
      afterFirstRead <= 1'h0; // @[Cache.scala 355:22]
    end else if (!(4'h5 == state)) begin // @[Cache.scala 353:18]
      if (!(4'h6 == state)) begin // @[Cache.scala 353:18]
        afterFirstRead <= _GEN_57;
      end
    end
    if (reset) begin // @[Reg.scala 35:20]
      alreadyOutFire <= 1'h0; // @[Reg.scala 35:20]
    end else if (4'h0 == state) begin // @[Cache.scala 353:18]
      alreadyOutFire <= 1'h0; // @[Cache.scala 356:22]
    end else begin
      alreadyOutFire <= _GEN_12;
    end
    if (_inRdataRegDemand_T_2) begin // @[Reg.scala 20:18]
      if (mmio) begin // @[Cache.scala 339:39]
        inRdataRegDemand <= io_mmio_resp_bits_rdata;
      end else begin
        inRdataRegDemand <= io_mem_resp_bits_rdata;
      end
    end
    if (reset) begin // @[Counter.scala 61:40]
      releaseLast_c_value <= 3'h0; // @[Counter.scala 61:40]
    end else if (_releaseLast_T_2) begin // @[Counter.scala 118:16]
      releaseLast_c_value <= _releaseLast_wrap_value_T_1; // @[Counter.scala 77:15]
    end
    if (reset) begin // @[Counter.scala 61:40]
      respToL1Last_c_value <= 3'h0; // @[Counter.scala 61:40]
    end else if (_respToL1Last_T_6) begin // @[Counter.scala 118:16]
      respToL1Last_c_value <= _respToL1Last_wrap_value_T_1; // @[Counter.scala 77:15]
    end
    `ifndef SYNTHESIS
    `ifdef PRINTF_COND
      if (`PRINTF_COND) begin
    `endif
        if (~reset & ~(~(mmio & hit))) begin
          $fwrite(32'h80000002,
            "Assertion failed: MMIO request should not hit in cache\n    at Cache.scala:265 assert(!(mmio && hit), \"MMIO request should not hit in cache\")\n"
            ); // @[Cache.scala 265:9]
        end
    `ifdef PRINTF_COND
      end
    `endif
    `endif // SYNTHESIS
    `ifndef SYNTHESIS
    `ifdef STOP_COND
      if (`STOP_COND) begin
    `endif
        if (~(~(mmio & hit)) & ~reset) begin
          $fatal; // @[Cache.scala 265:9]
        end
    `ifdef STOP_COND
      end
    `endif
    `endif // SYNTHESIS
    `ifndef SYNTHESIS
    `ifdef PRINTF_COND
      if (`PRINTF_COND) begin
    `endif
        if (_T_3 & ~(~(metaHitWriteBus_x5 & metaRefillWriteBus_req_valid))) begin
          $fwrite(32'h80000002,
            "Assertion failed\n    at Cache.scala:461 assert(!(metaHitWriteBus.req.valid && metaRefillWriteBus.req.valid))\n"
            ); // @[Cache.scala 461:9]
        end
    `ifdef PRINTF_COND
      end
    `endif
    `endif // SYNTHESIS
    `ifndef SYNTHESIS
    `ifdef STOP_COND
      if (`STOP_COND) begin
    `endif
        if (~(~(metaHitWriteBus_x5 & metaRefillWriteBus_req_valid)) & _T_3) begin
          $fatal; // @[Cache.scala 461:9]
        end
    `ifdef STOP_COND
      end
    `endif
    `endif // SYNTHESIS
    `ifndef SYNTHESIS
    `ifdef PRINTF_COND
      if (`PRINTF_COND) begin
    `endif
        if (_T_3 & ~(~(hitWrite & dataRefillWriteBus_x9))) begin
          $fwrite(32'h80000002,
            "Assertion failed\n    at Cache.scala:462 assert(!(dataHitWriteBus.req.valid && dataRefillWriteBus.req.valid))\n"
            ); // @[Cache.scala 462:9]
        end
    `ifdef PRINTF_COND
      end
    `endif
    `endif // SYNTHESIS
    `ifndef SYNTHESIS
    `ifdef STOP_COND
      if (`STOP_COND) begin
    `endif
        if (~(~(hitWrite & dataRefillWriteBus_x9)) & _T_3) begin
          $fatal; // @[Cache.scala 462:9]
        end
    `ifdef STOP_COND
      end
    `endif
    `endif // SYNTHESIS
    `ifndef SYNTHESIS
    `ifdef PRINTF_COND
      if (`PRINTF_COND) begin
    `endif
        if (_T_3 & ~_T_38) begin
          $fwrite(32'h80000002,
            "Assertion failed: only allow to flush icache\n    at Cache.scala:463 assert(!(!ro.B && io.flush), \"only allow to flush icache\")\n"
            ); // @[Cache.scala 463:9]
        end
    `ifdef PRINTF_COND
      end
    `endif
    `endif // SYNTHESIS
    `ifndef SYNTHESIS
    `ifdef STOP_COND
      if (`STOP_COND) begin
    `endif
        if (~_T_38 & _T_3) begin
          $fatal; // @[Cache.scala 463:9]
        end
    `ifdef STOP_COND
      end
    `endif
    `endif // SYNTHESIS
  end
// Register and memory initialization
`ifdef RANDOMIZE_GARBAGE_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_INVALID_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_REG_INIT
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_MEM_INIT
`define RANDOMIZE
`endif
`ifndef RANDOM
`define RANDOM $random
`endif
`ifdef RANDOMIZE_MEM_INIT
  integer initvar;
`endif
`ifndef SYNTHESIS
`ifdef FIRRTL_BEFORE_INITIAL
`FIRRTL_BEFORE_INITIAL
`endif
initial begin
  `ifdef RANDOMIZE
    `ifdef INIT_RANDOM
      `INIT_RANDOM
    `endif
    `ifndef VERILATOR
      `ifdef RANDOMIZE_DELAY
        #`RANDOMIZE_DELAY begin end
      `else
        #0.002 begin end
      `endif
    `endif
`ifdef RANDOMIZE_REG_INIT
  _RAND_0 = {1{`RANDOM}};
  writeL2BeatCnt_value = _RAND_0[2:0];
  _RAND_1 = {1{`RANDOM}};
  state = _RAND_1[3:0];
  _RAND_2 = {1{`RANDOM}};
  needFlush = _RAND_2[0:0];
  _RAND_3 = {1{`RANDOM}};
  readBeatCnt_value = _RAND_3[2:0];
  _RAND_4 = {1{`RANDOM}};
  writeBeatCnt_value = _RAND_4[2:0];
  _RAND_5 = {1{`RANDOM}};
  state2 = _RAND_5[1:0];
  _RAND_6 = {2{`RANDOM}};
  dataWay_0_data = _RAND_6[63:0];
  _RAND_7 = {2{`RANDOM}};
  dataWay_1_data = _RAND_7[63:0];
  _RAND_8 = {2{`RANDOM}};
  dataWay_2_data = _RAND_8[63:0];
  _RAND_9 = {2{`RANDOM}};
  dataWay_3_data = _RAND_9[63:0];
  _RAND_10 = {1{`RANDOM}};
  afterFirstRead = _RAND_10[0:0];
  _RAND_11 = {1{`RANDOM}};
  alreadyOutFire = _RAND_11[0:0];
  _RAND_12 = {2{`RANDOM}};
  inRdataRegDemand = _RAND_12[63:0];
  _RAND_13 = {1{`RANDOM}};
  releaseLast_c_value = _RAND_13[2:0];
  _RAND_14 = {1{`RANDOM}};
  respToL1Last_c_value = _RAND_14[2:0];
`endif // RANDOMIZE_REG_INIT
  `endif // RANDOMIZE
end // initial
`ifdef FIRRTL_AFTER_INITIAL
`FIRRTL_AFTER_INITIAL
`endif
`endif // SYNTHESIS
endmodule
module SRAMTemplate(
  input         clock,
  input         reset,
  output        io_r_req_ready,
  input         io_r_req_valid,
  input  [6:0]  io_r_req_bits_setIdx,
  output [18:0] io_r_resp_data_0_tag,
  output        io_r_resp_data_0_valid,
  output        io_r_resp_data_0_dirty,
  output [18:0] io_r_resp_data_1_tag,
  output        io_r_resp_data_1_valid,
  output        io_r_resp_data_1_dirty,
  output [18:0] io_r_resp_data_2_tag,
  output        io_r_resp_data_2_valid,
  output        io_r_resp_data_2_dirty,
  output [18:0] io_r_resp_data_3_tag,
  output        io_r_resp_data_3_valid,
  output        io_r_resp_data_3_dirty,
  input         io_w_req_valid,
  input  [6:0]  io_w_req_bits_setIdx,
  input  [18:0] io_w_req_bits_data_tag,
  input         io_w_req_bits_data_dirty,
  input  [3:0]  io_w_req_bits_waymask
);
`ifdef RANDOMIZE_MEM_INIT
  reg [31:0] _RAND_0;
  reg [31:0] _RAND_3;
  reg [31:0] _RAND_6;
  reg [31:0] _RAND_9;
`endif // RANDOMIZE_MEM_INIT
`ifdef RANDOMIZE_REG_INIT
  reg [31:0] _RAND_1;
  reg [31:0] _RAND_2;
  reg [31:0] _RAND_4;
  reg [31:0] _RAND_5;
  reg [31:0] _RAND_7;
  reg [31:0] _RAND_8;
  reg [31:0] _RAND_10;
  reg [31:0] _RAND_11;
  reg [31:0] _RAND_12;
  reg [31:0] _RAND_13;
`endif // RANDOMIZE_REG_INIT
  reg [20:0] array_0 [0:127]; // @[SRAMTemplate.scala 76:26]
  wire  array_0_rdata_MPORT_en; // @[SRAMTemplate.scala 76:26]
  wire [6:0] array_0_rdata_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire [20:0] array_0_rdata_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [20:0] array_0_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [6:0] array_0_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire  array_0_MPORT_mask; // @[SRAMTemplate.scala 76:26]
  wire  array_0_MPORT_en; // @[SRAMTemplate.scala 76:26]
  reg  array_0_rdata_MPORT_en_pipe_0;
  reg [6:0] array_0_rdata_MPORT_addr_pipe_0;
  reg [20:0] array_1 [0:127]; // @[SRAMTemplate.scala 76:26]
  wire  array_1_rdata_MPORT_en; // @[SRAMTemplate.scala 76:26]
  wire [6:0] array_1_rdata_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire [20:0] array_1_rdata_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [20:0] array_1_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [6:0] array_1_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire  array_1_MPORT_mask; // @[SRAMTemplate.scala 76:26]
  wire  array_1_MPORT_en; // @[SRAMTemplate.scala 76:26]
  reg  array_1_rdata_MPORT_en_pipe_0;
  reg [6:0] array_1_rdata_MPORT_addr_pipe_0;
  reg [20:0] array_2 [0:127]; // @[SRAMTemplate.scala 76:26]
  wire  array_2_rdata_MPORT_en; // @[SRAMTemplate.scala 76:26]
  wire [6:0] array_2_rdata_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire [20:0] array_2_rdata_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [20:0] array_2_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [6:0] array_2_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire  array_2_MPORT_mask; // @[SRAMTemplate.scala 76:26]
  wire  array_2_MPORT_en; // @[SRAMTemplate.scala 76:26]
  reg  array_2_rdata_MPORT_en_pipe_0;
  reg [6:0] array_2_rdata_MPORT_addr_pipe_0;
  reg [20:0] array_3 [0:127]; // @[SRAMTemplate.scala 76:26]
  wire  array_3_rdata_MPORT_en; // @[SRAMTemplate.scala 76:26]
  wire [6:0] array_3_rdata_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire [20:0] array_3_rdata_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [20:0] array_3_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [6:0] array_3_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire  array_3_MPORT_mask; // @[SRAMTemplate.scala 76:26]
  wire  array_3_MPORT_en; // @[SRAMTemplate.scala 76:26]
  reg  array_3_rdata_MPORT_en_pipe_0;
  reg [6:0] array_3_rdata_MPORT_addr_pipe_0;
  reg  resetState; // @[SRAMTemplate.scala 80:30]
  reg [6:0] resetSet; // @[Counter.scala 61:40]
  wire  wrap_wrap = resetSet == 7'h7f; // @[Counter.scala 73:24]
  wire [6:0] _wrap_value_T_1 = resetSet + 7'h1; // @[Counter.scala 77:24]
  wire  resetFinish = resetState & wrap_wrap; // @[Counter.scala 118:{16,23} 117:24]
  wire  _GEN_2 = resetFinish ? 1'h0 : resetState; // @[SRAMTemplate.scala 82:24 80:30 82:38]
  wire  wen = io_w_req_valid | resetState; // @[SRAMTemplate.scala 88:52]
  wire  _realRen_T = ~wen; // @[SRAMTemplate.scala 89:41]
  wire [20:0] _wdataword_T = {io_w_req_bits_data_tag,1'h1,io_w_req_bits_data_dirty}; // @[SRAMTemplate.scala 92:78]
  wire [3:0] waymask = resetState ? 4'hf : io_w_req_bits_waymask; // @[SRAMTemplate.scala 93:20]
  wire [20:0] _rdata_WIRE_1 = array_0_rdata_MPORT_data; // @[SRAMTemplate.scala 98:{78,78}]
  wire [20:0] _rdata_WIRE_2 = array_1_rdata_MPORT_data; // @[SRAMTemplate.scala 98:{78,78}]
  wire [20:0] _rdata_WIRE_3 = array_2_rdata_MPORT_data; // @[SRAMTemplate.scala 98:{78,78}]
  wire [20:0] _rdata_WIRE_4 = array_3_rdata_MPORT_data; // @[SRAMTemplate.scala 98:{78,78}]
  assign array_0_rdata_MPORT_en = array_0_rdata_MPORT_en_pipe_0;
  assign array_0_rdata_MPORT_addr = array_0_rdata_MPORT_addr_pipe_0;
  assign array_0_rdata_MPORT_data = array_0[array_0_rdata_MPORT_addr]; // @[SRAMTemplate.scala 76:26]
  assign array_0_MPORT_data = resetState ? 21'h0 : _wdataword_T;
  assign array_0_MPORT_addr = resetState ? resetSet : io_w_req_bits_setIdx;
  assign array_0_MPORT_mask = waymask[0];
  assign array_0_MPORT_en = io_w_req_valid | resetState;
  assign array_1_rdata_MPORT_en = array_1_rdata_MPORT_en_pipe_0;
  assign array_1_rdata_MPORT_addr = array_1_rdata_MPORT_addr_pipe_0;
  assign array_1_rdata_MPORT_data = array_1[array_1_rdata_MPORT_addr]; // @[SRAMTemplate.scala 76:26]
  assign array_1_MPORT_data = resetState ? 21'h0 : _wdataword_T;
  assign array_1_MPORT_addr = resetState ? resetSet : io_w_req_bits_setIdx;
  assign array_1_MPORT_mask = waymask[1];
  assign array_1_MPORT_en = io_w_req_valid | resetState;
  assign array_2_rdata_MPORT_en = array_2_rdata_MPORT_en_pipe_0;
  assign array_2_rdata_MPORT_addr = array_2_rdata_MPORT_addr_pipe_0;
  assign array_2_rdata_MPORT_data = array_2[array_2_rdata_MPORT_addr]; // @[SRAMTemplate.scala 76:26]
  assign array_2_MPORT_data = resetState ? 21'h0 : _wdataword_T;
  assign array_2_MPORT_addr = resetState ? resetSet : io_w_req_bits_setIdx;
  assign array_2_MPORT_mask = waymask[2];
  assign array_2_MPORT_en = io_w_req_valid | resetState;
  assign array_3_rdata_MPORT_en = array_3_rdata_MPORT_en_pipe_0;
  assign array_3_rdata_MPORT_addr = array_3_rdata_MPORT_addr_pipe_0;
  assign array_3_rdata_MPORT_data = array_3[array_3_rdata_MPORT_addr]; // @[SRAMTemplate.scala 76:26]
  assign array_3_MPORT_data = resetState ? 21'h0 : _wdataword_T;
  assign array_3_MPORT_addr = resetState ? resetSet : io_w_req_bits_setIdx;
  assign array_3_MPORT_mask = waymask[3];
  assign array_3_MPORT_en = io_w_req_valid | resetState;
  assign io_r_req_ready = ~resetState & _realRen_T; // @[SRAMTemplate.scala 101:33]
  assign io_r_resp_data_0_tag = _rdata_WIRE_1[20:2]; // @[SRAMTemplate.scala 98:78]
  assign io_r_resp_data_0_valid = _rdata_WIRE_1[1]; // @[SRAMTemplate.scala 98:78]
  assign io_r_resp_data_0_dirty = _rdata_WIRE_1[0]; // @[SRAMTemplate.scala 98:78]
  assign io_r_resp_data_1_tag = _rdata_WIRE_2[20:2]; // @[SRAMTemplate.scala 98:78]
  assign io_r_resp_data_1_valid = _rdata_WIRE_2[1]; // @[SRAMTemplate.scala 98:78]
  assign io_r_resp_data_1_dirty = _rdata_WIRE_2[0]; // @[SRAMTemplate.scala 98:78]
  assign io_r_resp_data_2_tag = _rdata_WIRE_3[20:2]; // @[SRAMTemplate.scala 98:78]
  assign io_r_resp_data_2_valid = _rdata_WIRE_3[1]; // @[SRAMTemplate.scala 98:78]
  assign io_r_resp_data_2_dirty = _rdata_WIRE_3[0]; // @[SRAMTemplate.scala 98:78]
  assign io_r_resp_data_3_tag = _rdata_WIRE_4[20:2]; // @[SRAMTemplate.scala 98:78]
  assign io_r_resp_data_3_valid = _rdata_WIRE_4[1]; // @[SRAMTemplate.scala 98:78]
  assign io_r_resp_data_3_dirty = _rdata_WIRE_4[0]; // @[SRAMTemplate.scala 98:78]
  always @(posedge clock) begin
    if (array_0_MPORT_en & array_0_MPORT_mask) begin
      array_0[array_0_MPORT_addr] <= array_0_MPORT_data; // @[SRAMTemplate.scala 76:26]
    end
    array_0_rdata_MPORT_en_pipe_0 <= io_r_req_valid & _realRen_T;
    if (io_r_req_valid & _realRen_T) begin
      array_0_rdata_MPORT_addr_pipe_0 <= io_r_req_bits_setIdx;
    end
    if (array_1_MPORT_en & array_1_MPORT_mask) begin
      array_1[array_1_MPORT_addr] <= array_1_MPORT_data; // @[SRAMTemplate.scala 76:26]
    end
    array_1_rdata_MPORT_en_pipe_0 <= io_r_req_valid & _realRen_T;
    if (io_r_req_valid & _realRen_T) begin
      array_1_rdata_MPORT_addr_pipe_0 <= io_r_req_bits_setIdx;
    end
    if (array_2_MPORT_en & array_2_MPORT_mask) begin
      array_2[array_2_MPORT_addr] <= array_2_MPORT_data; // @[SRAMTemplate.scala 76:26]
    end
    array_2_rdata_MPORT_en_pipe_0 <= io_r_req_valid & _realRen_T;
    if (io_r_req_valid & _realRen_T) begin
      array_2_rdata_MPORT_addr_pipe_0 <= io_r_req_bits_setIdx;
    end
    if (array_3_MPORT_en & array_3_MPORT_mask) begin
      array_3[array_3_MPORT_addr] <= array_3_MPORT_data; // @[SRAMTemplate.scala 76:26]
    end
    array_3_rdata_MPORT_en_pipe_0 <= io_r_req_valid & _realRen_T;
    if (io_r_req_valid & _realRen_T) begin
      array_3_rdata_MPORT_addr_pipe_0 <= io_r_req_bits_setIdx;
    end
    resetState <= reset | _GEN_2; // @[SRAMTemplate.scala 80:{30,30}]
    if (reset) begin // @[Counter.scala 61:40]
      resetSet <= 7'h0; // @[Counter.scala 61:40]
    end else if (resetState) begin // @[Counter.scala 118:16]
      resetSet <= _wrap_value_T_1; // @[Counter.scala 77:15]
    end
  end
// Register and memory initialization
`ifdef RANDOMIZE_GARBAGE_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_INVALID_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_REG_INIT
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_MEM_INIT
`define RANDOMIZE
`endif
`ifndef RANDOM
`define RANDOM $random
`endif
`ifdef RANDOMIZE_MEM_INIT
  integer initvar;
`endif
`ifndef SYNTHESIS
`ifdef FIRRTL_BEFORE_INITIAL
`FIRRTL_BEFORE_INITIAL
`endif
initial begin
  `ifdef RANDOMIZE
    `ifdef INIT_RANDOM
      `INIT_RANDOM
    `endif
    `ifndef VERILATOR
      `ifdef RANDOMIZE_DELAY
        #`RANDOMIZE_DELAY begin end
      `else
        #0.002 begin end
      `endif
    `endif
`ifdef RANDOMIZE_MEM_INIT
  _RAND_0 = {1{`RANDOM}};
  for (initvar = 0; initvar < 128; initvar = initvar+1)
    array_0[initvar] = _RAND_0[20:0];
  _RAND_3 = {1{`RANDOM}};
  for (initvar = 0; initvar < 128; initvar = initvar+1)
    array_1[initvar] = _RAND_3[20:0];
  _RAND_6 = {1{`RANDOM}};
  for (initvar = 0; initvar < 128; initvar = initvar+1)
    array_2[initvar] = _RAND_6[20:0];
  _RAND_9 = {1{`RANDOM}};
  for (initvar = 0; initvar < 128; initvar = initvar+1)
    array_3[initvar] = _RAND_9[20:0];
`endif // RANDOMIZE_MEM_INIT
`ifdef RANDOMIZE_REG_INIT
  _RAND_1 = {1{`RANDOM}};
  array_0_rdata_MPORT_en_pipe_0 = _RAND_1[0:0];
  _RAND_2 = {1{`RANDOM}};
  array_0_rdata_MPORT_addr_pipe_0 = _RAND_2[6:0];
  _RAND_4 = {1{`RANDOM}};
  array_1_rdata_MPORT_en_pipe_0 = _RAND_4[0:0];
  _RAND_5 = {1{`RANDOM}};
  array_1_rdata_MPORT_addr_pipe_0 = _RAND_5[6:0];
  _RAND_7 = {1{`RANDOM}};
  array_2_rdata_MPORT_en_pipe_0 = _RAND_7[0:0];
  _RAND_8 = {1{`RANDOM}};
  array_2_rdata_MPORT_addr_pipe_0 = _RAND_8[6:0];
  _RAND_10 = {1{`RANDOM}};
  array_3_rdata_MPORT_en_pipe_0 = _RAND_10[0:0];
  _RAND_11 = {1{`RANDOM}};
  array_3_rdata_MPORT_addr_pipe_0 = _RAND_11[6:0];
  _RAND_12 = {1{`RANDOM}};
  resetState = _RAND_12[0:0];
  _RAND_13 = {1{`RANDOM}};
  resetSet = _RAND_13[6:0];
`endif // RANDOMIZE_REG_INIT
  `endif // RANDOMIZE
end // initial
`ifdef FIRRTL_AFTER_INITIAL
`FIRRTL_AFTER_INITIAL
`endif
`endif // SYNTHESIS
endmodule
module Arbiter_2(
  output       io_in_0_ready,
  input        io_in_0_valid,
  input  [6:0] io_in_0_bits_setIdx,
  input        io_out_ready,
  output       io_out_valid,
  output [6:0] io_out_bits_setIdx
);
  assign io_in_0_ready = io_out_ready; // @[Arbiter.scala 146:19]
  assign io_out_valid = io_in_0_valid; // @[Arbiter.scala 147:31]
  assign io_out_bits_setIdx = io_in_0_bits_setIdx; // @[Arbiter.scala 136:15]
endmodule
module SRAMTemplateWithArbiter(
  input         clock,
  input         reset,
  output        io_r_0_req_ready,
  input         io_r_0_req_valid,
  input  [6:0]  io_r_0_req_bits_setIdx,
  output [18:0] io_r_0_resp_data_0_tag,
  output        io_r_0_resp_data_0_valid,
  output        io_r_0_resp_data_0_dirty,
  output [18:0] io_r_0_resp_data_1_tag,
  output        io_r_0_resp_data_1_valid,
  output        io_r_0_resp_data_1_dirty,
  output [18:0] io_r_0_resp_data_2_tag,
  output        io_r_0_resp_data_2_valid,
  output        io_r_0_resp_data_2_dirty,
  output [18:0] io_r_0_resp_data_3_tag,
  output        io_r_0_resp_data_3_valid,
  output        io_r_0_resp_data_3_dirty,
  input         io_w_req_valid,
  input  [6:0]  io_w_req_bits_setIdx,
  input  [18:0] io_w_req_bits_data_tag,
  input         io_w_req_bits_data_dirty,
  input  [3:0]  io_w_req_bits_waymask
);
`ifdef RANDOMIZE_REG_INIT
  reg [31:0] _RAND_0;
  reg [31:0] _RAND_1;
  reg [31:0] _RAND_2;
  reg [31:0] _RAND_3;
  reg [31:0] _RAND_4;
  reg [31:0] _RAND_5;
  reg [31:0] _RAND_6;
  reg [31:0] _RAND_7;
  reg [31:0] _RAND_8;
  reg [31:0] _RAND_9;
  reg [31:0] _RAND_10;
  reg [31:0] _RAND_11;
  reg [31:0] _RAND_12;
`endif // RANDOMIZE_REG_INIT
  wire  ram_clock; // @[SRAMTemplate.scala 121:19]
  wire  ram_reset; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_r_req_ready; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_r_req_valid; // @[SRAMTemplate.scala 121:19]
  wire [6:0] ram_io_r_req_bits_setIdx; // @[SRAMTemplate.scala 121:19]
  wire [18:0] ram_io_r_resp_data_0_tag; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_r_resp_data_0_valid; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_r_resp_data_0_dirty; // @[SRAMTemplate.scala 121:19]
  wire [18:0] ram_io_r_resp_data_1_tag; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_r_resp_data_1_valid; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_r_resp_data_1_dirty; // @[SRAMTemplate.scala 121:19]
  wire [18:0] ram_io_r_resp_data_2_tag; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_r_resp_data_2_valid; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_r_resp_data_2_dirty; // @[SRAMTemplate.scala 121:19]
  wire [18:0] ram_io_r_resp_data_3_tag; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_r_resp_data_3_valid; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_r_resp_data_3_dirty; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_w_req_valid; // @[SRAMTemplate.scala 121:19]
  wire [6:0] ram_io_w_req_bits_setIdx; // @[SRAMTemplate.scala 121:19]
  wire [18:0] ram_io_w_req_bits_data_tag; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_w_req_bits_data_dirty; // @[SRAMTemplate.scala 121:19]
  wire [3:0] ram_io_w_req_bits_waymask; // @[SRAMTemplate.scala 121:19]
  wire  readArb_io_in_0_ready; // @[SRAMTemplate.scala 124:23]
  wire  readArb_io_in_0_valid; // @[SRAMTemplate.scala 124:23]
  wire [6:0] readArb_io_in_0_bits_setIdx; // @[SRAMTemplate.scala 124:23]
  wire  readArb_io_out_ready; // @[SRAMTemplate.scala 124:23]
  wire  readArb_io_out_valid; // @[SRAMTemplate.scala 124:23]
  wire [6:0] readArb_io_out_bits_setIdx; // @[SRAMTemplate.scala 124:23]
  reg  REG; // @[SRAMTemplate.scala 130:58]
  reg [18:0] r_0_tag; // @[Reg.scala 35:20]
  reg  r_0_valid; // @[Reg.scala 35:20]
  reg  r_0_dirty; // @[Reg.scala 35:20]
  reg [18:0] r_1_tag; // @[Reg.scala 35:20]
  reg  r_1_valid; // @[Reg.scala 35:20]
  reg  r_1_dirty; // @[Reg.scala 35:20]
  reg [18:0] r_2_tag; // @[Reg.scala 35:20]
  reg  r_2_valid; // @[Reg.scala 35:20]
  reg  r_2_dirty; // @[Reg.scala 35:20]
  reg [18:0] r_3_tag; // @[Reg.scala 35:20]
  reg  r_3_valid; // @[Reg.scala 35:20]
  reg  r_3_dirty; // @[Reg.scala 35:20]
  SRAMTemplate ram ( // @[SRAMTemplate.scala 121:19]
    .clock(ram_clock),
    .reset(ram_reset),
    .io_r_req_ready(ram_io_r_req_ready),
    .io_r_req_valid(ram_io_r_req_valid),
    .io_r_req_bits_setIdx(ram_io_r_req_bits_setIdx),
    .io_r_resp_data_0_tag(ram_io_r_resp_data_0_tag),
    .io_r_resp_data_0_valid(ram_io_r_resp_data_0_valid),
    .io_r_resp_data_0_dirty(ram_io_r_resp_data_0_dirty),
    .io_r_resp_data_1_tag(ram_io_r_resp_data_1_tag),
    .io_r_resp_data_1_valid(ram_io_r_resp_data_1_valid),
    .io_r_resp_data_1_dirty(ram_io_r_resp_data_1_dirty),
    .io_r_resp_data_2_tag(ram_io_r_resp_data_2_tag),
    .io_r_resp_data_2_valid(ram_io_r_resp_data_2_valid),
    .io_r_resp_data_2_dirty(ram_io_r_resp_data_2_dirty),
    .io_r_resp_data_3_tag(ram_io_r_resp_data_3_tag),
    .io_r_resp_data_3_valid(ram_io_r_resp_data_3_valid),
    .io_r_resp_data_3_dirty(ram_io_r_resp_data_3_dirty),
    .io_w_req_valid(ram_io_w_req_valid),
    .io_w_req_bits_setIdx(ram_io_w_req_bits_setIdx),
    .io_w_req_bits_data_tag(ram_io_w_req_bits_data_tag),
    .io_w_req_bits_data_dirty(ram_io_w_req_bits_data_dirty),
    .io_w_req_bits_waymask(ram_io_w_req_bits_waymask)
  );
  Arbiter_2 readArb ( // @[SRAMTemplate.scala 124:23]
    .io_in_0_ready(readArb_io_in_0_ready),
    .io_in_0_valid(readArb_io_in_0_valid),
    .io_in_0_bits_setIdx(readArb_io_in_0_bits_setIdx),
    .io_out_ready(readArb_io_out_ready),
    .io_out_valid(readArb_io_out_valid),
    .io_out_bits_setIdx(readArb_io_out_bits_setIdx)
  );
  assign io_r_0_req_ready = readArb_io_in_0_ready; // @[SRAMTemplate.scala 125:17]
  assign io_r_0_resp_data_0_tag = REG ? ram_io_r_resp_data_0_tag : r_0_tag; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_0_valid = REG ? ram_io_r_resp_data_0_valid : r_0_valid; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_0_dirty = REG ? ram_io_r_resp_data_0_dirty : r_0_dirty; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_1_tag = REG ? ram_io_r_resp_data_1_tag : r_1_tag; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_1_valid = REG ? ram_io_r_resp_data_1_valid : r_1_valid; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_1_dirty = REG ? ram_io_r_resp_data_1_dirty : r_1_dirty; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_2_tag = REG ? ram_io_r_resp_data_2_tag : r_2_tag; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_2_valid = REG ? ram_io_r_resp_data_2_valid : r_2_valid; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_2_dirty = REG ? ram_io_r_resp_data_2_dirty : r_2_dirty; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_3_tag = REG ? ram_io_r_resp_data_3_tag : r_3_tag; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_3_valid = REG ? ram_io_r_resp_data_3_valid : r_3_valid; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_3_dirty = REG ? ram_io_r_resp_data_3_dirty : r_3_dirty; // @[Hold.scala 23:48]
  assign ram_clock = clock;
  assign ram_reset = reset;
  assign ram_io_r_req_valid = readArb_io_out_valid; // @[SRAMTemplate.scala 126:16]
  assign ram_io_r_req_bits_setIdx = readArb_io_out_bits_setIdx; // @[SRAMTemplate.scala 126:16]
  assign ram_io_w_req_valid = io_w_req_valid; // @[SRAMTemplate.scala 122:12]
  assign ram_io_w_req_bits_setIdx = io_w_req_bits_setIdx; // @[SRAMTemplate.scala 122:12]
  assign ram_io_w_req_bits_data_tag = io_w_req_bits_data_tag; // @[SRAMTemplate.scala 122:12]
  assign ram_io_w_req_bits_data_dirty = io_w_req_bits_data_dirty; // @[SRAMTemplate.scala 122:12]
  assign ram_io_w_req_bits_waymask = io_w_req_bits_waymask; // @[SRAMTemplate.scala 122:12]
  assign readArb_io_in_0_valid = io_r_0_req_valid; // @[SRAMTemplate.scala 125:17]
  assign readArb_io_in_0_bits_setIdx = io_r_0_req_bits_setIdx; // @[SRAMTemplate.scala 125:17]
  assign readArb_io_out_ready = ram_io_r_req_ready; // @[SRAMTemplate.scala 126:16]
  always @(posedge clock) begin
    REG <= io_r_0_req_ready & io_r_0_req_valid; // @[Decoupled.scala 51:35]
    if (reset) begin // @[Reg.scala 35:20]
      r_0_tag <= 19'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r_0_tag <= ram_io_r_resp_data_0_tag; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_0_valid <= 1'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r_0_valid <= ram_io_r_resp_data_0_valid; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_0_dirty <= 1'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r_0_dirty <= ram_io_r_resp_data_0_dirty; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_1_tag <= 19'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r_1_tag <= ram_io_r_resp_data_1_tag; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_1_valid <= 1'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r_1_valid <= ram_io_r_resp_data_1_valid; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_1_dirty <= 1'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r_1_dirty <= ram_io_r_resp_data_1_dirty; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_2_tag <= 19'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r_2_tag <= ram_io_r_resp_data_2_tag; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_2_valid <= 1'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r_2_valid <= ram_io_r_resp_data_2_valid; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_2_dirty <= 1'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r_2_dirty <= ram_io_r_resp_data_2_dirty; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_3_tag <= 19'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r_3_tag <= ram_io_r_resp_data_3_tag; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_3_valid <= 1'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r_3_valid <= ram_io_r_resp_data_3_valid; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_3_dirty <= 1'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r_3_dirty <= ram_io_r_resp_data_3_dirty; // @[Reg.scala 36:22]
    end
  end
// Register and memory initialization
`ifdef RANDOMIZE_GARBAGE_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_INVALID_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_REG_INIT
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_MEM_INIT
`define RANDOMIZE
`endif
`ifndef RANDOM
`define RANDOM $random
`endif
`ifdef RANDOMIZE_MEM_INIT
  integer initvar;
`endif
`ifndef SYNTHESIS
`ifdef FIRRTL_BEFORE_INITIAL
`FIRRTL_BEFORE_INITIAL
`endif
initial begin
  `ifdef RANDOMIZE
    `ifdef INIT_RANDOM
      `INIT_RANDOM
    `endif
    `ifndef VERILATOR
      `ifdef RANDOMIZE_DELAY
        #`RANDOMIZE_DELAY begin end
      `else
        #0.002 begin end
      `endif
    `endif
`ifdef RANDOMIZE_REG_INIT
  _RAND_0 = {1{`RANDOM}};
  REG = _RAND_0[0:0];
  _RAND_1 = {1{`RANDOM}};
  r_0_tag = _RAND_1[18:0];
  _RAND_2 = {1{`RANDOM}};
  r_0_valid = _RAND_2[0:0];
  _RAND_3 = {1{`RANDOM}};
  r_0_dirty = _RAND_3[0:0];
  _RAND_4 = {1{`RANDOM}};
  r_1_tag = _RAND_4[18:0];
  _RAND_5 = {1{`RANDOM}};
  r_1_valid = _RAND_5[0:0];
  _RAND_6 = {1{`RANDOM}};
  r_1_dirty = _RAND_6[0:0];
  _RAND_7 = {1{`RANDOM}};
  r_2_tag = _RAND_7[18:0];
  _RAND_8 = {1{`RANDOM}};
  r_2_valid = _RAND_8[0:0];
  _RAND_9 = {1{`RANDOM}};
  r_2_dirty = _RAND_9[0:0];
  _RAND_10 = {1{`RANDOM}};
  r_3_tag = _RAND_10[18:0];
  _RAND_11 = {1{`RANDOM}};
  r_3_valid = _RAND_11[0:0];
  _RAND_12 = {1{`RANDOM}};
  r_3_dirty = _RAND_12[0:0];
`endif // RANDOMIZE_REG_INIT
  `endif // RANDOMIZE
end // initial
`ifdef FIRRTL_AFTER_INITIAL
`FIRRTL_AFTER_INITIAL
`endif
`endif // SYNTHESIS
endmodule
module SRAMTemplate_1(
  input         clock,
  output        io_r_req_ready,
  input         io_r_req_valid,
  input  [9:0]  io_r_req_bits_setIdx,
  output [63:0] io_r_resp_data_0_data,
  output [63:0] io_r_resp_data_1_data,
  output [63:0] io_r_resp_data_2_data,
  output [63:0] io_r_resp_data_3_data,
  input         io_w_req_valid,
  input  [9:0]  io_w_req_bits_setIdx,
  input  [63:0] io_w_req_bits_data_data,
  input  [3:0]  io_w_req_bits_waymask
);
`ifdef RANDOMIZE_MEM_INIT
  reg [63:0] _RAND_0;
  reg [63:0] _RAND_3;
  reg [63:0] _RAND_6;
  reg [63:0] _RAND_9;
`endif // RANDOMIZE_MEM_INIT
`ifdef RANDOMIZE_REG_INIT
  reg [31:0] _RAND_1;
  reg [31:0] _RAND_2;
  reg [31:0] _RAND_4;
  reg [31:0] _RAND_5;
  reg [31:0] _RAND_7;
  reg [31:0] _RAND_8;
  reg [31:0] _RAND_10;
  reg [31:0] _RAND_11;
`endif // RANDOMIZE_REG_INIT
  reg [63:0] array_0 [0:1023]; // @[SRAMTemplate.scala 76:26]
  wire  array_0_rdata_MPORT_en; // @[SRAMTemplate.scala 76:26]
  wire [9:0] array_0_rdata_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire [63:0] array_0_rdata_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [63:0] array_0_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [9:0] array_0_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire  array_0_MPORT_mask; // @[SRAMTemplate.scala 76:26]
  wire  array_0_MPORT_en; // @[SRAMTemplate.scala 76:26]
  reg  array_0_rdata_MPORT_en_pipe_0;
  reg [9:0] array_0_rdata_MPORT_addr_pipe_0;
  reg [63:0] array_1 [0:1023]; // @[SRAMTemplate.scala 76:26]
  wire  array_1_rdata_MPORT_en; // @[SRAMTemplate.scala 76:26]
  wire [9:0] array_1_rdata_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire [63:0] array_1_rdata_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [63:0] array_1_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [9:0] array_1_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire  array_1_MPORT_mask; // @[SRAMTemplate.scala 76:26]
  wire  array_1_MPORT_en; // @[SRAMTemplate.scala 76:26]
  reg  array_1_rdata_MPORT_en_pipe_0;
  reg [9:0] array_1_rdata_MPORT_addr_pipe_0;
  reg [63:0] array_2 [0:1023]; // @[SRAMTemplate.scala 76:26]
  wire  array_2_rdata_MPORT_en; // @[SRAMTemplate.scala 76:26]
  wire [9:0] array_2_rdata_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire [63:0] array_2_rdata_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [63:0] array_2_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [9:0] array_2_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire  array_2_MPORT_mask; // @[SRAMTemplate.scala 76:26]
  wire  array_2_MPORT_en; // @[SRAMTemplate.scala 76:26]
  reg  array_2_rdata_MPORT_en_pipe_0;
  reg [9:0] array_2_rdata_MPORT_addr_pipe_0;
  reg [63:0] array_3 [0:1023]; // @[SRAMTemplate.scala 76:26]
  wire  array_3_rdata_MPORT_en; // @[SRAMTemplate.scala 76:26]
  wire [9:0] array_3_rdata_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire [63:0] array_3_rdata_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [63:0] array_3_MPORT_data; // @[SRAMTemplate.scala 76:26]
  wire [9:0] array_3_MPORT_addr; // @[SRAMTemplate.scala 76:26]
  wire  array_3_MPORT_mask; // @[SRAMTemplate.scala 76:26]
  wire  array_3_MPORT_en; // @[SRAMTemplate.scala 76:26]
  reg  array_3_rdata_MPORT_en_pipe_0;
  reg [9:0] array_3_rdata_MPORT_addr_pipe_0;
  wire  _realRen_T = ~io_w_req_valid; // @[SRAMTemplate.scala 89:41]
  assign array_0_rdata_MPORT_en = array_0_rdata_MPORT_en_pipe_0;
  assign array_0_rdata_MPORT_addr = array_0_rdata_MPORT_addr_pipe_0;
  assign array_0_rdata_MPORT_data = array_0[array_0_rdata_MPORT_addr]; // @[SRAMTemplate.scala 76:26]
  assign array_0_MPORT_data = io_w_req_bits_data_data;
  assign array_0_MPORT_addr = io_w_req_bits_setIdx;
  assign array_0_MPORT_mask = io_w_req_bits_waymask[0];
  assign array_0_MPORT_en = io_w_req_valid;
  assign array_1_rdata_MPORT_en = array_1_rdata_MPORT_en_pipe_0;
  assign array_1_rdata_MPORT_addr = array_1_rdata_MPORT_addr_pipe_0;
  assign array_1_rdata_MPORT_data = array_1[array_1_rdata_MPORT_addr]; // @[SRAMTemplate.scala 76:26]
  assign array_1_MPORT_data = io_w_req_bits_data_data;
  assign array_1_MPORT_addr = io_w_req_bits_setIdx;
  assign array_1_MPORT_mask = io_w_req_bits_waymask[1];
  assign array_1_MPORT_en = io_w_req_valid;
  assign array_2_rdata_MPORT_en = array_2_rdata_MPORT_en_pipe_0;
  assign array_2_rdata_MPORT_addr = array_2_rdata_MPORT_addr_pipe_0;
  assign array_2_rdata_MPORT_data = array_2[array_2_rdata_MPORT_addr]; // @[SRAMTemplate.scala 76:26]
  assign array_2_MPORT_data = io_w_req_bits_data_data;
  assign array_2_MPORT_addr = io_w_req_bits_setIdx;
  assign array_2_MPORT_mask = io_w_req_bits_waymask[2];
  assign array_2_MPORT_en = io_w_req_valid;
  assign array_3_rdata_MPORT_en = array_3_rdata_MPORT_en_pipe_0;
  assign array_3_rdata_MPORT_addr = array_3_rdata_MPORT_addr_pipe_0;
  assign array_3_rdata_MPORT_data = array_3[array_3_rdata_MPORT_addr]; // @[SRAMTemplate.scala 76:26]
  assign array_3_MPORT_data = io_w_req_bits_data_data;
  assign array_3_MPORT_addr = io_w_req_bits_setIdx;
  assign array_3_MPORT_mask = io_w_req_bits_waymask[3];
  assign array_3_MPORT_en = io_w_req_valid;
  assign io_r_req_ready = ~io_w_req_valid; // @[SRAMTemplate.scala 101:53]
  assign io_r_resp_data_0_data = array_0_rdata_MPORT_data; // @[SRAMTemplate.scala 98:{78,78}]
  assign io_r_resp_data_1_data = array_1_rdata_MPORT_data; // @[SRAMTemplate.scala 98:{78,78}]
  assign io_r_resp_data_2_data = array_2_rdata_MPORT_data; // @[SRAMTemplate.scala 98:{78,78}]
  assign io_r_resp_data_3_data = array_3_rdata_MPORT_data; // @[SRAMTemplate.scala 98:{78,78}]
  always @(posedge clock) begin
    if (array_0_MPORT_en & array_0_MPORT_mask) begin
      array_0[array_0_MPORT_addr] <= array_0_MPORT_data; // @[SRAMTemplate.scala 76:26]
    end
    array_0_rdata_MPORT_en_pipe_0 <= io_r_req_valid & _realRen_T;
    if (io_r_req_valid & _realRen_T) begin
      array_0_rdata_MPORT_addr_pipe_0 <= io_r_req_bits_setIdx;
    end
    if (array_1_MPORT_en & array_1_MPORT_mask) begin
      array_1[array_1_MPORT_addr] <= array_1_MPORT_data; // @[SRAMTemplate.scala 76:26]
    end
    array_1_rdata_MPORT_en_pipe_0 <= io_r_req_valid & _realRen_T;
    if (io_r_req_valid & _realRen_T) begin
      array_1_rdata_MPORT_addr_pipe_0 <= io_r_req_bits_setIdx;
    end
    if (array_2_MPORT_en & array_2_MPORT_mask) begin
      array_2[array_2_MPORT_addr] <= array_2_MPORT_data; // @[SRAMTemplate.scala 76:26]
    end
    array_2_rdata_MPORT_en_pipe_0 <= io_r_req_valid & _realRen_T;
    if (io_r_req_valid & _realRen_T) begin
      array_2_rdata_MPORT_addr_pipe_0 <= io_r_req_bits_setIdx;
    end
    if (array_3_MPORT_en & array_3_MPORT_mask) begin
      array_3[array_3_MPORT_addr] <= array_3_MPORT_data; // @[SRAMTemplate.scala 76:26]
    end
    array_3_rdata_MPORT_en_pipe_0 <= io_r_req_valid & _realRen_T;
    if (io_r_req_valid & _realRen_T) begin
      array_3_rdata_MPORT_addr_pipe_0 <= io_r_req_bits_setIdx;
    end
  end
// Register and memory initialization
`ifdef RANDOMIZE_GARBAGE_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_INVALID_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_REG_INIT
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_MEM_INIT
`define RANDOMIZE
`endif
`ifndef RANDOM
`define RANDOM $random
`endif
`ifdef RANDOMIZE_MEM_INIT
  integer initvar;
`endif
`ifndef SYNTHESIS
`ifdef FIRRTL_BEFORE_INITIAL
`FIRRTL_BEFORE_INITIAL
`endif
initial begin
  `ifdef RANDOMIZE
    `ifdef INIT_RANDOM
      `INIT_RANDOM
    `endif
    `ifndef VERILATOR
      `ifdef RANDOMIZE_DELAY
        #`RANDOMIZE_DELAY begin end
      `else
        #0.002 begin end
      `endif
    `endif
`ifdef RANDOMIZE_MEM_INIT
  _RAND_0 = {2{`RANDOM}};
  for (initvar = 0; initvar < 1024; initvar = initvar+1)
    array_0[initvar] = _RAND_0[63:0];
  _RAND_3 = {2{`RANDOM}};
  for (initvar = 0; initvar < 1024; initvar = initvar+1)
    array_1[initvar] = _RAND_3[63:0];
  _RAND_6 = {2{`RANDOM}};
  for (initvar = 0; initvar < 1024; initvar = initvar+1)
    array_2[initvar] = _RAND_6[63:0];
  _RAND_9 = {2{`RANDOM}};
  for (initvar = 0; initvar < 1024; initvar = initvar+1)
    array_3[initvar] = _RAND_9[63:0];
`endif // RANDOMIZE_MEM_INIT
`ifdef RANDOMIZE_REG_INIT
  _RAND_1 = {1{`RANDOM}};
  array_0_rdata_MPORT_en_pipe_0 = _RAND_1[0:0];
  _RAND_2 = {1{`RANDOM}};
  array_0_rdata_MPORT_addr_pipe_0 = _RAND_2[9:0];
  _RAND_4 = {1{`RANDOM}};
  array_1_rdata_MPORT_en_pipe_0 = _RAND_4[0:0];
  _RAND_5 = {1{`RANDOM}};
  array_1_rdata_MPORT_addr_pipe_0 = _RAND_5[9:0];
  _RAND_7 = {1{`RANDOM}};
  array_2_rdata_MPORT_en_pipe_0 = _RAND_7[0:0];
  _RAND_8 = {1{`RANDOM}};
  array_2_rdata_MPORT_addr_pipe_0 = _RAND_8[9:0];
  _RAND_10 = {1{`RANDOM}};
  array_3_rdata_MPORT_en_pipe_0 = _RAND_10[0:0];
  _RAND_11 = {1{`RANDOM}};
  array_3_rdata_MPORT_addr_pipe_0 = _RAND_11[9:0];
`endif // RANDOMIZE_REG_INIT
  `endif // RANDOMIZE
end // initial
`ifdef FIRRTL_AFTER_INITIAL
`FIRRTL_AFTER_INITIAL
`endif
`endif // SYNTHESIS
endmodule
module Arbiter_3(
  output       io_in_0_ready,
  input        io_in_0_valid,
  input  [9:0] io_in_0_bits_setIdx,
  output       io_in_1_ready,
  input        io_in_1_valid,
  input  [9:0] io_in_1_bits_setIdx,
  input        io_out_ready,
  output       io_out_valid,
  output [9:0] io_out_bits_setIdx
);
  wire  grant_1 = ~io_in_0_valid; // @[Arbiter.scala 45:78]
  assign io_in_0_ready = io_out_ready; // @[Arbiter.scala 146:19]
  assign io_in_1_ready = grant_1 & io_out_ready; // @[Arbiter.scala 146:19]
  assign io_out_valid = ~grant_1 | io_in_1_valid; // @[Arbiter.scala 147:31]
  assign io_out_bits_setIdx = io_in_0_valid ? io_in_0_bits_setIdx : io_in_1_bits_setIdx; // @[Arbiter.scala 136:15 138:26 140:19]
endmodule
module SRAMTemplateWithArbiter_1(
  input         clock,
  input         reset,
  output        io_r_0_req_ready,
  input         io_r_0_req_valid,
  input  [9:0]  io_r_0_req_bits_setIdx,
  output [63:0] io_r_0_resp_data_0_data,
  output [63:0] io_r_0_resp_data_1_data,
  output [63:0] io_r_0_resp_data_2_data,
  output [63:0] io_r_0_resp_data_3_data,
  output        io_r_1_req_ready,
  input         io_r_1_req_valid,
  input  [9:0]  io_r_1_req_bits_setIdx,
  output [63:0] io_r_1_resp_data_0_data,
  output [63:0] io_r_1_resp_data_1_data,
  output [63:0] io_r_1_resp_data_2_data,
  output [63:0] io_r_1_resp_data_3_data,
  input         io_w_req_valid,
  input  [9:0]  io_w_req_bits_setIdx,
  input  [63:0] io_w_req_bits_data_data,
  input  [3:0]  io_w_req_bits_waymask
);
`ifdef RANDOMIZE_REG_INIT
  reg [31:0] _RAND_0;
  reg [63:0] _RAND_1;
  reg [63:0] _RAND_2;
  reg [63:0] _RAND_3;
  reg [63:0] _RAND_4;
  reg [31:0] _RAND_5;
  reg [63:0] _RAND_6;
  reg [63:0] _RAND_7;
  reg [63:0] _RAND_8;
  reg [63:0] _RAND_9;
`endif // RANDOMIZE_REG_INIT
  wire  ram_clock; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_r_req_ready; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_r_req_valid; // @[SRAMTemplate.scala 121:19]
  wire [9:0] ram_io_r_req_bits_setIdx; // @[SRAMTemplate.scala 121:19]
  wire [63:0] ram_io_r_resp_data_0_data; // @[SRAMTemplate.scala 121:19]
  wire [63:0] ram_io_r_resp_data_1_data; // @[SRAMTemplate.scala 121:19]
  wire [63:0] ram_io_r_resp_data_2_data; // @[SRAMTemplate.scala 121:19]
  wire [63:0] ram_io_r_resp_data_3_data; // @[SRAMTemplate.scala 121:19]
  wire  ram_io_w_req_valid; // @[SRAMTemplate.scala 121:19]
  wire [9:0] ram_io_w_req_bits_setIdx; // @[SRAMTemplate.scala 121:19]
  wire [63:0] ram_io_w_req_bits_data_data; // @[SRAMTemplate.scala 121:19]
  wire [3:0] ram_io_w_req_bits_waymask; // @[SRAMTemplate.scala 121:19]
  wire  readArb_io_in_0_ready; // @[SRAMTemplate.scala 124:23]
  wire  readArb_io_in_0_valid; // @[SRAMTemplate.scala 124:23]
  wire [9:0] readArb_io_in_0_bits_setIdx; // @[SRAMTemplate.scala 124:23]
  wire  readArb_io_in_1_ready; // @[SRAMTemplate.scala 124:23]
  wire  readArb_io_in_1_valid; // @[SRAMTemplate.scala 124:23]
  wire [9:0] readArb_io_in_1_bits_setIdx; // @[SRAMTemplate.scala 124:23]
  wire  readArb_io_out_ready; // @[SRAMTemplate.scala 124:23]
  wire  readArb_io_out_valid; // @[SRAMTemplate.scala 124:23]
  wire [9:0] readArb_io_out_bits_setIdx; // @[SRAMTemplate.scala 124:23]
  reg  REG; // @[SRAMTemplate.scala 130:58]
  reg [63:0] r__0_data; // @[Reg.scala 35:20]
  reg [63:0] r__1_data; // @[Reg.scala 35:20]
  reg [63:0] r__2_data; // @[Reg.scala 35:20]
  reg [63:0] r__3_data; // @[Reg.scala 35:20]
  reg  REG_1; // @[SRAMTemplate.scala 130:58]
  reg [63:0] r_1_0_data; // @[Reg.scala 35:20]
  reg [63:0] r_1_1_data; // @[Reg.scala 35:20]
  reg [63:0] r_1_2_data; // @[Reg.scala 35:20]
  reg [63:0] r_1_3_data; // @[Reg.scala 35:20]
  SRAMTemplate_1 ram ( // @[SRAMTemplate.scala 121:19]
    .clock(ram_clock),
    .io_r_req_ready(ram_io_r_req_ready),
    .io_r_req_valid(ram_io_r_req_valid),
    .io_r_req_bits_setIdx(ram_io_r_req_bits_setIdx),
    .io_r_resp_data_0_data(ram_io_r_resp_data_0_data),
    .io_r_resp_data_1_data(ram_io_r_resp_data_1_data),
    .io_r_resp_data_2_data(ram_io_r_resp_data_2_data),
    .io_r_resp_data_3_data(ram_io_r_resp_data_3_data),
    .io_w_req_valid(ram_io_w_req_valid),
    .io_w_req_bits_setIdx(ram_io_w_req_bits_setIdx),
    .io_w_req_bits_data_data(ram_io_w_req_bits_data_data),
    .io_w_req_bits_waymask(ram_io_w_req_bits_waymask)
  );
  Arbiter_3 readArb ( // @[SRAMTemplate.scala 124:23]
    .io_in_0_ready(readArb_io_in_0_ready),
    .io_in_0_valid(readArb_io_in_0_valid),
    .io_in_0_bits_setIdx(readArb_io_in_0_bits_setIdx),
    .io_in_1_ready(readArb_io_in_1_ready),
    .io_in_1_valid(readArb_io_in_1_valid),
    .io_in_1_bits_setIdx(readArb_io_in_1_bits_setIdx),
    .io_out_ready(readArb_io_out_ready),
    .io_out_valid(readArb_io_out_valid),
    .io_out_bits_setIdx(readArb_io_out_bits_setIdx)
  );
  assign io_r_0_req_ready = readArb_io_in_0_ready; // @[SRAMTemplate.scala 125:17]
  assign io_r_0_resp_data_0_data = REG ? ram_io_r_resp_data_0_data : r__0_data; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_1_data = REG ? ram_io_r_resp_data_1_data : r__1_data; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_2_data = REG ? ram_io_r_resp_data_2_data : r__2_data; // @[Hold.scala 23:48]
  assign io_r_0_resp_data_3_data = REG ? ram_io_r_resp_data_3_data : r__3_data; // @[Hold.scala 23:48]
  assign io_r_1_req_ready = readArb_io_in_1_ready; // @[SRAMTemplate.scala 125:17]
  assign io_r_1_resp_data_0_data = REG_1 ? ram_io_r_resp_data_0_data : r_1_0_data; // @[Hold.scala 23:48]
  assign io_r_1_resp_data_1_data = REG_1 ? ram_io_r_resp_data_1_data : r_1_1_data; // @[Hold.scala 23:48]
  assign io_r_1_resp_data_2_data = REG_1 ? ram_io_r_resp_data_2_data : r_1_2_data; // @[Hold.scala 23:48]
  assign io_r_1_resp_data_3_data = REG_1 ? ram_io_r_resp_data_3_data : r_1_3_data; // @[Hold.scala 23:48]
  assign ram_clock = clock;
  assign ram_io_r_req_valid = readArb_io_out_valid; // @[SRAMTemplate.scala 126:16]
  assign ram_io_r_req_bits_setIdx = readArb_io_out_bits_setIdx; // @[SRAMTemplate.scala 126:16]
  assign ram_io_w_req_valid = io_w_req_valid; // @[SRAMTemplate.scala 122:12]
  assign ram_io_w_req_bits_setIdx = io_w_req_bits_setIdx; // @[SRAMTemplate.scala 122:12]
  assign ram_io_w_req_bits_data_data = io_w_req_bits_data_data; // @[SRAMTemplate.scala 122:12]
  assign ram_io_w_req_bits_waymask = io_w_req_bits_waymask; // @[SRAMTemplate.scala 122:12]
  assign readArb_io_in_0_valid = io_r_0_req_valid; // @[SRAMTemplate.scala 125:17]
  assign readArb_io_in_0_bits_setIdx = io_r_0_req_bits_setIdx; // @[SRAMTemplate.scala 125:17]
  assign readArb_io_in_1_valid = io_r_1_req_valid; // @[SRAMTemplate.scala 125:17]
  assign readArb_io_in_1_bits_setIdx = io_r_1_req_bits_setIdx; // @[SRAMTemplate.scala 125:17]
  assign readArb_io_out_ready = ram_io_r_req_ready; // @[SRAMTemplate.scala 126:16]
  always @(posedge clock) begin
    REG <= io_r_0_req_ready & io_r_0_req_valid; // @[Decoupled.scala 51:35]
    if (reset) begin // @[Reg.scala 35:20]
      r__0_data <= 64'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r__0_data <= ram_io_r_resp_data_0_data; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r__1_data <= 64'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r__1_data <= ram_io_r_resp_data_1_data; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r__2_data <= 64'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r__2_data <= ram_io_r_resp_data_2_data; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r__3_data <= 64'h0; // @[Reg.scala 35:20]
    end else if (REG) begin // @[Reg.scala 36:18]
      r__3_data <= ram_io_r_resp_data_3_data; // @[Reg.scala 36:22]
    end
    REG_1 <= io_r_1_req_ready & io_r_1_req_valid; // @[Decoupled.scala 51:35]
    if (reset) begin // @[Reg.scala 35:20]
      r_1_0_data <= 64'h0; // @[Reg.scala 35:20]
    end else if (REG_1) begin // @[Reg.scala 36:18]
      r_1_0_data <= ram_io_r_resp_data_0_data; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_1_1_data <= 64'h0; // @[Reg.scala 35:20]
    end else if (REG_1) begin // @[Reg.scala 36:18]
      r_1_1_data <= ram_io_r_resp_data_1_data; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_1_2_data <= 64'h0; // @[Reg.scala 35:20]
    end else if (REG_1) begin // @[Reg.scala 36:18]
      r_1_2_data <= ram_io_r_resp_data_2_data; // @[Reg.scala 36:22]
    end
    if (reset) begin // @[Reg.scala 35:20]
      r_1_3_data <= 64'h0; // @[Reg.scala 35:20]
    end else if (REG_1) begin // @[Reg.scala 36:18]
      r_1_3_data <= ram_io_r_resp_data_3_data; // @[Reg.scala 36:22]
    end
  end
// Register and memory initialization
`ifdef RANDOMIZE_GARBAGE_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_INVALID_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_REG_INIT
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_MEM_INIT
`define RANDOMIZE
`endif
`ifndef RANDOM
`define RANDOM $random
`endif
`ifdef RANDOMIZE_MEM_INIT
  integer initvar;
`endif
`ifndef SYNTHESIS
`ifdef FIRRTL_BEFORE_INITIAL
`FIRRTL_BEFORE_INITIAL
`endif
initial begin
  `ifdef RANDOMIZE
    `ifdef INIT_RANDOM
      `INIT_RANDOM
    `endif
    `ifndef VERILATOR
      `ifdef RANDOMIZE_DELAY
        #`RANDOMIZE_DELAY begin end
      `else
        #0.002 begin end
      `endif
    `endif
`ifdef RANDOMIZE_REG_INIT
  _RAND_0 = {1{`RANDOM}};
  REG = _RAND_0[0:0];
  _RAND_1 = {2{`RANDOM}};
  r__0_data = _RAND_1[63:0];
  _RAND_2 = {2{`RANDOM}};
  r__1_data = _RAND_2[63:0];
  _RAND_3 = {2{`RANDOM}};
  r__2_data = _RAND_3[63:0];
  _RAND_4 = {2{`RANDOM}};
  r__3_data = _RAND_4[63:0];
  _RAND_5 = {1{`RANDOM}};
  REG_1 = _RAND_5[0:0];
  _RAND_6 = {2{`RANDOM}};
  r_1_0_data = _RAND_6[63:0];
  _RAND_7 = {2{`RANDOM}};
  r_1_1_data = _RAND_7[63:0];
  _RAND_8 = {2{`RANDOM}};
  r_1_2_data = _RAND_8[63:0];
  _RAND_9 = {2{`RANDOM}};
  r_1_3_data = _RAND_9[63:0];
`endif // RANDOMIZE_REG_INIT
  `endif // RANDOMIZE
end // initial
`ifdef FIRRTL_AFTER_INITIAL
`FIRRTL_AFTER_INITIAL
`endif
`endif // SYNTHESIS
endmodule
module Arbiter_4(
  output        io_in_0_ready,
  input         io_in_0_valid,
  input  [31:0] io_in_0_bits_addr,
  input  [2:0]  io_in_0_bits_size,
  input  [3:0]  io_in_0_bits_cmd,
  input  [7:0]  io_in_0_bits_wmask,
  input  [63:0] io_in_0_bits_wdata,
  output        io_in_1_ready,
  input         io_in_1_valid,
  input  [31:0] io_in_1_bits_addr,
  input  [2:0]  io_in_1_bits_size,
  input  [3:0]  io_in_1_bits_cmd,
  input  [7:0]  io_in_1_bits_wmask,
  input  [63:0] io_in_1_bits_wdata,
  input  [15:0] io_in_1_bits_user,
  input         io_out_ready,
  output        io_out_valid,
  output [31:0] io_out_bits_addr,
  output [2:0]  io_out_bits_size,
  output [3:0]  io_out_bits_cmd,
  output [7:0]  io_out_bits_wmask,
  output [63:0] io_out_bits_wdata,
  output [15:0] io_out_bits_user
);
  wire  grant_1 = ~io_in_0_valid; // @[Arbiter.scala 45:78]
  assign io_in_0_ready = io_out_ready; // @[Arbiter.scala 146:19]
  assign io_in_1_ready = grant_1 & io_out_ready; // @[Arbiter.scala 146:19]
  assign io_out_valid = ~grant_1 | io_in_1_valid; // @[Arbiter.scala 147:31]
  assign io_out_bits_addr = io_in_0_valid ? io_in_0_bits_addr : io_in_1_bits_addr; // @[Arbiter.scala 136:15 138:26 140:19]
  assign io_out_bits_size = io_in_0_valid ? io_in_0_bits_size : io_in_1_bits_size; // @[Arbiter.scala 136:15 138:26 140:19]
  assign io_out_bits_cmd = io_in_0_valid ? io_in_0_bits_cmd : io_in_1_bits_cmd; // @[Arbiter.scala 136:15 138:26 140:19]
  assign io_out_bits_wmask = io_in_0_valid ? io_in_0_bits_wmask : io_in_1_bits_wmask; // @[Arbiter.scala 136:15 138:26 140:19]
  assign io_out_bits_wdata = io_in_0_valid ? io_in_0_bits_wdata : io_in_1_bits_wdata; // @[Arbiter.scala 136:15 138:26 140:19]
  assign io_out_bits_user = io_in_0_valid ? 16'h0 : io_in_1_bits_user; // @[Arbiter.scala 136:15 138:26 140:19]
endmodule
module Cache(
  input         clock,
  input         reset,
  output        io_in_req_ready,
  input         io_in_req_valid,
  input  [31:0] io_in_req_bits_addr,
  input  [2:0]  io_in_req_bits_size,
  input  [3:0]  io_in_req_bits_cmd,
  input  [7:0]  io_in_req_bits_wmask,
  input  [63:0] io_in_req_bits_wdata,
  input  [15:0] io_in_req_bits_user,
  input         io_in_resp_ready,
  output        io_in_resp_valid,
  output [3:0]  io_in_resp_bits_cmd,
  output [63:0] io_in_resp_bits_rdata,
  output [15:0] io_in_resp_bits_user,
  input  [1:0]  io_flush,
  input         io_out_mem_req_ready,
  output        io_out_mem_req_valid,
  output [31:0] io_out_mem_req_bits_addr,
  output [2:0]  io_out_mem_req_bits_size,
  output [3:0]  io_out_mem_req_bits_cmd,
  output [7:0]  io_out_mem_req_bits_wmask,
  output [63:0] io_out_mem_req_bits_wdata,
  output        io_out_mem_resp_ready,
  input         io_out_mem_resp_valid,
  input  [3:0]  io_out_mem_resp_bits_cmd,
  input  [63:0] io_out_mem_resp_bits_rdata,
  output        io_out_coh_req_ready,
  input         io_out_coh_req_valid,
  input  [31:0] io_out_coh_req_bits_addr,
  input  [2:0]  io_out_coh_req_bits_size,
  input  [3:0]  io_out_coh_req_bits_cmd,
  input  [7:0]  io_out_coh_req_bits_wmask,
  input  [63:0] io_out_coh_req_bits_wdata,
  input         io_out_coh_resp_ready,
  output        io_out_coh_resp_valid,
  output [3:0]  io_out_coh_resp_bits_cmd,
  output [63:0] io_out_coh_resp_bits_rdata,
  input         io_mmio_req_ready,
  output        io_mmio_req_valid,
  output [31:0] io_mmio_req_bits_addr,
  output [2:0]  io_mmio_req_bits_size,
  output [3:0]  io_mmio_req_bits_cmd,
  output [7:0]  io_mmio_req_bits_wmask,
  output [63:0] io_mmio_req_bits_wdata,
  output        io_mmio_resp_ready,
  input         io_mmio_resp_valid,
  input  [3:0]  io_mmio_resp_bits_cmd,
  input  [63:0] io_mmio_resp_bits_rdata,
  output        io_empty
);
`ifdef RANDOMIZE_REG_INIT
  reg [31:0] _RAND_0;
  reg [31:0] _RAND_1;
  reg [31:0] _RAND_2;
  reg [31:0] _RAND_3;
  reg [31:0] _RAND_4;
  reg [63:0] _RAND_5;
  reg [31:0] _RAND_6;
  reg [31:0] _RAND_7;
  reg [31:0] _RAND_8;
  reg [31:0] _RAND_9;
  reg [31:0] _RAND_10;
  reg [31:0] _RAND_11;
  reg [63:0] _RAND_12;
  reg [31:0] _RAND_13;
  reg [31:0] _RAND_14;
  reg [31:0] _RAND_15;
  reg [31:0] _RAND_16;
  reg [31:0] _RAND_17;
  reg [31:0] _RAND_18;
  reg [31:0] _RAND_19;
  reg [31:0] _RAND_20;
  reg [31:0] _RAND_21;
  reg [63:0] _RAND_22;
  reg [63:0] _RAND_23;
  reg [63:0] _RAND_24;
  reg [63:0] _RAND_25;
  reg [31:0] _RAND_26;
  reg [31:0] _RAND_27;
  reg [31:0] _RAND_28;
  reg [31:0] _RAND_29;
  reg [63:0] _RAND_30;
  reg [31:0] _RAND_31;
`endif // RANDOMIZE_REG_INIT
  wire  s1_io_in_ready; // @[Cache.scala 480:18]
  wire  s1_io_in_valid; // @[Cache.scala 480:18]
  wire [31:0] s1_io_in_bits_addr; // @[Cache.scala 480:18]
  wire [2:0] s1_io_in_bits_size; // @[Cache.scala 480:18]
  wire [3:0] s1_io_in_bits_cmd; // @[Cache.scala 480:18]
  wire [7:0] s1_io_in_bits_wmask; // @[Cache.scala 480:18]
  wire [63:0] s1_io_in_bits_wdata; // @[Cache.scala 480:18]
  wire [15:0] s1_io_in_bits_user; // @[Cache.scala 480:18]
  wire  s1_io_out_ready; // @[Cache.scala 480:18]
  wire  s1_io_out_valid; // @[Cache.scala 480:18]
  wire [31:0] s1_io_out_bits_req_addr; // @[Cache.scala 480:18]
  wire [2:0] s1_io_out_bits_req_size; // @[Cache.scala 480:18]
  wire [3:0] s1_io_out_bits_req_cmd; // @[Cache.scala 480:18]
  wire [7:0] s1_io_out_bits_req_wmask; // @[Cache.scala 480:18]
  wire [63:0] s1_io_out_bits_req_wdata; // @[Cache.scala 480:18]
  wire [15:0] s1_io_out_bits_req_user; // @[Cache.scala 480:18]
  wire  s1_io_metaReadBus_req_ready; // @[Cache.scala 480:18]
  wire  s1_io_metaReadBus_req_valid; // @[Cache.scala 480:18]
  wire [6:0] s1_io_metaReadBus_req_bits_setIdx; // @[Cache.scala 480:18]
  wire [18:0] s1_io_metaReadBus_resp_data_0_tag; // @[Cache.scala 480:18]
  wire  s1_io_metaReadBus_resp_data_0_valid; // @[Cache.scala 480:18]
  wire  s1_io_metaReadBus_resp_data_0_dirty; // @[Cache.scala 480:18]
  wire [18:0] s1_io_metaReadBus_resp_data_1_tag; // @[Cache.scala 480:18]
  wire  s1_io_metaReadBus_resp_data_1_valid; // @[Cache.scala 480:18]
  wire  s1_io_metaReadBus_resp_data_1_dirty; // @[Cache.scala 480:18]
  wire [18:0] s1_io_metaReadBus_resp_data_2_tag; // @[Cache.scala 480:18]
  wire  s1_io_metaReadBus_resp_data_2_valid; // @[Cache.scala 480:18]
  wire  s1_io_metaReadBus_resp_data_2_dirty; // @[Cache.scala 480:18]
  wire [18:0] s1_io_metaReadBus_resp_data_3_tag; // @[Cache.scala 480:18]
  wire  s1_io_metaReadBus_resp_data_3_valid; // @[Cache.scala 480:18]
  wire  s1_io_metaReadBus_resp_data_3_dirty; // @[Cache.scala 480:18]
  wire  s1_io_dataReadBus_req_ready; // @[Cache.scala 480:18]
  wire  s1_io_dataReadBus_req_valid; // @[Cache.scala 480:18]
  wire [9:0] s1_io_dataReadBus_req_bits_setIdx; // @[Cache.scala 480:18]
  wire [63:0] s1_io_dataReadBus_resp_data_0_data; // @[Cache.scala 480:18]
  wire [63:0] s1_io_dataReadBus_resp_data_1_data; // @[Cache.scala 480:18]
  wire [63:0] s1_io_dataReadBus_resp_data_2_data; // @[Cache.scala 480:18]
  wire [63:0] s1_io_dataReadBus_resp_data_3_data; // @[Cache.scala 480:18]
  wire  s2_clock; // @[Cache.scala 481:18]
  wire  s2_reset; // @[Cache.scala 481:18]
  wire  s2_io_in_ready; // @[Cache.scala 481:18]
  wire  s2_io_in_valid; // @[Cache.scala 481:18]
  wire [31:0] s2_io_in_bits_req_addr; // @[Cache.scala 481:18]
  wire [2:0] s2_io_in_bits_req_size; // @[Cache.scala 481:18]
  wire [3:0] s2_io_in_bits_req_cmd; // @[Cache.scala 481:18]
  wire [7:0] s2_io_in_bits_req_wmask; // @[Cache.scala 481:18]
  wire [63:0] s2_io_in_bits_req_wdata; // @[Cache.scala 481:18]
  wire [15:0] s2_io_in_bits_req_user; // @[Cache.scala 481:18]
  wire  s2_io_out_ready; // @[Cache.scala 481:18]
  wire  s2_io_out_valid; // @[Cache.scala 481:18]
  wire [31:0] s2_io_out_bits_req_addr; // @[Cache.scala 481:18]
  wire [2:0] s2_io_out_bits_req_size; // @[Cache.scala 481:18]
  wire [3:0] s2_io_out_bits_req_cmd; // @[Cache.scala 481:18]
  wire [7:0] s2_io_out_bits_req_wmask; // @[Cache.scala 481:18]
  wire [63:0] s2_io_out_bits_req_wdata; // @[Cache.scala 481:18]
  wire [15:0] s2_io_out_bits_req_user; // @[Cache.scala 481:18]
  wire [18:0] s2_io_out_bits_metas_0_tag; // @[Cache.scala 481:18]
  wire  s2_io_out_bits_metas_0_dirty; // @[Cache.scala 481:18]
  wire [18:0] s2_io_out_bits_metas_1_tag; // @[Cache.scala 481:18]
  wire  s2_io_out_bits_metas_1_dirty; // @[Cache.scala 481:18]
  wire [18:0] s2_io_out_bits_metas_2_tag; // @[Cache.scala 481:18]
  wire  s2_io_out_bits_metas_2_dirty; // @[Cache.scala 481:18]
  wire [18:0] s2_io_out_bits_metas_3_tag; // @[Cache.scala 481:18]
  wire  s2_io_out_bits_metas_3_dirty; // @[Cache.scala 481:18]
  wire [63:0] s2_io_out_bits_datas_0_data; // @[Cache.scala 481:18]
  wire [63:0] s2_io_out_bits_datas_1_data; // @[Cache.scala 481:18]
  wire [63:0] s2_io_out_bits_datas_2_data; // @[Cache.scala 481:18]
  wire [63:0] s2_io_out_bits_datas_3_data; // @[Cache.scala 481:18]
  wire  s2_io_out_bits_hit; // @[Cache.scala 481:18]
  wire [3:0] s2_io_out_bits_waymask; // @[Cache.scala 481:18]
  wire  s2_io_out_bits_mmio; // @[Cache.scala 481:18]
  wire  s2_io_out_bits_isForwardData; // @[Cache.scala 481:18]
  wire [63:0] s2_io_out_bits_forwardData_data_data; // @[Cache.scala 481:18]
  wire [3:0] s2_io_out_bits_forwardData_waymask; // @[Cache.scala 481:18]
  wire [18:0] s2_io_metaReadResp_0_tag; // @[Cache.scala 481:18]
  wire  s2_io_metaReadResp_0_valid; // @[Cache.scala 481:18]
  wire  s2_io_metaReadResp_0_dirty; // @[Cache.scala 481:18]
  wire [18:0] s2_io_metaReadResp_1_tag; // @[Cache.scala 481:18]
  wire  s2_io_metaReadResp_1_valid; // @[Cache.scala 481:18]
  wire  s2_io_metaReadResp_1_dirty; // @[Cache.scala 481:18]
  wire [18:0] s2_io_metaReadResp_2_tag; // @[Cache.scala 481:18]
  wire  s2_io_metaReadResp_2_valid; // @[Cache.scala 481:18]
  wire  s2_io_metaReadResp_2_dirty; // @[Cache.scala 481:18]
  wire [18:0] s2_io_metaReadResp_3_tag; // @[Cache.scala 481:18]
  wire  s2_io_metaReadResp_3_valid; // @[Cache.scala 481:18]
  wire  s2_io_metaReadResp_3_dirty; // @[Cache.scala 481:18]
  wire [63:0] s2_io_dataReadResp_0_data; // @[Cache.scala 481:18]
  wire [63:0] s2_io_dataReadResp_1_data; // @[Cache.scala 481:18]
  wire [63:0] s2_io_dataReadResp_2_data; // @[Cache.scala 481:18]
  wire [63:0] s2_io_dataReadResp_3_data; // @[Cache.scala 481:18]
  wire  s2_io_metaWriteBus_req_valid; // @[Cache.scala 481:18]
  wire [6:0] s2_io_metaWriteBus_req_bits_setIdx; // @[Cache.scala 481:18]
  wire [18:0] s2_io_metaWriteBus_req_bits_data_tag; // @[Cache.scala 481:18]
  wire  s2_io_metaWriteBus_req_bits_data_dirty; // @[Cache.scala 481:18]
  wire [3:0] s2_io_metaWriteBus_req_bits_waymask; // @[Cache.scala 481:18]
  wire  s2_io_dataWriteBus_req_valid; // @[Cache.scala 481:18]
  wire [9:0] s2_io_dataWriteBus_req_bits_setIdx; // @[Cache.scala 481:18]
  wire [63:0] s2_io_dataWriteBus_req_bits_data_data; // @[Cache.scala 481:18]
  wire [3:0] s2_io_dataWriteBus_req_bits_waymask; // @[Cache.scala 481:18]
  wire  s3_clock; // @[Cache.scala 482:18]
  wire  s3_reset; // @[Cache.scala 482:18]
  wire  s3_io_in_ready; // @[Cache.scala 482:18]
  wire  s3_io_in_valid; // @[Cache.scala 482:18]
  wire [31:0] s3_io_in_bits_req_addr; // @[Cache.scala 482:18]
  wire [2:0] s3_io_in_bits_req_size; // @[Cache.scala 482:18]
  wire [3:0] s3_io_in_bits_req_cmd; // @[Cache.scala 482:18]
  wire [7:0] s3_io_in_bits_req_wmask; // @[Cache.scala 482:18]
  wire [63:0] s3_io_in_bits_req_wdata; // @[Cache.scala 482:18]
  wire [15:0] s3_io_in_bits_req_user; // @[Cache.scala 482:18]
  wire [18:0] s3_io_in_bits_metas_0_tag; // @[Cache.scala 482:18]
  wire  s3_io_in_bits_metas_0_dirty; // @[Cache.scala 482:18]
  wire [18:0] s3_io_in_bits_metas_1_tag; // @[Cache.scala 482:18]
  wire  s3_io_in_bits_metas_1_dirty; // @[Cache.scala 482:18]
  wire [18:0] s3_io_in_bits_metas_2_tag; // @[Cache.scala 482:18]
  wire  s3_io_in_bits_metas_2_dirty; // @[Cache.scala 482:18]
  wire [18:0] s3_io_in_bits_metas_3_tag; // @[Cache.scala 482:18]
  wire  s3_io_in_bits_metas_3_dirty; // @[Cache.scala 482:18]
  wire [63:0] s3_io_in_bits_datas_0_data; // @[Cache.scala 482:18]
  wire [63:0] s3_io_in_bits_datas_1_data; // @[Cache.scala 482:18]
  wire [63:0] s3_io_in_bits_datas_2_data; // @[Cache.scala 482:18]
  wire [63:0] s3_io_in_bits_datas_3_data; // @[Cache.scala 482:18]
  wire  s3_io_in_bits_hit; // @[Cache.scala 482:18]
  wire [3:0] s3_io_in_bits_waymask; // @[Cache.scala 482:18]
  wire  s3_io_in_bits_mmio; // @[Cache.scala 482:18]
  wire  s3_io_in_bits_isForwardData; // @[Cache.scala 482:18]
  wire [63:0] s3_io_in_bits_forwardData_data_data; // @[Cache.scala 482:18]
  wire [3:0] s3_io_in_bits_forwardData_waymask; // @[Cache.scala 482:18]
  wire  s3_io_out_ready; // @[Cache.scala 482:18]
  wire  s3_io_out_valid; // @[Cache.scala 482:18]
  wire [3:0] s3_io_out_bits_cmd; // @[Cache.scala 482:18]
  wire [63:0] s3_io_out_bits_rdata; // @[Cache.scala 482:18]
  wire [15:0] s3_io_out_bits_user; // @[Cache.scala 482:18]
  wire  s3_io_isFinish; // @[Cache.scala 482:18]
  wire  s3_io_flush; // @[Cache.scala 482:18]
  wire  s3_io_dataReadBus_req_ready; // @[Cache.scala 482:18]
  wire  s3_io_dataReadBus_req_valid; // @[Cache.scala 482:18]
  wire [9:0] s3_io_dataReadBus_req_bits_setIdx; // @[Cache.scala 482:18]
  wire [63:0] s3_io_dataReadBus_resp_data_0_data; // @[Cache.scala 482:18]
  wire [63:0] s3_io_dataReadBus_resp_data_1_data; // @[Cache.scala 482:18]
  wire [63:0] s3_io_dataReadBus_resp_data_2_data; // @[Cache.scala 482:18]
  wire [63:0] s3_io_dataReadBus_resp_data_3_data; // @[Cache.scala 482:18]
  wire  s3_io_dataWriteBus_req_valid; // @[Cache.scala 482:18]
  wire [9:0] s3_io_dataWriteBus_req_bits_setIdx; // @[Cache.scala 482:18]
  wire [63:0] s3_io_dataWriteBus_req_bits_data_data; // @[Cache.scala 482:18]
  wire [3:0] s3_io_dataWriteBus_req_bits_waymask; // @[Cache.scala 482:18]
  wire  s3_io_metaWriteBus_req_valid; // @[Cache.scala 482:18]
  wire [6:0] s3_io_metaWriteBus_req_bits_setIdx; // @[Cache.scala 482:18]
  wire [18:0] s3_io_metaWriteBus_req_bits_data_tag; // @[Cache.scala 482:18]
  wire  s3_io_metaWriteBus_req_bits_data_dirty; // @[Cache.scala 482:18]
  wire [3:0] s3_io_metaWriteBus_req_bits_waymask; // @[Cache.scala 482:18]
  wire  s3_io_mem_req_ready; // @[Cache.scala 482:18]
  wire  s3_io_mem_req_valid; // @[Cache.scala 482:18]
  wire [31:0] s3_io_mem_req_bits_addr; // @[Cache.scala 482:18]
  wire [3:0] s3_io_mem_req_bits_cmd; // @[Cache.scala 482:18]
  wire [63:0] s3_io_mem_req_bits_wdata; // @[Cache.scala 482:18]
  wire  s3_io_mem_resp_ready; // @[Cache.scala 482:18]
  wire  s3_io_mem_resp_valid; // @[Cache.scala 482:18]
  wire [3:0] s3_io_mem_resp_bits_cmd; // @[Cache.scala 482:18]
  wire [63:0] s3_io_mem_resp_bits_rdata; // @[Cache.scala 482:18]
  wire  s3_io_mmio_req_ready; // @[Cache.scala 482:18]
  wire  s3_io_mmio_req_valid; // @[Cache.scala 482:18]
  wire [31:0] s3_io_mmio_req_bits_addr; // @[Cache.scala 482:18]
  wire [2:0] s3_io_mmio_req_bits_size; // @[Cache.scala 482:18]
  wire [3:0] s3_io_mmio_req_bits_cmd; // @[Cache.scala 482:18]
  wire [7:0] s3_io_mmio_req_bits_wmask; // @[Cache.scala 482:18]
  wire [63:0] s3_io_mmio_req_bits_wdata; // @[Cache.scala 482:18]
  wire  s3_io_mmio_resp_ready; // @[Cache.scala 482:18]
  wire  s3_io_mmio_resp_valid; // @[Cache.scala 482:18]
  wire [63:0] s3_io_mmio_resp_bits_rdata; // @[Cache.scala 482:18]
  wire  s3_io_cohResp_ready; // @[Cache.scala 482:18]
  wire  s3_io_cohResp_valid; // @[Cache.scala 482:18]
  wire [3:0] s3_io_cohResp_bits_cmd; // @[Cache.scala 482:18]
  wire [63:0] s3_io_cohResp_bits_rdata; // @[Cache.scala 482:18]
  wire  s3_io_dataReadRespToL1; // @[Cache.scala 482:18]
  wire  metaArray_clock; // @[Cache.scala 483:25]
  wire  metaArray_reset; // @[Cache.scala 483:25]
  wire  metaArray_io_r_0_req_ready; // @[Cache.scala 483:25]
  wire  metaArray_io_r_0_req_valid; // @[Cache.scala 483:25]
  wire [6:0] metaArray_io_r_0_req_bits_setIdx; // @[Cache.scala 483:25]
  wire [18:0] metaArray_io_r_0_resp_data_0_tag; // @[Cache.scala 483:25]
  wire  metaArray_io_r_0_resp_data_0_valid; // @[Cache.scala 483:25]
  wire  metaArray_io_r_0_resp_data_0_dirty; // @[Cache.scala 483:25]
  wire [18:0] metaArray_io_r_0_resp_data_1_tag; // @[Cache.scala 483:25]
  wire  metaArray_io_r_0_resp_data_1_valid; // @[Cache.scala 483:25]
  wire  metaArray_io_r_0_resp_data_1_dirty; // @[Cache.scala 483:25]
  wire [18:0] metaArray_io_r_0_resp_data_2_tag; // @[Cache.scala 483:25]
  wire  metaArray_io_r_0_resp_data_2_valid; // @[Cache.scala 483:25]
  wire  metaArray_io_r_0_resp_data_2_dirty; // @[Cache.scala 483:25]
  wire [18:0] metaArray_io_r_0_resp_data_3_tag; // @[Cache.scala 483:25]
  wire  metaArray_io_r_0_resp_data_3_valid; // @[Cache.scala 483:25]
  wire  metaArray_io_r_0_resp_data_3_dirty; // @[Cache.scala 483:25]
  wire  metaArray_io_w_req_valid; // @[Cache.scala 483:25]
  wire [6:0] metaArray_io_w_req_bits_setIdx; // @[Cache.scala 483:25]
  wire [18:0] metaArray_io_w_req_bits_data_tag; // @[Cache.scala 483:25]
  wire  metaArray_io_w_req_bits_data_dirty; // @[Cache.scala 483:25]
  wire [3:0] metaArray_io_w_req_bits_waymask; // @[Cache.scala 483:25]
  wire  dataArray_clock; // @[Cache.scala 484:25]
  wire  dataArray_reset; // @[Cache.scala 484:25]
  wire  dataArray_io_r_0_req_ready; // @[Cache.scala 484:25]
  wire  dataArray_io_r_0_req_valid; // @[Cache.scala 484:25]
  wire [9:0] dataArray_io_r_0_req_bits_setIdx; // @[Cache.scala 484:25]
  wire [63:0] dataArray_io_r_0_resp_data_0_data; // @[Cache.scala 484:25]
  wire [63:0] dataArray_io_r_0_resp_data_1_data; // @[Cache.scala 484:25]
  wire [63:0] dataArray_io_r_0_resp_data_2_data; // @[Cache.scala 484:25]
  wire [63:0] dataArray_io_r_0_resp_data_3_data; // @[Cache.scala 484:25]
  wire  dataArray_io_r_1_req_ready; // @[Cache.scala 484:25]
  wire  dataArray_io_r_1_req_valid; // @[Cache.scala 484:25]
  wire [9:0] dataArray_io_r_1_req_bits_setIdx; // @[Cache.scala 484:25]
  wire [63:0] dataArray_io_r_1_resp_data_0_data; // @[Cache.scala 484:25]
  wire [63:0] dataArray_io_r_1_resp_data_1_data; // @[Cache.scala 484:25]
  wire [63:0] dataArray_io_r_1_resp_data_2_data; // @[Cache.scala 484:25]
  wire [63:0] dataArray_io_r_1_resp_data_3_data; // @[Cache.scala 484:25]
  wire  dataArray_io_w_req_valid; // @[Cache.scala 484:25]
  wire [9:0] dataArray_io_w_req_bits_setIdx; // @[Cache.scala 484:25]
  wire [63:0] dataArray_io_w_req_bits_data_data; // @[Cache.scala 484:25]
  wire [3:0] dataArray_io_w_req_bits_waymask; // @[Cache.scala 484:25]
  wire  arb_io_in_0_ready; // @[Cache.scala 493:19]
  wire  arb_io_in_0_valid; // @[Cache.scala 493:19]
  wire [31:0] arb_io_in_0_bits_addr; // @[Cache.scala 493:19]
  wire [2:0] arb_io_in_0_bits_size; // @[Cache.scala 493:19]
  wire [3:0] arb_io_in_0_bits_cmd; // @[Cache.scala 493:19]
  wire [7:0] arb_io_in_0_bits_wmask; // @[Cache.scala 493:19]
  wire [63:0] arb_io_in_0_bits_wdata; // @[Cache.scala 493:19]
  wire  arb_io_in_1_ready; // @[Cache.scala 493:19]
  wire  arb_io_in_1_valid; // @[Cache.scala 493:19]
  wire [31:0] arb_io_in_1_bits_addr; // @[Cache.scala 493:19]
  wire [2:0] arb_io_in_1_bits_size; // @[Cache.scala 493:19]
  wire [3:0] arb_io_in_1_bits_cmd; // @[Cache.scala 493:19]
  wire [7:0] arb_io_in_1_bits_wmask; // @[Cache.scala 493:19]
  wire [63:0] arb_io_in_1_bits_wdata; // @[Cache.scala 493:19]
  wire [15:0] arb_io_in_1_bits_user; // @[Cache.scala 493:19]
  wire  arb_io_out_ready; // @[Cache.scala 493:19]
  wire  arb_io_out_valid; // @[Cache.scala 493:19]
  wire [31:0] arb_io_out_bits_addr; // @[Cache.scala 493:19]
  wire [2:0] arb_io_out_bits_size; // @[Cache.scala 493:19]
  wire [3:0] arb_io_out_bits_cmd; // @[Cache.scala 493:19]
  wire [7:0] arb_io_out_bits_wmask; // @[Cache.scala 493:19]
  wire [63:0] arb_io_out_bits_wdata; // @[Cache.scala 493:19]
  wire [15:0] arb_io_out_bits_user; // @[Cache.scala 493:19]
  wire  _T = s2_io_out_ready & s2_io_out_valid; // @[Decoupled.scala 51:35]
  reg  valid; // @[Pipeline.scala 24:24]
  wire  _GEN_0 = _T ? 1'h0 : valid; // @[Pipeline.scala 24:24 25:{25,33}]
  wire  _T_2 = s1_io_out_valid & s2_io_in_ready; // @[Pipeline.scala 26:22]
  wire  _GEN_1 = s1_io_out_valid & s2_io_in_ready | _GEN_0; // @[Pipeline.scala 26:{38,46}]
  reg [31:0] s2_io_in_bits_r_req_addr; // @[Reg.scala 19:16]
  reg [2:0] s2_io_in_bits_r_req_size; // @[Reg.scala 19:16]
  reg [3:0] s2_io_in_bits_r_req_cmd; // @[Reg.scala 19:16]
  reg [7:0] s2_io_in_bits_r_req_wmask; // @[Reg.scala 19:16]
  reg [63:0] s2_io_in_bits_r_req_wdata; // @[Reg.scala 19:16]
  reg [15:0] s2_io_in_bits_r_req_user; // @[Reg.scala 19:16]
  reg  valid_1; // @[Pipeline.scala 24:24]
  wire  _GEN_9 = s3_io_isFinish ? 1'h0 : valid_1; // @[Pipeline.scala 24:24 25:{25,33}]
  wire  _T_4 = s2_io_out_valid & s3_io_in_ready; // @[Pipeline.scala 26:22]
  wire  _GEN_10 = s2_io_out_valid & s3_io_in_ready | _GEN_9; // @[Pipeline.scala 26:{38,46}]
  reg [31:0] s3_io_in_bits_r_req_addr; // @[Reg.scala 19:16]
  reg [2:0] s3_io_in_bits_r_req_size; // @[Reg.scala 19:16]
  reg [3:0] s3_io_in_bits_r_req_cmd; // @[Reg.scala 19:16]
  reg [7:0] s3_io_in_bits_r_req_wmask; // @[Reg.scala 19:16]
  reg [63:0] s3_io_in_bits_r_req_wdata; // @[Reg.scala 19:16]
  reg [15:0] s3_io_in_bits_r_req_user; // @[Reg.scala 19:16]
  reg [18:0] s3_io_in_bits_r_metas_0_tag; // @[Reg.scala 19:16]
  reg  s3_io_in_bits_r_metas_0_dirty; // @[Reg.scala 19:16]
  reg [18:0] s3_io_in_bits_r_metas_1_tag; // @[Reg.scala 19:16]
  reg  s3_io_in_bits_r_metas_1_dirty; // @[Reg.scala 19:16]
  reg [18:0] s3_io_in_bits_r_metas_2_tag; // @[Reg.scala 19:16]
  reg  s3_io_in_bits_r_metas_2_dirty; // @[Reg.scala 19:16]
  reg [18:0] s3_io_in_bits_r_metas_3_tag; // @[Reg.scala 19:16]
  reg  s3_io_in_bits_r_metas_3_dirty; // @[Reg.scala 19:16]
  reg [63:0] s3_io_in_bits_r_datas_0_data; // @[Reg.scala 19:16]
  reg [63:0] s3_io_in_bits_r_datas_1_data; // @[Reg.scala 19:16]
  reg [63:0] s3_io_in_bits_r_datas_2_data; // @[Reg.scala 19:16]
  reg [63:0] s3_io_in_bits_r_datas_3_data; // @[Reg.scala 19:16]
  reg  s3_io_in_bits_r_hit; // @[Reg.scala 19:16]
  reg [3:0] s3_io_in_bits_r_waymask; // @[Reg.scala 19:16]
  reg  s3_io_in_bits_r_mmio; // @[Reg.scala 19:16]
  reg  s3_io_in_bits_r_isForwardData; // @[Reg.scala 19:16]
  reg [63:0] s3_io_in_bits_r_forwardData_data_data; // @[Reg.scala 19:16]
  reg [3:0] s3_io_in_bits_r_forwardData_waymask; // @[Reg.scala 19:16]
  wire  _io_in_resp_valid_T = s3_io_out_bits_cmd == 4'h4; // @[SimpleBus.scala 95:24]
  CacheStage1 s1 ( // @[Cache.scala 480:18]
    .io_in_ready(s1_io_in_ready),
    .io_in_valid(s1_io_in_valid),
    .io_in_bits_addr(s1_io_in_bits_addr),
    .io_in_bits_size(s1_io_in_bits_size),
    .io_in_bits_cmd(s1_io_in_bits_cmd),
    .io_in_bits_wmask(s1_io_in_bits_wmask),
    .io_in_bits_wdata(s1_io_in_bits_wdata),
    .io_in_bits_user(s1_io_in_bits_user),
    .io_out_ready(s1_io_out_ready),
    .io_out_valid(s1_io_out_valid),
    .io_out_bits_req_addr(s1_io_out_bits_req_addr),
    .io_out_bits_req_size(s1_io_out_bits_req_size),
    .io_out_bits_req_cmd(s1_io_out_bits_req_cmd),
    .io_out_bits_req_wmask(s1_io_out_bits_req_wmask),
    .io_out_bits_req_wdata(s1_io_out_bits_req_wdata),
    .io_out_bits_req_user(s1_io_out_bits_req_user),
    .io_metaReadBus_req_ready(s1_io_metaReadBus_req_ready),
    .io_metaReadBus_req_valid(s1_io_metaReadBus_req_valid),
    .io_metaReadBus_req_bits_setIdx(s1_io_metaReadBus_req_bits_setIdx),
    .io_metaReadBus_resp_data_0_tag(s1_io_metaReadBus_resp_data_0_tag),
    .io_metaReadBus_resp_data_0_valid(s1_io_metaReadBus_resp_data_0_valid),
    .io_metaReadBus_resp_data_0_dirty(s1_io_metaReadBus_resp_data_0_dirty),
    .io_metaReadBus_resp_data_1_tag(s1_io_metaReadBus_resp_data_1_tag),
    .io_metaReadBus_resp_data_1_valid(s1_io_metaReadBus_resp_data_1_valid),
    .io_metaReadBus_resp_data_1_dirty(s1_io_metaReadBus_resp_data_1_dirty),
    .io_metaReadBus_resp_data_2_tag(s1_io_metaReadBus_resp_data_2_tag),
    .io_metaReadBus_resp_data_2_valid(s1_io_metaReadBus_resp_data_2_valid),
    .io_metaReadBus_resp_data_2_dirty(s1_io_metaReadBus_resp_data_2_dirty),
    .io_metaReadBus_resp_data_3_tag(s1_io_metaReadBus_resp_data_3_tag),
    .io_metaReadBus_resp_data_3_valid(s1_io_metaReadBus_resp_data_3_valid),
    .io_metaReadBus_resp_data_3_dirty(s1_io_metaReadBus_resp_data_3_dirty),
    .io_dataReadBus_req_ready(s1_io_dataReadBus_req_ready),
    .io_dataReadBus_req_valid(s1_io_dataReadBus_req_valid),
    .io_dataReadBus_req_bits_setIdx(s1_io_dataReadBus_req_bits_setIdx),
    .io_dataReadBus_resp_data_0_data(s1_io_dataReadBus_resp_data_0_data),
    .io_dataReadBus_resp_data_1_data(s1_io_dataReadBus_resp_data_1_data),
    .io_dataReadBus_resp_data_2_data(s1_io_dataReadBus_resp_data_2_data),
    .io_dataReadBus_resp_data_3_data(s1_io_dataReadBus_resp_data_3_data)
  );
  CacheStage2 s2 ( // @[Cache.scala 481:18]
    .clock(s2_clock),
    .reset(s2_reset),
    .io_in_ready(s2_io_in_ready),
    .io_in_valid(s2_io_in_valid),
    .io_in_bits_req_addr(s2_io_in_bits_req_addr),
    .io_in_bits_req_size(s2_io_in_bits_req_size),
    .io_in_bits_req_cmd(s2_io_in_bits_req_cmd),
    .io_in_bits_req_wmask(s2_io_in_bits_req_wmask),
    .io_in_bits_req_wdata(s2_io_in_bits_req_wdata),
    .io_in_bits_req_user(s2_io_in_bits_req_user),
    .io_out_ready(s2_io_out_ready),
    .io_out_valid(s2_io_out_valid),
    .io_out_bits_req_addr(s2_io_out_bits_req_addr),
    .io_out_bits_req_size(s2_io_out_bits_req_size),
    .io_out_bits_req_cmd(s2_io_out_bits_req_cmd),
    .io_out_bits_req_wmask(s2_io_out_bits_req_wmask),
    .io_out_bits_req_wdata(s2_io_out_bits_req_wdata),
    .io_out_bits_req_user(s2_io_out_bits_req_user),
    .io_out_bits_metas_0_tag(s2_io_out_bits_metas_0_tag),
    .io_out_bits_metas_0_dirty(s2_io_out_bits_metas_0_dirty),
    .io_out_bits_metas_1_tag(s2_io_out_bits_metas_1_tag),
    .io_out_bits_metas_1_dirty(s2_io_out_bits_metas_1_dirty),
    .io_out_bits_metas_2_tag(s2_io_out_bits_metas_2_tag),
    .io_out_bits_metas_2_dirty(s2_io_out_bits_metas_2_dirty),
    .io_out_bits_metas_3_tag(s2_io_out_bits_metas_3_tag),
    .io_out_bits_metas_3_dirty(s2_io_out_bits_metas_3_dirty),
    .io_out_bits_datas_0_data(s2_io_out_bits_datas_0_data),
    .io_out_bits_datas_1_data(s2_io_out_bits_datas_1_data),
    .io_out_bits_datas_2_data(s2_io_out_bits_datas_2_data),
    .io_out_bits_datas_3_data(s2_io_out_bits_datas_3_data),
    .io_out_bits_hit(s2_io_out_bits_hit),
    .io_out_bits_waymask(s2_io_out_bits_waymask),
    .io_out_bits_mmio(s2_io_out_bits_mmio),
    .io_out_bits_isForwardData(s2_io_out_bits_isForwardData),
    .io_out_bits_forwardData_data_data(s2_io_out_bits_forwardData_data_data),
    .io_out_bits_forwardData_waymask(s2_io_out_bits_forwardData_waymask),
    .io_metaReadResp_0_tag(s2_io_metaReadResp_0_tag),
    .io_metaReadResp_0_valid(s2_io_metaReadResp_0_valid),
    .io_metaReadResp_0_dirty(s2_io_metaReadResp_0_dirty),
    .io_metaReadResp_1_tag(s2_io_metaReadResp_1_tag),
    .io_metaReadResp_1_valid(s2_io_metaReadResp_1_valid),
    .io_metaReadResp_1_dirty(s2_io_metaReadResp_1_dirty),
    .io_metaReadResp_2_tag(s2_io_metaReadResp_2_tag),
    .io_metaReadResp_2_valid(s2_io_metaReadResp_2_valid),
    .io_metaReadResp_2_dirty(s2_io_metaReadResp_2_dirty),
    .io_metaReadResp_3_tag(s2_io_metaReadResp_3_tag),
    .io_metaReadResp_3_valid(s2_io_metaReadResp_3_valid),
    .io_metaReadResp_3_dirty(s2_io_metaReadResp_3_dirty),
    .io_dataReadResp_0_data(s2_io_dataReadResp_0_data),
    .io_dataReadResp_1_data(s2_io_dataReadResp_1_data),
    .io_dataReadResp_2_data(s2_io_dataReadResp_2_data),
    .io_dataReadResp_3_data(s2_io_dataReadResp_3_data),
    .io_metaWriteBus_req_valid(s2_io_metaWriteBus_req_valid),
    .io_metaWriteBus_req_bits_setIdx(s2_io_metaWriteBus_req_bits_setIdx),
    .io_metaWriteBus_req_bits_data_tag(s2_io_metaWriteBus_req_bits_data_tag),
    .io_metaWriteBus_req_bits_data_dirty(s2_io_metaWriteBus_req_bits_data_dirty),
    .io_metaWriteBus_req_bits_waymask(s2_io_metaWriteBus_req_bits_waymask),
    .io_dataWriteBus_req_valid(s2_io_dataWriteBus_req_valid),
    .io_dataWriteBus_req_bits_setIdx(s2_io_dataWriteBus_req_bits_setIdx),
    .io_dataWriteBus_req_bits_data_data(s2_io_dataWriteBus_req_bits_data_data),
    .io_dataWriteBus_req_bits_waymask(s2_io_dataWriteBus_req_bits_waymask)
  );
  CacheStage3 s3 ( // @[Cache.scala 482:18]
    .clock(s3_clock),
    .reset(s3_reset),
    .io_in_ready(s3_io_in_ready),
    .io_in_valid(s3_io_in_valid),
    .io_in_bits_req_addr(s3_io_in_bits_req_addr),
    .io_in_bits_req_size(s3_io_in_bits_req_size),
    .io_in_bits_req_cmd(s3_io_in_bits_req_cmd),
    .io_in_bits_req_wmask(s3_io_in_bits_req_wmask),
    .io_in_bits_req_wdata(s3_io_in_bits_req_wdata),
    .io_in_bits_req_user(s3_io_in_bits_req_user),
    .io_in_bits_metas_0_tag(s3_io_in_bits_metas_0_tag),
    .io_in_bits_metas_0_dirty(s3_io_in_bits_metas_0_dirty),
    .io_in_bits_metas_1_tag(s3_io_in_bits_metas_1_tag),
    .io_in_bits_metas_1_dirty(s3_io_in_bits_metas_1_dirty),
    .io_in_bits_metas_2_tag(s3_io_in_bits_metas_2_tag),
    .io_in_bits_metas_2_dirty(s3_io_in_bits_metas_2_dirty),
    .io_in_bits_metas_3_tag(s3_io_in_bits_metas_3_tag),
    .io_in_bits_metas_3_dirty(s3_io_in_bits_metas_3_dirty),
    .io_in_bits_datas_0_data(s3_io_in_bits_datas_0_data),
    .io_in_bits_datas_1_data(s3_io_in_bits_datas_1_data),
    .io_in_bits_datas_2_data(s3_io_in_bits_datas_2_data),
    .io_in_bits_datas_3_data(s3_io_in_bits_datas_3_data),
    .io_in_bits_hit(s3_io_in_bits_hit),
    .io_in_bits_waymask(s3_io_in_bits_waymask),
    .io_in_bits_mmio(s3_io_in_bits_mmio),
    .io_in_bits_isForwardData(s3_io_in_bits_isForwardData),
    .io_in_bits_forwardData_data_data(s3_io_in_bits_forwardData_data_data),
    .io_in_bits_forwardData_waymask(s3_io_in_bits_forwardData_waymask),
    .io_out_ready(s3_io_out_ready),
    .io_out_valid(s3_io_out_valid),
    .io_out_bits_cmd(s3_io_out_bits_cmd),
    .io_out_bits_rdata(s3_io_out_bits_rdata),
    .io_out_bits_user(s3_io_out_bits_user),
    .io_isFinish(s3_io_isFinish),
    .io_flush(s3_io_flush),
    .io_dataReadBus_req_ready(s3_io_dataReadBus_req_ready),
    .io_dataReadBus_req_valid(s3_io_dataReadBus_req_valid),
    .io_dataReadBus_req_bits_setIdx(s3_io_dataReadBus_req_bits_setIdx),
    .io_dataReadBus_resp_data_0_data(s3_io_dataReadBus_resp_data_0_data),
    .io_dataReadBus_resp_data_1_data(s3_io_dataReadBus_resp_data_1_data),
    .io_dataReadBus_resp_data_2_data(s3_io_dataReadBus_resp_data_2_data),
    .io_dataReadBus_resp_data_3_data(s3_io_dataReadBus_resp_data_3_data),
    .io_dataWriteBus_req_valid(s3_io_dataWriteBus_req_valid),
    .io_dataWriteBus_req_bits_setIdx(s3_io_dataWriteBus_req_bits_setIdx),
    .io_dataWriteBus_req_bits_data_data(s3_io_dataWriteBus_req_bits_data_data),
    .io_dataWriteBus_req_bits_waymask(s3_io_dataWriteBus_req_bits_waymask),
    .io_metaWriteBus_req_valid(s3_io_metaWriteBus_req_valid),
    .io_metaWriteBus_req_bits_setIdx(s3_io_metaWriteBus_req_bits_setIdx),
    .io_metaWriteBus_req_bits_data_tag(s3_io_metaWriteBus_req_bits_data_tag),
    .io_metaWriteBus_req_bits_data_dirty(s3_io_metaWriteBus_req_bits_data_dirty),
    .io_metaWriteBus_req_bits_waymask(s3_io_metaWriteBus_req_bits_waymask),
    .io_mem_req_ready(s3_io_mem_req_ready),
    .io_mem_req_valid(s3_io_mem_req_valid),
    .io_mem_req_bits_addr(s3_io_mem_req_bits_addr),
    .io_mem_req_bits_cmd(s3_io_mem_req_bits_cmd),
    .io_mem_req_bits_wdata(s3_io_mem_req_bits_wdata),
    .io_mem_resp_ready(s3_io_mem_resp_ready),
    .io_mem_resp_valid(s3_io_mem_resp_valid),
    .io_mem_resp_bits_cmd(s3_io_mem_resp_bits_cmd),
    .io_mem_resp_bits_rdata(s3_io_mem_resp_bits_rdata),
    .io_mmio_req_ready(s3_io_mmio_req_ready),
    .io_mmio_req_valid(s3_io_mmio_req_valid),
    .io_mmio_req_bits_addr(s3_io_mmio_req_bits_addr),
    .io_mmio_req_bits_size(s3_io_mmio_req_bits_size),
    .io_mmio_req_bits_cmd(s3_io_mmio_req_bits_cmd),
    .io_mmio_req_bits_wmask(s3_io_mmio_req_bits_wmask),
    .io_mmio_req_bits_wdata(s3_io_mmio_req_bits_wdata),
    .io_mmio_resp_ready(s3_io_mmio_resp_ready),
    .io_mmio_resp_valid(s3_io_mmio_resp_valid),
    .io_mmio_resp_bits_rdata(s3_io_mmio_resp_bits_rdata),
    .io_cohResp_ready(s3_io_cohResp_ready),
    .io_cohResp_valid(s3_io_cohResp_valid),
    .io_cohResp_bits_cmd(s3_io_cohResp_bits_cmd),
    .io_cohResp_bits_rdata(s3_io_cohResp_bits_rdata),
    .io_dataReadRespToL1(s3_io_dataReadRespToL1)
  );
  SRAMTemplateWithArbiter metaArray ( // @[Cache.scala 483:25]
    .clock(metaArray_clock),
    .reset(metaArray_reset),
    .io_r_0_req_ready(metaArray_io_r_0_req_ready),
    .io_r_0_req_valid(metaArray_io_r_0_req_valid),
    .io_r_0_req_bits_setIdx(metaArray_io_r_0_req_bits_setIdx),
    .io_r_0_resp_data_0_tag(metaArray_io_r_0_resp_data_0_tag),
    .io_r_0_resp_data_0_valid(metaArray_io_r_0_resp_data_0_valid),
    .io_r_0_resp_data_0_dirty(metaArray_io_r_0_resp_data_0_dirty),
    .io_r_0_resp_data_1_tag(metaArray_io_r_0_resp_data_1_tag),
    .io_r_0_resp_data_1_valid(metaArray_io_r_0_resp_data_1_valid),
    .io_r_0_resp_data_1_dirty(metaArray_io_r_0_resp_data_1_dirty),
    .io_r_0_resp_data_2_tag(metaArray_io_r_0_resp_data_2_tag),
    .io_r_0_resp_data_2_valid(metaArray_io_r_0_resp_data_2_valid),
    .io_r_0_resp_data_2_dirty(metaArray_io_r_0_resp_data_2_dirty),
    .io_r_0_resp_data_3_tag(metaArray_io_r_0_resp_data_3_tag),
    .io_r_0_resp_data_3_valid(metaArray_io_r_0_resp_data_3_valid),
    .io_r_0_resp_data_3_dirty(metaArray_io_r_0_resp_data_3_dirty),
    .io_w_req_valid(metaArray_io_w_req_valid),
    .io_w_req_bits_setIdx(metaArray_io_w_req_bits_setIdx),
    .io_w_req_bits_data_tag(metaArray_io_w_req_bits_data_tag),
    .io_w_req_bits_data_dirty(metaArray_io_w_req_bits_data_dirty),
    .io_w_req_bits_waymask(metaArray_io_w_req_bits_waymask)
  );
  SRAMTemplateWithArbiter_1 dataArray ( // @[Cache.scala 484:25]
    .clock(dataArray_clock),
    .reset(dataArray_reset),
    .io_r_0_req_ready(dataArray_io_r_0_req_ready),
    .io_r_0_req_valid(dataArray_io_r_0_req_valid),
    .io_r_0_req_bits_setIdx(dataArray_io_r_0_req_bits_setIdx),
    .io_r_0_resp_data_0_data(dataArray_io_r_0_resp_data_0_data),
    .io_r_0_resp_data_1_data(dataArray_io_r_0_resp_data_1_data),
    .io_r_0_resp_data_2_data(dataArray_io_r_0_resp_data_2_data),
    .io_r_0_resp_data_3_data(dataArray_io_r_0_resp_data_3_data),
    .io_r_1_req_ready(dataArray_io_r_1_req_ready),
    .io_r_1_req_valid(dataArray_io_r_1_req_valid),
    .io_r_1_req_bits_setIdx(dataArray_io_r_1_req_bits_setIdx),
    .io_r_1_resp_data_0_data(dataArray_io_r_1_resp_data_0_data),
    .io_r_1_resp_data_1_data(dataArray_io_r_1_resp_data_1_data),
    .io_r_1_resp_data_2_data(dataArray_io_r_1_resp_data_2_data),
    .io_r_1_resp_data_3_data(dataArray_io_r_1_resp_data_3_data),
    .io_w_req_valid(dataArray_io_w_req_valid),
    .io_w_req_bits_setIdx(dataArray_io_w_req_bits_setIdx),
    .io_w_req_bits_data_data(dataArray_io_w_req_bits_data_data),
    .io_w_req_bits_waymask(dataArray_io_w_req_bits_waymask)
  );
  Arbiter_4 arb ( // @[Cache.scala 493:19]
    .io_in_0_ready(arb_io_in_0_ready),
    .io_in_0_valid(arb_io_in_0_valid),
    .io_in_0_bits_addr(arb_io_in_0_bits_addr),
    .io_in_0_bits_size(arb_io_in_0_bits_size),
    .io_in_0_bits_cmd(arb_io_in_0_bits_cmd),
    .io_in_0_bits_wmask(arb_io_in_0_bits_wmask),
    .io_in_0_bits_wdata(arb_io_in_0_bits_wdata),
    .io_in_1_ready(arb_io_in_1_ready),
    .io_in_1_valid(arb_io_in_1_valid),
    .io_in_1_bits_addr(arb_io_in_1_bits_addr),
    .io_in_1_bits_size(arb_io_in_1_bits_size),
    .io_in_1_bits_cmd(arb_io_in_1_bits_cmd),
    .io_in_1_bits_wmask(arb_io_in_1_bits_wmask),
    .io_in_1_bits_wdata(arb_io_in_1_bits_wdata),
    .io_in_1_bits_user(arb_io_in_1_bits_user),
    .io_out_ready(arb_io_out_ready),
    .io_out_valid(arb_io_out_valid),
    .io_out_bits_addr(arb_io_out_bits_addr),
    .io_out_bits_size(arb_io_out_bits_size),
    .io_out_bits_cmd(arb_io_out_bits_cmd),
    .io_out_bits_wmask(arb_io_out_bits_wmask),
    .io_out_bits_wdata(arb_io_out_bits_wdata),
    .io_out_bits_user(arb_io_out_bits_user)
  );
  assign io_in_req_ready = arb_io_in_1_ready; // @[Cache.scala 494:28]
  assign io_in_resp_valid = s3_io_out_valid & _io_in_resp_valid_T ? 1'h0 : s3_io_out_valid | s3_io_dataReadRespToL1; // @[Cache.scala 510:26]
  assign io_in_resp_bits_cmd = s3_io_out_bits_cmd; // @[Cache.scala 504:14]
  assign io_in_resp_bits_rdata = s3_io_out_bits_rdata; // @[Cache.scala 504:14]
  assign io_in_resp_bits_user = s3_io_out_bits_user; // @[Cache.scala 504:14]
  assign io_out_mem_req_valid = s3_io_mem_req_valid; // @[Cache.scala 506:14]
  assign io_out_mem_req_bits_addr = s3_io_mem_req_bits_addr; // @[Cache.scala 506:14]
  assign io_out_mem_req_bits_size = 3'h3; // @[Cache.scala 506:14]
  assign io_out_mem_req_bits_cmd = s3_io_mem_req_bits_cmd; // @[Cache.scala 506:14]
  assign io_out_mem_req_bits_wmask = 8'hff; // @[Cache.scala 506:14]
  assign io_out_mem_req_bits_wdata = s3_io_mem_req_bits_wdata; // @[Cache.scala 506:14]
  assign io_out_mem_resp_ready = 1'h1; // @[Cache.scala 506:14]
  assign io_out_coh_req_ready = arb_io_in_0_ready; // @[Cache.scala 519:26]
  assign io_out_coh_resp_valid = s3_io_cohResp_valid; // @[Cache.scala 520:21]
  assign io_out_coh_resp_bits_cmd = s3_io_cohResp_bits_cmd; // @[Cache.scala 520:21]
  assign io_out_coh_resp_bits_rdata = s3_io_cohResp_bits_rdata; // @[Cache.scala 520:21]
  assign io_mmio_req_valid = s3_io_mmio_req_valid; // @[Cache.scala 507:11]
  assign io_mmio_req_bits_addr = s3_io_mmio_req_bits_addr; // @[Cache.scala 507:11]
  assign io_mmio_req_bits_size = s3_io_mmio_req_bits_size; // @[Cache.scala 507:11]
  assign io_mmio_req_bits_cmd = s3_io_mmio_req_bits_cmd; // @[Cache.scala 507:11]
  assign io_mmio_req_bits_wmask = s3_io_mmio_req_bits_wmask; // @[Cache.scala 507:11]
  assign io_mmio_req_bits_wdata = s3_io_mmio_req_bits_wdata; // @[Cache.scala 507:11]
  assign io_mmio_resp_ready = 1'h1; // @[Cache.scala 507:11]
  assign io_empty = ~s2_io_in_valid & ~s3_io_in_valid; // @[Cache.scala 508:31]
  assign s1_io_in_valid = arb_io_out_valid; // @[Cache.scala 496:12]
  assign s1_io_in_bits_addr = arb_io_out_bits_addr; // @[Cache.scala 496:12]
  assign s1_io_in_bits_size = arb_io_out_bits_size; // @[Cache.scala 496:12]
  assign s1_io_in_bits_cmd = arb_io_out_bits_cmd; // @[Cache.scala 496:12]
  assign s1_io_in_bits_wmask = arb_io_out_bits_wmask; // @[Cache.scala 496:12]
  assign s1_io_in_bits_wdata = arb_io_out_bits_wdata; // @[Cache.scala 496:12]
  assign s1_io_in_bits_user = arb_io_out_bits_user; // @[Cache.scala 496:12]
  assign s1_io_out_ready = s2_io_in_ready; // @[Pipeline.scala 29:16]
  assign s1_io_metaReadBus_req_ready = metaArray_io_r_0_req_ready; // @[Cache.scala 528:21]
  assign s1_io_metaReadBus_resp_data_0_tag = metaArray_io_r_0_resp_data_0_tag; // @[Cache.scala 528:21]
  assign s1_io_metaReadBus_resp_data_0_valid = metaArray_io_r_0_resp_data_0_valid; // @[Cache.scala 528:21]
  assign s1_io_metaReadBus_resp_data_0_dirty = metaArray_io_r_0_resp_data_0_dirty; // @[Cache.scala 528:21]
  assign s1_io_metaReadBus_resp_data_1_tag = metaArray_io_r_0_resp_data_1_tag; // @[Cache.scala 528:21]
  assign s1_io_metaReadBus_resp_data_1_valid = metaArray_io_r_0_resp_data_1_valid; // @[Cache.scala 528:21]
  assign s1_io_metaReadBus_resp_data_1_dirty = metaArray_io_r_0_resp_data_1_dirty; // @[Cache.scala 528:21]
  assign s1_io_metaReadBus_resp_data_2_tag = metaArray_io_r_0_resp_data_2_tag; // @[Cache.scala 528:21]
  assign s1_io_metaReadBus_resp_data_2_valid = metaArray_io_r_0_resp_data_2_valid; // @[Cache.scala 528:21]
  assign s1_io_metaReadBus_resp_data_2_dirty = metaArray_io_r_0_resp_data_2_dirty; // @[Cache.scala 528:21]
  assign s1_io_metaReadBus_resp_data_3_tag = metaArray_io_r_0_resp_data_3_tag; // @[Cache.scala 528:21]
  assign s1_io_metaReadBus_resp_data_3_valid = metaArray_io_r_0_resp_data_3_valid; // @[Cache.scala 528:21]
  assign s1_io_metaReadBus_resp_data_3_dirty = metaArray_io_r_0_resp_data_3_dirty; // @[Cache.scala 528:21]
  assign s1_io_dataReadBus_req_ready = dataArray_io_r_0_req_ready; // @[Cache.scala 529:21]
  assign s1_io_dataReadBus_resp_data_0_data = dataArray_io_r_0_resp_data_0_data; // @[Cache.scala 529:21]
  assign s1_io_dataReadBus_resp_data_1_data = dataArray_io_r_0_resp_data_1_data; // @[Cache.scala 529:21]
  assign s1_io_dataReadBus_resp_data_2_data = dataArray_io_r_0_resp_data_2_data; // @[Cache.scala 529:21]
  assign s1_io_dataReadBus_resp_data_3_data = dataArray_io_r_0_resp_data_3_data; // @[Cache.scala 529:21]
  assign s2_clock = clock;
  assign s2_reset = reset;
  assign s2_io_in_valid = valid; // @[Pipeline.scala 31:17]
  assign s2_io_in_bits_req_addr = s2_io_in_bits_r_req_addr; // @[Pipeline.scala 30:16]
  assign s2_io_in_bits_req_size = s2_io_in_bits_r_req_size; // @[Pipeline.scala 30:16]
  assign s2_io_in_bits_req_cmd = s2_io_in_bits_r_req_cmd; // @[Pipeline.scala 30:16]
  assign s2_io_in_bits_req_wmask = s2_io_in_bits_r_req_wmask; // @[Pipeline.scala 30:16]
  assign s2_io_in_bits_req_wdata = s2_io_in_bits_r_req_wdata; // @[Pipeline.scala 30:16]
  assign s2_io_in_bits_req_user = s2_io_in_bits_r_req_user; // @[Pipeline.scala 30:16]
  assign s2_io_out_ready = s3_io_in_ready; // @[Pipeline.scala 29:16]
  assign s2_io_metaReadResp_0_tag = s1_io_metaReadBus_resp_data_0_tag; // @[Cache.scala 535:22]
  assign s2_io_metaReadResp_0_valid = s1_io_metaReadBus_resp_data_0_valid; // @[Cache.scala 535:22]
  assign s2_io_metaReadResp_0_dirty = s1_io_metaReadBus_resp_data_0_dirty; // @[Cache.scala 535:22]
  assign s2_io_metaReadResp_1_tag = s1_io_metaReadBus_resp_data_1_tag; // @[Cache.scala 535:22]
  assign s2_io_metaReadResp_1_valid = s1_io_metaReadBus_resp_data_1_valid; // @[Cache.scala 535:22]
  assign s2_io_metaReadResp_1_dirty = s1_io_metaReadBus_resp_data_1_dirty; // @[Cache.scala 535:22]
  assign s2_io_metaReadResp_2_tag = s1_io_metaReadBus_resp_data_2_tag; // @[Cache.scala 535:22]
  assign s2_io_metaReadResp_2_valid = s1_io_metaReadBus_resp_data_2_valid; // @[Cache.scala 535:22]
  assign s2_io_metaReadResp_2_dirty = s1_io_metaReadBus_resp_data_2_dirty; // @[Cache.scala 535:22]
  assign s2_io_metaReadResp_3_tag = s1_io_metaReadBus_resp_data_3_tag; // @[Cache.scala 535:22]
  assign s2_io_metaReadResp_3_valid = s1_io_metaReadBus_resp_data_3_valid; // @[Cache.scala 535:22]
  assign s2_io_metaReadResp_3_dirty = s1_io_metaReadBus_resp_data_3_dirty; // @[Cache.scala 535:22]
  assign s2_io_dataReadResp_0_data = s1_io_dataReadBus_resp_data_0_data; // @[Cache.scala 536:22]
  assign s2_io_dataReadResp_1_data = s1_io_dataReadBus_resp_data_1_data; // @[Cache.scala 536:22]
  assign s2_io_dataReadResp_2_data = s1_io_dataReadBus_resp_data_2_data; // @[Cache.scala 536:22]
  assign s2_io_dataReadResp_3_data = s1_io_dataReadBus_resp_data_3_data; // @[Cache.scala 536:22]
  assign s2_io_metaWriteBus_req_valid = s3_io_metaWriteBus_req_valid; // @[Cache.scala 538:22]
  assign s2_io_metaWriteBus_req_bits_setIdx = s3_io_metaWriteBus_req_bits_setIdx; // @[Cache.scala 538:22]
  assign s2_io_metaWriteBus_req_bits_data_tag = s3_io_metaWriteBus_req_bits_data_tag; // @[Cache.scala 538:22]
  assign s2_io_metaWriteBus_req_bits_data_dirty = s3_io_metaWriteBus_req_bits_data_dirty; // @[Cache.scala 538:22]
  assign s2_io_metaWriteBus_req_bits_waymask = s3_io_metaWriteBus_req_bits_waymask; // @[Cache.scala 538:22]
  assign s2_io_dataWriteBus_req_valid = s3_io_dataWriteBus_req_valid; // @[Cache.scala 537:22]
  assign s2_io_dataWriteBus_req_bits_setIdx = s3_io_dataWriteBus_req_bits_setIdx; // @[Cache.scala 537:22]
  assign s2_io_dataWriteBus_req_bits_data_data = s3_io_dataWriteBus_req_bits_data_data; // @[Cache.scala 537:22]
  assign s2_io_dataWriteBus_req_bits_waymask = s3_io_dataWriteBus_req_bits_waymask; // @[Cache.scala 537:22]
  assign s3_clock = clock;
  assign s3_reset = reset;
  assign s3_io_in_valid = valid_1; // @[Pipeline.scala 31:17]
  assign s3_io_in_bits_req_addr = s3_io_in_bits_r_req_addr; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_req_size = s3_io_in_bits_r_req_size; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_req_cmd = s3_io_in_bits_r_req_cmd; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_req_wmask = s3_io_in_bits_r_req_wmask; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_req_wdata = s3_io_in_bits_r_req_wdata; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_req_user = s3_io_in_bits_r_req_user; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_metas_0_tag = s3_io_in_bits_r_metas_0_tag; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_metas_0_dirty = s3_io_in_bits_r_metas_0_dirty; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_metas_1_tag = s3_io_in_bits_r_metas_1_tag; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_metas_1_dirty = s3_io_in_bits_r_metas_1_dirty; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_metas_2_tag = s3_io_in_bits_r_metas_2_tag; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_metas_2_dirty = s3_io_in_bits_r_metas_2_dirty; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_metas_3_tag = s3_io_in_bits_r_metas_3_tag; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_metas_3_dirty = s3_io_in_bits_r_metas_3_dirty; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_datas_0_data = s3_io_in_bits_r_datas_0_data; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_datas_1_data = s3_io_in_bits_r_datas_1_data; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_datas_2_data = s3_io_in_bits_r_datas_2_data; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_datas_3_data = s3_io_in_bits_r_datas_3_data; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_hit = s3_io_in_bits_r_hit; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_waymask = s3_io_in_bits_r_waymask; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_mmio = s3_io_in_bits_r_mmio; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_isForwardData = s3_io_in_bits_r_isForwardData; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_forwardData_data_data = s3_io_in_bits_r_forwardData_data_data; // @[Pipeline.scala 30:16]
  assign s3_io_in_bits_forwardData_waymask = s3_io_in_bits_r_forwardData_waymask; // @[Pipeline.scala 30:16]
  assign s3_io_out_ready = io_in_resp_ready; // @[Cache.scala 504:14]
  assign s3_io_flush = io_flush[1]; // @[Cache.scala 505:26]
  assign s3_io_dataReadBus_req_ready = dataArray_io_r_1_req_ready; // @[Cache.scala 530:21]
  assign s3_io_dataReadBus_resp_data_0_data = dataArray_io_r_1_resp_data_0_data; // @[Cache.scala 530:21]
  assign s3_io_dataReadBus_resp_data_1_data = dataArray_io_r_1_resp_data_1_data; // @[Cache.scala 530:21]
  assign s3_io_dataReadBus_resp_data_2_data = dataArray_io_r_1_resp_data_2_data; // @[Cache.scala 530:21]
  assign s3_io_dataReadBus_resp_data_3_data = dataArray_io_r_1_resp_data_3_data; // @[Cache.scala 530:21]
  assign s3_io_mem_req_ready = io_out_mem_req_ready; // @[Cache.scala 506:14]
  assign s3_io_mem_resp_valid = io_out_mem_resp_valid; // @[Cache.scala 506:14]
  assign s3_io_mem_resp_bits_cmd = io_out_mem_resp_bits_cmd; // @[Cache.scala 506:14]
  assign s3_io_mem_resp_bits_rdata = io_out_mem_resp_bits_rdata; // @[Cache.scala 506:14]
  assign s3_io_mmio_req_ready = io_mmio_req_ready; // @[Cache.scala 507:11]
  assign s3_io_mmio_resp_valid = io_mmio_resp_valid; // @[Cache.scala 507:11]
  assign s3_io_mmio_resp_bits_rdata = io_mmio_resp_bits_rdata; // @[Cache.scala 507:11]
  assign s3_io_cohResp_ready = io_out_coh_resp_ready; // @[Cache.scala 520:21]
  assign metaArray_clock = clock;
  assign metaArray_reset = reset;
  assign metaArray_io_r_0_req_valid = s1_io_metaReadBus_req_valid; // @[Cache.scala 528:21]
  assign metaArray_io_r_0_req_bits_setIdx = s1_io_metaReadBus_req_bits_setIdx; // @[Cache.scala 528:21]
  assign metaArray_io_w_req_valid = s3_io_metaWriteBus_req_valid; // @[Cache.scala 532:18]
  assign metaArray_io_w_req_bits_setIdx = s3_io_metaWriteBus_req_bits_setIdx; // @[Cache.scala 532:18]
  assign metaArray_io_w_req_bits_data_tag = s3_io_metaWriteBus_req_bits_data_tag; // @[Cache.scala 532:18]
  assign metaArray_io_w_req_bits_data_dirty = s3_io_metaWriteBus_req_bits_data_dirty; // @[Cache.scala 532:18]
  assign metaArray_io_w_req_bits_waymask = s3_io_metaWriteBus_req_bits_waymask; // @[Cache.scala 532:18]
  assign dataArray_clock = clock;
  assign dataArray_reset = reset;
  assign dataArray_io_r_0_req_valid = s1_io_dataReadBus_req_valid; // @[Cache.scala 529:21]
  assign dataArray_io_r_0_req_bits_setIdx = s1_io_dataReadBus_req_bits_setIdx; // @[Cache.scala 529:21]
  assign dataArray_io_r_1_req_valid = s3_io_dataReadBus_req_valid; // @[Cache.scala 530:21]
  assign dataArray_io_r_1_req_bits_setIdx = s3_io_dataReadBus_req_bits_setIdx; // @[Cache.scala 530:21]
  assign dataArray_io_w_req_valid = s3_io_dataWriteBus_req_valid; // @[Cache.scala 533:18]
  assign dataArray_io_w_req_bits_setIdx = s3_io_dataWriteBus_req_bits_setIdx; // @[Cache.scala 533:18]
  assign dataArray_io_w_req_bits_data_data = s3_io_dataWriteBus_req_bits_data_data; // @[Cache.scala 533:18]
  assign dataArray_io_w_req_bits_waymask = s3_io_dataWriteBus_req_bits_waymask; // @[Cache.scala 533:18]
  assign arb_io_in_0_valid = io_out_coh_req_valid; // @[Cache.scala 518:24]
  assign arb_io_in_0_bits_addr = io_out_coh_req_bits_addr; // @[Cache.scala 515:19 SimpleBus.scala 64:15]
  assign arb_io_in_0_bits_size = io_out_coh_req_bits_size; // @[Cache.scala 515:19 SimpleBus.scala 66:15]
  assign arb_io_in_0_bits_cmd = io_out_coh_req_bits_cmd; // @[Cache.scala 515:19 SimpleBus.scala 65:14]
  assign arb_io_in_0_bits_wmask = io_out_coh_req_bits_wmask; // @[Cache.scala 515:19 SimpleBus.scala 68:16]
  assign arb_io_in_0_bits_wdata = io_out_coh_req_bits_wdata; // @[Cache.scala 515:19 SimpleBus.scala 67:16]
  assign arb_io_in_1_valid = io_in_req_valid; // @[Cache.scala 494:28]
  assign arb_io_in_1_bits_addr = io_in_req_bits_addr; // @[Cache.scala 494:28]
  assign arb_io_in_1_bits_size = io_in_req_bits_size; // @[Cache.scala 494:28]
  assign arb_io_in_1_bits_cmd = io_in_req_bits_cmd; // @[Cache.scala 494:28]
  assign arb_io_in_1_bits_wmask = io_in_req_bits_wmask; // @[Cache.scala 494:28]
  assign arb_io_in_1_bits_wdata = io_in_req_bits_wdata; // @[Cache.scala 494:28]
  assign arb_io_in_1_bits_user = io_in_req_bits_user; // @[Cache.scala 494:28]
  assign arb_io_out_ready = s1_io_in_ready; // @[Cache.scala 496:12]
  always @(posedge clock) begin
    if (reset) begin // @[Pipeline.scala 24:24]
      valid <= 1'h0; // @[Pipeline.scala 24:24]
    end else if (io_flush[0]) begin // @[Pipeline.scala 27:20]
      valid <= 1'h0; // @[Pipeline.scala 27:28]
    end else begin
      valid <= _GEN_1;
    end
    if (_T_2) begin // @[Reg.scala 20:18]
      s2_io_in_bits_r_req_addr <= s1_io_out_bits_req_addr; // @[Reg.scala 20:22]
    end
    if (_T_2) begin // @[Reg.scala 20:18]
      s2_io_in_bits_r_req_size <= s1_io_out_bits_req_size; // @[Reg.scala 20:22]
    end
    if (_T_2) begin // @[Reg.scala 20:18]
      s2_io_in_bits_r_req_cmd <= s1_io_out_bits_req_cmd; // @[Reg.scala 20:22]
    end
    if (_T_2) begin // @[Reg.scala 20:18]
      s2_io_in_bits_r_req_wmask <= s1_io_out_bits_req_wmask; // @[Reg.scala 20:22]
    end
    if (_T_2) begin // @[Reg.scala 20:18]
      s2_io_in_bits_r_req_wdata <= s1_io_out_bits_req_wdata; // @[Reg.scala 20:22]
    end
    if (_T_2) begin // @[Reg.scala 20:18]
      s2_io_in_bits_r_req_user <= s1_io_out_bits_req_user; // @[Reg.scala 20:22]
    end
    if (reset) begin // @[Pipeline.scala 24:24]
      valid_1 <= 1'h0; // @[Pipeline.scala 24:24]
    end else if (io_flush[1]) begin // @[Pipeline.scala 27:20]
      valid_1 <= 1'h0; // @[Pipeline.scala 27:28]
    end else begin
      valid_1 <= _GEN_10;
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_req_addr <= s2_io_out_bits_req_addr; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_req_size <= s2_io_out_bits_req_size; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_req_cmd <= s2_io_out_bits_req_cmd; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_req_wmask <= s2_io_out_bits_req_wmask; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_req_wdata <= s2_io_out_bits_req_wdata; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_req_user <= s2_io_out_bits_req_user; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_metas_0_tag <= s2_io_out_bits_metas_0_tag; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_metas_0_dirty <= s2_io_out_bits_metas_0_dirty; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_metas_1_tag <= s2_io_out_bits_metas_1_tag; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_metas_1_dirty <= s2_io_out_bits_metas_1_dirty; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_metas_2_tag <= s2_io_out_bits_metas_2_tag; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_metas_2_dirty <= s2_io_out_bits_metas_2_dirty; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_metas_3_tag <= s2_io_out_bits_metas_3_tag; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_metas_3_dirty <= s2_io_out_bits_metas_3_dirty; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_datas_0_data <= s2_io_out_bits_datas_0_data; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_datas_1_data <= s2_io_out_bits_datas_1_data; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_datas_2_data <= s2_io_out_bits_datas_2_data; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_datas_3_data <= s2_io_out_bits_datas_3_data; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_hit <= s2_io_out_bits_hit; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_waymask <= s2_io_out_bits_waymask; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_mmio <= s2_io_out_bits_mmio; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_isForwardData <= s2_io_out_bits_isForwardData; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_forwardData_data_data <= s2_io_out_bits_forwardData_data_data; // @[Reg.scala 20:22]
    end
    if (_T_4) begin // @[Reg.scala 20:18]
      s3_io_in_bits_r_forwardData_waymask <= s2_io_out_bits_forwardData_waymask; // @[Reg.scala 20:22]
    end
  end
// Register and memory initialization
`ifdef RANDOMIZE_GARBAGE_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_INVALID_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_REG_INIT
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_MEM_INIT
`define RANDOMIZE
`endif
`ifndef RANDOM
`define RANDOM $random
`endif
`ifdef RANDOMIZE_MEM_INIT
  integer initvar;
`endif
`ifndef SYNTHESIS
`ifdef FIRRTL_BEFORE_INITIAL
`FIRRTL_BEFORE_INITIAL
`endif
initial begin
  `ifdef RANDOMIZE
    `ifdef INIT_RANDOM
      `INIT_RANDOM
    `endif
    `ifndef VERILATOR
      `ifdef RANDOMIZE_DELAY
        #`RANDOMIZE_DELAY begin end
      `else
        #0.002 begin end
      `endif
    `endif
`ifdef RANDOMIZE_REG_INIT
  _RAND_0 = {1{`RANDOM}};
  valid = _RAND_0[0:0];
  _RAND_1 = {1{`RANDOM}};
  s2_io_in_bits_r_req_addr = _RAND_1[31:0];
  _RAND_2 = {1{`RANDOM}};
  s2_io_in_bits_r_req_size = _RAND_2[2:0];
  _RAND_3 = {1{`RANDOM}};
  s2_io_in_bits_r_req_cmd = _RAND_3[3:0];
  _RAND_4 = {1{`RANDOM}};
  s2_io_in_bits_r_req_wmask = _RAND_4[7:0];
  _RAND_5 = {2{`RANDOM}};
  s2_io_in_bits_r_req_wdata = _RAND_5[63:0];
  _RAND_6 = {1{`RANDOM}};
  s2_io_in_bits_r_req_user = _RAND_6[15:0];
  _RAND_7 = {1{`RANDOM}};
  valid_1 = _RAND_7[0:0];
  _RAND_8 = {1{`RANDOM}};
  s3_io_in_bits_r_req_addr = _RAND_8[31:0];
  _RAND_9 = {1{`RANDOM}};
  s3_io_in_bits_r_req_size = _RAND_9[2:0];
  _RAND_10 = {1{`RANDOM}};
  s3_io_in_bits_r_req_cmd = _RAND_10[3:0];
  _RAND_11 = {1{`RANDOM}};
  s3_io_in_bits_r_req_wmask = _RAND_11[7:0];
  _RAND_12 = {2{`RANDOM}};
  s3_io_in_bits_r_req_wdata = _RAND_12[63:0];
  _RAND_13 = {1{`RANDOM}};
  s3_io_in_bits_r_req_user = _RAND_13[15:0];
  _RAND_14 = {1{`RANDOM}};
  s3_io_in_bits_r_metas_0_tag = _RAND_14[18:0];
  _RAND_15 = {1{`RANDOM}};
  s3_io_in_bits_r_metas_0_dirty = _RAND_15[0:0];
  _RAND_16 = {1{`RANDOM}};
  s3_io_in_bits_r_metas_1_tag = _RAND_16[18:0];
  _RAND_17 = {1{`RANDOM}};
  s3_io_in_bits_r_metas_1_dirty = _RAND_17[0:0];
  _RAND_18 = {1{`RANDOM}};
  s3_io_in_bits_r_metas_2_tag = _RAND_18[18:0];
  _RAND_19 = {1{`RANDOM}};
  s3_io_in_bits_r_metas_2_dirty = _RAND_19[0:0];
  _RAND_20 = {1{`RANDOM}};
  s3_io_in_bits_r_metas_3_tag = _RAND_20[18:0];
  _RAND_21 = {1{`RANDOM}};
  s3_io_in_bits_r_metas_3_dirty = _RAND_21[0:0];
  _RAND_22 = {2{`RANDOM}};
  s3_io_in_bits_r_datas_0_data = _RAND_22[63:0];
  _RAND_23 = {2{`RANDOM}};
  s3_io_in_bits_r_datas_1_data = _RAND_23[63:0];
  _RAND_24 = {2{`RANDOM}};
  s3_io_in_bits_r_datas_2_data = _RAND_24[63:0];
  _RAND_25 = {2{`RANDOM}};
  s3_io_in_bits_r_datas_3_data = _RAND_25[63:0];
  _RAND_26 = {1{`RANDOM}};
  s3_io_in_bits_r_hit = _RAND_26[0:0];
  _RAND_27 = {1{`RANDOM}};
  s3_io_in_bits_r_waymask = _RAND_27[3:0];
  _RAND_28 = {1{`RANDOM}};
  s3_io_in_bits_r_mmio = _RAND_28[0:0];
  _RAND_29 = {1{`RANDOM}};
  s3_io_in_bits_r_isForwardData = _RAND_29[0:0];
  _RAND_30 = {2{`RANDOM}};
  s3_io_in_bits_r_forwardData_data_data = _RAND_30[63:0];
  _RAND_31 = {1{`RANDOM}};
  s3_io_in_bits_r_forwardData_waymask = _RAND_31[3:0];
`endif // RANDOMIZE_REG_INIT
  `endif // RANDOMIZE
end // initial
`ifdef FIRRTL_AFTER_INITIAL
`FIRRTL_AFTER_INITIAL
`endif
`endif // SYNTHESIS
endmodule
