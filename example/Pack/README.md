# 快速开始
在安装好picker工具以后，可以通过picker的pack命令，根据指定的sequence生成一个UVM的agent和Python的class，分别在UVM和Python中import对应的agent并完成相应的配置，来实现UVM和Python的通信

## 安装picker
picker工具的安装可以参考open-verify.cc

安装完成后，执行`picker pack --help`命令可以得到以下输出:

```
XDut Generate. 
Convert DUT(*.v/*.sv) to C++ DUT libs.

Usage: ./build/bin/picker [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  -v,--version                Print version
  --show_default_template_path
                              Print default template path
  --show_xcom_lib_location_cpp
                              Print xspcomm lib and include location
  --show_xcom_lib_location_java
                              Print xspcomm-java.jar location
  --show_xcom_lib_location_scala
                              Print xspcomm-scala.jar location
  --show_xcom_lib_location_python
                              Print python module xspcomm location
  --show_xcom_lib_location_golang
                              Print golang module xspcomm location
  --check                     check install location and supproted languages

Subcommands:
  export                      Export RTL Projects Sources as Software libraries such as C++/Python
  pack                        Pack UVM transaction as a UVM agent and Python class
```
## picker的使用
通过`picker pack xxx.sv -e`即可生成对应agent和对应的uvm和python双向通信示例代码
文件结构如下：
```
sequence_name
|-- sequence_name_xagent.sv
|-- sequence_name_xagent.py
|-- example
|   |--example_uvm.sv
|   |--example_python.py
```

进入`example`目录，执行`make`命令即可运行


下面我们将通过一个加法器的sequence来演示如何实现通信，该部分代码位于/example/pack
# UVM发送到Python
进入/example/pack/目录下，执行`picker pack Adder_sequence.sv `,即可生成对应的文件，将/example/pack/Python2UVM，运行`make`命令
## UVM端
要实现UVM发送sequence到Python，
- 首先创建自己的 monitor 继承于Adder_sequence_xmonitor，然后实现sequence_send方法，sequence_send方法中，只需要构造出要发送的transaction
```
class example_monitor extends Adder_trans_xmonitor;
    `uvm_component_utils(example_monitor)
    virtual example_interface     vif;
    Adder_trans                   newtr;
    function new (string name = "example_monitor", uvm_component parent = null);
        super.new(name,parent);
    endfunction

    virtual function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        if(!uvm_config_db#(virtual example_interface)::get(this,"","vif",vif))
            `uvm_fatal("example_env","virtual interface must be set for vif")
    endfunction

  
    task sequence_send (Adder_trans tr);
        @(posedge vif.clk iff(vif.valid));
        newtr = new("newtr")
        tr.randomize();
        `uvm_info("example_monitor",$sformatf("uvm sub message: \n%s", tr.sprint()), UVM_LWO)
        
    endtask //sequence_receive
endclass
```
- 在env中实例化Adder_trans_xagent,Adder_trans_xagent_config
- Adder_trans_xagent_config 需要配置用于通信的端口名称和用户自己定义的driver和monitor的类型，在这里，我们将上面example_monitor的类型
```
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
```
## Python端
在python端，需要将创建对应的agent与与对应的处理接收到的sequence的方法
- agent中共需要三个参数,第一个参数为发送通道的名称，第二个和第三个参数为接收通道的名称和处理接收sequence的方法
- agent可以同时创建发送和接收两个通道，也可以根据需要只创建发送或者接收通道
```
import sys
sys.path.append('../')
from xsp_seq_xagent import *

if __name__ == "__main__":

    def receive_sequence(message):
        sequence = adder_trans(message)
        print("python receive sequence",sequence.a,sequence.b)

    env = Env("","adder_trans",receive_sequence)
    env.run(30)
```

# Python发送到UVM
## UVM端
要实现Python发送sequence到UVM，
- 首先创建自己的 driver 继承于Adder_sequence_xdriver，然后实现sequence_receive方法，用于处理接收到的sequence
```
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
```
- 在env中实例化Adder_trans_xagent,Adder_trans_xagent_config
- Adder_trans_xagent_config 需要配置用于通信的端口名称和用户自己定义的driver和monitor的类型，在这里，我们将上面example_monitor的类型
```
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
```
## Python端
在python端，需要将创建对应的agent与sequence
- 这里的agent只需要发送通道的名字
```
import sys
sys.path.append('../')
from xsp_seq_xagent import *

if __name__ == "__main__":

    agent = Agent("adder_trans")
    sequence = adder_trans()

    for i in range(10):
        sequence.a.value = i
        sequence.b.value = i + 1
        sequence.sum.value = sequence.a.value + sequence.b.value
        sequence.send(agent)
    
    agent.run(30)
```