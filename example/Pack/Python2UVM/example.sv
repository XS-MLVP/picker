import uvm_pkg::*
import uvmc_pkg::*
`include "../Adder_sequence.sv"
`include "../Adder_xagent.sv"

class example_driver extends Adder_trans_xdriver;
    `uvm_component_utils(example_driver);
    uvm_analysis_port #(xsp_seq) ap;

    function new (string name = "example_driver", uvm_component parent = null);
        super.new(name,parent);
    endfunction

    virtual function void build_phase(uvm_phase phase);
        super.build_phase(phase);
    endfunction

    function sequence_receive(adder_trans tr);
        `uvm_info("example_driver", $sformatf("uvm receive message: \n%s",tr.sprint()), UVM_LOW)
    endfunction
endclass

class example_env extends uvm_env;
    `uvm_component_utils(example_env)
    Adder_trans_xagent            xagent;
    Adder_trans_xagent_config     xagent_config;
    example_driver                driver;
    
    function new (string name = "example_env", uvm_component parent = null);
        super.new(name,parent);
        xagent_config = new("xagent_config");
        xagent_config.drv_type = example_driver::get_type();
        uvm_config_db#(Adder_trans_xagent_config)::set(this,"xagent", "Adder_trans_xagent_config", xagent_config);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        xagent = Adder_trans_xagent::type_id::create("xagent",this);
    endfunction

    virtual task main_phase(uvm_phase phase);
        phase.raise_objection(this);
        for (int i = 0; i < 30; i++)begin
            @(posedge vif.clk)
            vif.data <= i;
            vif.valid <= 1'b1
        end
    endtask

endclass

class example_test extends uvm_test;
    `uvm_component_utils(example_test)
    example_env env;
    function new (string name = "example_test", uvm_component parent = null);
        super.new(name,parent)
    endfunction

    function void build_phase(uvm_phase);
        super.build_phase(phase);
        env = example_env::type_id::create("example_env",this);
    endfunction
endclass

module sv_main;
    logic clk;
    logic rst_n;
    example_interface vif(clk,rst_n);
    initial begin
        clk = 0;
        forever begin
            #2
            clk <= ~clk;
        end
    end

    initial begin
        rst_n <= 1'b0;
        #10
        rst_n <= 1'b0;
    end
    initial begin
        uvm_config_db #(virtual example_interface)::set(null,"uvm_test_top.example_env","vif","vif");
        run_test("example_test")
    end

endmodule