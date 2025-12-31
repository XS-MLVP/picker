//==============================================================================
// File       : example-uvm.sv
// Description: UVM testbench for Adder DUT with Python integration
//              - UVM environment drives Adder DUT  
//              - Python can monitor/drive through Adder transaction
//==============================================================================
`timescale 1ns/1ps

import uvm_pkg::*;
import uvmc_pkg::*;

// Include common utility package before agents
`include "Adder/utils_pkg.sv"

// Include transaction and generated agent
`include "Adder/Adder_trans.sv"
`include "Adder/xagent.sv"

// Interface for Adder signals  
interface adder_if;
    logic [127:0] a;
    logic [127:0] b;
    logic cin;
    wire [127:0] sum;   // Change to wire for output
    wire cout;          // Change to wire for output
    
    initial begin
        a = 128'h0;
        b = 128'h0;
        cin = 1'b0;
    end
endinterface

// Forward declaration
typedef class adder_python_monitor;

// Custom driver that integrates with Python and drives DUT
class adder_python_driver extends Adder_xdriver;
    `uvm_component_utils(adder_python_driver)
    
    virtual adder_if vif;
    
    function new(string name = "adder_python_driver", uvm_component parent = null);
        super.new(name, parent);
    endfunction
    
    virtual function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        if (!uvm_config_db#(virtual adder_if)::get(this, "", "vif", vif))
            `uvm_fatal("NOVIF", "Virtual interface not found")
    endfunction
    
    // Override sequence_receive to drive DUT
    virtual task sequence_receive(Adder_trans tr);
        `uvm_info("ADDER_DRV", $sformatf("Received from Python: a=0x%0x, b=0x%0x, cin=%0d", 
                  tr.a, tr.b, tr.cin), UVM_LOW)
        
        // Drive inputs to DUT
        vif.a = tr.a;
        vif.b = tr.b;
        vif.cin = tr.cin;
        
        `uvm_info("ADDER_DRV", "DUT inputs driven", UVM_MEDIUM)
    endtask
endclass

// Custom monitor that independently samples DUT and sends back to Python
class adder_python_monitor extends Adder_xmonitor;
    `uvm_component_utils(adder_python_monitor)
    
    virtual adder_if vif;
    Adder_trans prev_tr;
    
    function new(string name = "adder_python_monitor", uvm_component parent = null);
        super.new(name, parent);
    endfunction
    
    virtual function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        if (!uvm_config_db#(virtual adder_if)::get(this, "", "vif", vif))
            `uvm_fatal("NOVIF", "Virtual interface not found in monitor")
        prev_tr = Adder_trans::type_id::create("prev_tr");
    endfunction
    
    // Monitor independently samples interface
    virtual task run_phase(uvm_phase phase);
        Adder_trans tr;
        
        forever begin
            // Wait for signal change (input change indicates new transaction)
            @(vif.a or vif.b or vif.cin);
            
            // Wait for combinational logic to settle
            #1step;
            
            // Sample all signals
            tr = Adder_trans::type_id::create("tr");
            tr.a = vif.a;
            tr.b = vif.b;
            tr.cin = vif.cin;
            tr.sum = vif.sum;
            tr.cout = vif.cout;
            
            // Only send if this is a new transaction
            if (tr.a != prev_tr.a || tr.b != prev_tr.b || tr.cin != prev_tr.cin) begin
                sequence_send(tr);
                prev_tr.copy(tr);
                
                `uvm_info("ADDER_MON", $sformatf("Sampled: a=0x%0x, b=0x%0x, cin=%0d -> sum=0x%0x, cout=%0d",
                          tr.a, tr.b, tr.cin, tr.sum, tr.cout), UVM_LOW)
            end
        end
    endtask
endclass

// UVM Environment
class adder_env extends uvm_env;
    `uvm_component_utils(adder_env)
    
    Adder_xagent xagent;
    Adder_xagent_config xagent_config;
    virtual adder_if vif;
    
    function new(string name = "adder_env", uvm_component parent = null);
        super.new(name, parent);
        // Create and configure xagent config with factory pattern


    endfunction
    
    virtual function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        xagent_config = new("xagent_config");
        // Enable both monitor and driver
        xagent_config.is_active = UVM_ACTIVE;
        // Set config for xagent before it's built
        uvm_config_db#(Adder_xagent_config)::set(this, "xagent", "Adder_xagent_config", xagent_config);
        // Use factory overrides to use custom monitor/driver types
        set_type_override_by_type(Adder_xmonitor::get_type(), adder_python_monitor::get_type());
        set_type_override_by_type(Adder_xdriver::get_type(), adder_python_driver::get_type());

        xagent = Adder_xagent::type_id::create("xagent", this);
        if (!uvm_config_db#(virtual adder_if)::get(this, "", "vif", vif))
            `uvm_fatal("NOVIF", "Virtual interface not found")
    endfunction
endclass

// UVM Test
class adder_test extends uvm_test;
    `uvm_component_utils(adder_test)
    
    adder_env env;
    
    function new(string name = "adder_test", uvm_component parent = null);
        super.new(name, parent);
    endfunction
    
    virtual function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        env = adder_env::type_id::create("env", this);
    endfunction
    
    virtual task main_phase(uvm_phase phase);
        phase.raise_objection(this);
        `uvm_info("TEST", "Adder test started, waiting for Python to drive transactions...", UVM_LOW)
        #100000; // Keep simulation alive for Python
        phase.drop_objection(this);
    endtask
endclass

// Top module
module sv_main;
    logic clk;
    logic rst_n;
    
    // Create interface
    adder_if aif();
    
    // Instantiate DUT
    Adder dut (
        .a(aif.a),
        .b(aif.b),
        .cin(aif.cin),
        .sum(aif.sum),
        .cout(aif.cout)
    );
    
    // Generate clock (for UVM environment)
    initial begin
        clk = 0;
        forever #2 clk = ~clk;
    end
    
    // Generate reset
    initial begin
        rst_n = 0;
        #10 rst_n = 1;
    end
    
    initial begin
        // Set virtual interface
        uvm_config_db#(virtual adder_if)::set(null, "*", "vif", aif);

        // FSDB waveform dump for Verdi debugging
        $fsdbDumpfile("waves.fsdb");
        $fsdbDumpvars(0, sv_main);
        $fsdbDumpMDA();      // Dump multi-dimensional arrays
        $fsdbDumpSC();       // Dump SystemC signals (important for mixed design)

        // Run test
        run_test("adder_test");
    end
endmodule
