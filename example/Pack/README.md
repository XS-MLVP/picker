# 快速开始
使用`picker pack`功能可以将UVM环境进行打包成动态库，并通过TLM2.0进行和其他高级语言的数据通信
picker可以解析UVM中的sequence，生成一个UVM的agent组件和Python的agent类。agent中需要用户实现sequence的构造和处理接收到的sequence。具体的使用方法我们后面再讲。首先，我们来介绍picker工具的安装

## 安装picker

手动安装picker以及相关依赖，可以参考网站 open-verify.cc

在北京开源芯片研究院大机房上使用时，我们提供了安装picker所需要的依赖环境，路径在/nfs/home/songfangyuan/picker_workspace
通过`source set_picker_env.sh`命令，可以配置安装picker环境变量，并将picker源码复制到当前路径下，之后执行
```
cd picker
busb -Is make
busb -Is make install
```
大机房上没有root权限，需要安装到用户自己的.local下，需要在修改/picker/Makefile中第17行, 在原有的基础上加上如下命令
```
-DCMAKE_INSTALL_PREFIX=/nfs/home/songfangyuan/.local/
```
即可完成安装

由于安装picker需要gcc11以上的版本，使用picker将UVM环境打包，需要使用特定的gcc版本，如gcc7.3, gcc6.2, gcc5.2等，以及uvmc
使用`source set_picker_env.sh pack`可以将上述内容复制到当前路径下，并配置到环境变量中

## 测试Examples

```
# send: UVM send some sequence to Python
bash example/Pack/release-pack.sh --send
# receive: Python send some sequence to UVM
bash example/Pack/release-pack.sh --receive
# both: UVM send some sequence to Python , then Python send them to UVM and compare
bash example/Pack/release-pack.sh --both
```

## picker Pack 介绍
通过`picker pack xxx_sequence.sv -e`即可生成对应agent和对应的uvm和python双向通信示例代码

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

- UVM agent 包括xmonitor和xdriver两个基类，xagent_config配置类，通过TLM2.0 将sequence打包成字节流发送到Python，以及接收Python发送来的字节流并解析为sequence，用户可以根据需要实现 UVM => Python, Python => UVM, UVM <=> Python的数据传输
    - xmonitor负责从interface上读取sequence并发送到Python，其中的sequence_send()用于构建待发送的sequence，需要由用户实现
    - xdriver负责接收Python发送来的sequence，其中的sequence_receive()用于c处理接收到的sequence()，根据需求由用户实现
    - 用户需要继承基类创建自己的driver，monitor组件，并实现上述方法
    - 在实例化agent时，通过xagent_config,将用户定义的driver和monitor组件配置给agent
- Python agent 包含驱动UVM的Agent(),和sequence的定义

