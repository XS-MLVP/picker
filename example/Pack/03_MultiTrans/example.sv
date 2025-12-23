//==============================================================================
// File       : test_alu_uvm.sv
// Description: Custom UVM testbench for ALU multi-transaction test
//              - Receives alu_op from Python
//              - Computes ALU result
//              - Sends alu_result back to Python
//==============================================================================
import uvm_pkg::*;
import uvmc_pkg::*;

// Include common utility package before agents
`include "ALU/utils_pkg.sv"

// Include source files and generated agent files
// Source files are in pack_output/, agent file is in ALU_pkg/
`include "alu_op.sv"
`include "alu_result.sv"
`include "ALU/xagent.sv"

// ALU functional model
class alu_model extends uvm_component;
    `uvm_component_utils(alu_model)
    
    alu_result_xmonitor result_mon;
    
    function new(string name = "alu_model", uvm_component parent = null);
        super.new(name, parent);
    endfunction
    
    task compute_and_send(alu_op op_trans);
        alu_result res_trans;
        bit [15:0] result;
        bit [2:0] status;
        
        res_trans = alu_result::type_id::create("res_trans");
        status = 0;
        
        // Compute based on opcode
        case(op_trans.opcode)
            2'b00: begin  // ADD
                result = op_trans.operand_a + op_trans.operand_b;
                if (result > 16'hFFFF)
                    status[0] = 1;  // overflow
            end
            2'b01: begin  // SUB
                if (op_trans.operand_a >= op_trans.operand_b) begin
                    result = op_trans.operand_a - op_trans.operand_b;
                end else begin
                    result = op_trans.operand_b - op_trans.operand_a;
                    status[2] = 1;  // negative
                end
            end
            2'b10: begin  // MUL
                result = op_trans.operand_a * op_trans.operand_b;
                if (result > 16'hFFFF)
                    status[0] = 1;  // overflow
            end
            2'b11: begin  // DIV
                if (op_trans.operand_b == 0) begin
                    result = 0;
                    status[0] = 1;  // division by zero
                end else begin
                    result = op_trans.operand_a / op_trans.operand_b;
                end
            end
        endcase
        
        if (result == 0)
            status[1] = 1;  // zero flag
        
        // Set result transaction
        res_trans.result = result;
        res_trans.status = status;
        
        // Send result back to Python
        if (result_mon != null) begin
            result_mon.sequence_send(res_trans);
        end
        
        `uvm_info("alu_model", $sformatf("Computed: op=%0d, a=%0d, b=%0d => result=%0d, status=0x%0h",
                  op_trans.opcode, op_trans.operand_a, op_trans.operand_b, result, status), UVM_LOW)
    endtask
endclass


// Custom driver for alu_op - receives from Python and processes
class alu_op_driver extends alu_op_xdriver;
    `uvm_component_utils(alu_op_driver)
    
    alu_model model;
    
    function new(string name = "alu_op_driver", uvm_component parent = null);
        super.new(name, parent);
    endfunction
    
    virtual function void build_phase(uvm_phase phase);
        super.build_phase(phase);
    endfunction
    
    virtual task sequence_receive(alu_op tr);
        `uvm_info("alu_op_driver", $sformatf("Received operation from Python:\n%s", tr.sprint()), UVM_LOW)
        
        // Compute result and send back via result monitor
        if (model != null) begin
            model.compute_and_send(tr);
        end
    endtask
endclass


// Custom driver for alu_result - just receives from Python (not used in this flow)
class alu_result_driver extends alu_result_xdriver;
    `uvm_component_utils(alu_result_driver)
    
    function new(string name = "alu_result_driver", uvm_component parent = null);
        super.new(name, parent);
    endfunction
    
    virtual task sequence_receive(alu_result tr);
        `uvm_info("alu_result_driver", $sformatf("Received result from Python:\n%s", tr.sprint()), UVM_LOW)
    endtask
endclass


// Environment
class alu_env extends uvm_env;
    `uvm_component_utils(alu_env)
    
    alu_op_xagent           op_agent;
    alu_op_xagent_config    op_config;
    alu_op_driver           op_driver;
    
    alu_result_xagent         result_agent;
    alu_result_xagent_config  result_config;
    
    alu_model               model;
    
    function new(string name = "alu_env", uvm_component parent = null);
        super.new(name, parent);

        // Configure alu_op agent (receive from Python) - use factory pattern
        op_config = new("op_config");
        op_config.is_active = UVM_ACTIVE;  // Enable both monitor and driver
        uvm_config_db#(alu_op_xagent_config)::set(this, "op_agent", "alu_op_xagent_config", op_config);
        // Use factory override for custom driver
        set_type_override_by_type(alu_op_xdriver::get_type(), alu_op_driver::get_type());

        // Configure alu_result agent (send to Python) - use factory pattern
        result_config = new("result_config");
        result_config.is_active = UVM_ACTIVE;  // Enable both monitor and driver
        uvm_config_db#(alu_result_xagent_config)::set(this, "result_agent", "alu_result_xagent_config", result_config);
    endfunction
    
    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        op_agent = alu_op_xagent::type_id::create("op_agent", this);
        result_agent = alu_result_xagent::type_id::create("result_agent", this);
        model = alu_model::type_id::create("model", this);
    endfunction
    
    function void connect_phase(uvm_phase phase);
        super.connect_phase(phase);

        // Connect driver to model
        if (op_agent.alu_op_xdrv != null && $cast(op_driver, op_agent.alu_op_xdrv)) begin
            op_driver.model = model;
        end

        // Connect model to result monitor
        if (result_agent.alu_result_xmon != null) begin
            model.result_mon = result_agent.alu_result_xmon;
        end
    endfunction
    
    virtual task main_phase(uvm_phase phase);
        phase.raise_objection(this);
        // Wait for Python to complete testing
        #100000;
        phase.drop_objection(this);
    endtask
endclass


// Test
class alu_test extends uvm_test;
    `uvm_component_utils(alu_test)
    
    alu_env env;
    
    function new(string name = "alu_test", uvm_component parent = null);
        super.new(name, parent);
    endfunction
    
    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        env = alu_env::type_id::create("env", this);
    endfunction
endclass


// Top module
module sv_main;
    logic clk;
    logic rst_n;
    
    initial begin
        clk = 0;
        forever #2 clk = ~clk;
    end
    
    initial begin
        rst_n = 0;
        #10 rst_n = 1;
    end
    
    initial begin
        run_test("alu_test");
    end
endmodule