通过`picker pack xxx_sequence.sv -e` 的`-e`参数可以产生`xxx_sequence`对应的UVM和Python的双向通信的示例，下面两个例子演示UVM和Python之间的单向通信
下面我们将通过一个加法器的sequence来演示如何实现通信，该部分代码位于/example/pack目录下
# UVM发送到Python
进入/example/pack/目录下，执行`picker pack adder_trans.sv `,即可生成对应的文件，将/example/pack/Python2UVM，运行`make`命令
## UVM端
要实现UVM发送sequence到Python，
- 首先创建自己的 monitor 继承于Adder_sequence_xmonitor，然后实现sequence_send方法，sequence_send方法中，只需要构造出要发送的transaction
```
class example_monitor extends adder_trans_xmonitor;
    `uvm_component_utils(example_monitor)
    virtual example_interface     vif;
    adder_trans                   newtr;
    function new (string name = "example_monitor", uvm_component parent = null);
        super.new(name,parent);
    endfunction

    virtual function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        if(!uvm_config_db#(virtual example_interface)::get(this,"","vif",vif))
            `uvm_fatal("example_env","virtual interface must be set for vif")
    endfunction

  
    task sequence_send (adder_trans tr);
        @(posedge vif.clk iff(vif.valid));
        newtr = new("newtr");
        tr.randomize();
        `uvm_info("example_monitor",$sformatf("uvm send sequence: \n%s", tr.sprint()), UVM_LOW)
        
    endtask //sequence_receive
endclass
```
- 在env中实例化Adder_trans_xagent,Adder_trans_xagent_config
- Adder_trans_xagent_config 需要配置用于通信的端口名称和用户自己定义的driver和monitor的类型，在这里，我们将上面example_monitor的类型
```
class example_env extends uvm_env;
    `uvm_component_utils(example_env)
    adder_trans_xagent            xagent;
    adder_trans_xagent_config     xagent_config;
    virtual example_interface     vif;
    
    function new (string name = "example_env", uvm_component parent = null);
        super.new(name,parent);
        xagent_config = new("xagent_config");
        xagent_config.mon_type = example_monitor::get_type();
        uvm_config_db#(adder_trans_xagent_config)::set(this,"xagent", "adder_trans_xagent_config", xagent_config);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        xagent = adder_trans_xagent::type_id::create("xagent",this);
        if(!uvm_config_db#(virtual example_interface)::get(this,"","vif",vif))
            `uvm_fatal("example_env","virtual interface must be set for vif")
    endfunction

    virtual task main_phase(uvm_phase phase);
        phase.raise_objection(this);
        for (int i = 0; i < 30; i++)begin
            @(posedge vif.clk)
            vif.data <= i;
            vif.valid <= 1'b1;

            @(posedge vif.clk)
            vif.valid <= 1'b0;
        end
    endtask

endclass
```
## Python端
在python端，需要将创建对应的agent与与对应的处理接收到的sequence的方法
- agent中共需要三个参数,第一个参数为发送通道的名称，第二个和第三个参数为接收通道的名称和处理接收sequence的方法
- agent可以同时创建发送和接收两个通道，也可以根据需要只创建发送或者接收通道
```
if __name__ == "__main__":

    def receive_sequence(message):
        sequence = adder_trans(message)
        print("python receive sequence",sequence.a,sequence.b)
        
    agent = Agent("","adder_trans",receive_sequence)
    
    agent.run(200)

```

# Python发送到UVM
## UVM端
要实现Python发送sequence到UVM，
- 首先创建自己的 driver 继承于Adder_sequence_xdriver，然后实现sequence_receive方法，用于处理接收到的sequence
```
class example_driver extends adder_trans_xdriver;
    `uvm_component_utils(example_driver);

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
    adder_trans_xagent            xagent;
    adder_trans_xagent_config     xagent_config;
    virtual example_interface     vif;
    
    function new (string name = "example_env", uvm_component parent = null);
        super.new(name,parent);
        xagent_config = new("xagent_config");
        xagent_config.drv_type = example_driver::get_type();
        uvm_config_db#(adder_trans_xagent_config)::set(this,"xagent", "adder_trans_xagent_config", xagent_config);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        xagent = adder_trans_xagent::type_id::create("xagent",this);
        if(!uvm_config_db#(virtual example_interface)::get(this,"","vif",vif))
            `uvm_fatal("example_env","virtual interface must be set for vif")
    endfunction

    virtual task main_phase(uvm_phase phase);
        phase.raise_objection(this);
        for (int i = 0; i < 30; i++)begin
            @(posedge vif.clk)
            vif.data <= i;
            vif.valid <= 1'b1;

            @(posedge vif.clk)
            vif.valid <= 1'b0;
        end
    endtask

endclass
```
## Python端
在python端，需要将创建对应的agent与sequence
- 这里的agent只需要发送通道的名字
```
if __name__ == "__main__":

    agent = Agent("adder_trans")
    
    for i in range(10):
        sequence = adder_trans()
        sequence.a.value = i
        sequence.b.value = i + 1
        sequence.send(agent)
        agent.run(1)
    

```

## UVM环境中集成picker

TBD