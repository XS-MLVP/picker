# Multi-language-based Chip Verification （MCV）

#### 介绍

MCV是基于verilator的多语言转换工具，目标是将RTL设计验证模块(.v)，转换成 python(已支持), java（正在支持）, golang（待定）和 c++ (原生支持) 等编程语言接口。让用户可以基于现有的软件测试框架，例如 pytest, junit，TestNG, go test等，进行芯片验证。基于MCV进行验证具有如下优点：

1. 不泄露RTL设计。经过MCV转换后，原始的设计文件(.v)被转化成了二进制文件(.so)，脱离原始设计文件后，依旧可进行验证，且验证者无法获取源代码。
2. 减少编译时间。当DUT(Design Under Test)稳定时，只需要编译一次（打包成so）。
3. 用户面广。提供的编程接口多，可覆盖不同语言的开发者（传统IC验证，只用System Verilog）。
4. 可使用软件生态丰富。能使用python3, java, golang等生态。

缺点：MCV需要后端仿真器提交控制权，暴露编程接口。目前只适配verilator。

#### 使用方法

1.安装

**源码安装**
确保依赖 cmake(>=3.11)，g++(支持 c17)，python3(>=3.8)，pybind11(>=2.11.1)，verilator(>=4.226) 已经安装

```
# 下载源码
git clone <mcv_git_url> #例如：https://gitee.com/yaozhicheng/mcv.git

# 测试 examples
cd mcv
bash example/hello/cmd_python.bash
bash example/protect_lib/cmd_python.bash
bash example/tracing/cmd_python.bash

# 安装
bash script/build.sh
cd build
sudo make install/local
```

**二进制安装包**
下载二进制安装包(mcv.deb/)[TBD]

**二进制手动安装**
下载可执行文件mcv[TBD]
下载模板（项目中 template目录，包含生成python, java等库的模板文件）
执行如下命令
```
sudo mv mcv /usr/local/bin
sudo mv template /etc/mcv
```
mcv搜索模板的目录顺序为：".", "~/.mcv", "/etc/mcv"。因此也可以将模板目录放 ~/.mcv下仅给当前用户使用。


2.使用案例

安装完成后执行如下命令
```
mcv example/tracing/top.v -l python -t test -v example/tracing/vflags.txt -n trace -c -DVL_DEBUG=1

# 查看结果
tree test
test
└── python
    ├── Trace.py
    └── _Trace.cpython-38-x86_64-linux-gnu.so

1 directories, 2 files
```
上述命令将待验证文件top.v (DUT)转换成python库文件(.so)和对应的wrapper文件(.py)。基于该python接口可对DUT进行验证测试。具体编码可参考 example/tracing/tb.py


mcv命令参数说明如下：

```
Multi-language-based Chip Verification. 
Convert DUT(*.v) to other language libs.

Usage:
  mcv [OPTION...] <dut_file_to_convert>

  -l, --lang arg     Language to gen, select from [python, go, java, cpp], 
                     split by comma. Eg: --lang go,java. Default all
  -t, --target arg   Gen data in the target dir, default in current dir 
                     like _mcv_<name>_Ymd_HMS
  -n, --name arg     set mode name, default name is Lxx from lxx.v
  -v, --vflag arg    User defined verilator args, split by comma. Eg: -v 
                     -x-assign=fast,-Wall,--trace. Or a file contain 
                     params.
  -e, --exfiles arg  extend input files, split by comma. Eg: -e f1,f2.
  -c, --cppflag arg  User defined CPPFLAGS, split by comma. Eg: -c 
                     -Wall,-DVL_DEBUG. Or a file contain params.
  -h, --help         Print usage
  -d, --debug        Enable debuging
  -k, --keep         keep intermediate files
  -o, --overwrite    Force generate .cpp from .v

```

#### 其他

**一、MCV-python对比cocotb**

Cocotb [[https://www.cocotb.org/](https://www.cocotb.org/)]是COroutine based COsimulation TestBench的简称，是基于Python语言的新的IC验证平台，开源于2013年7月。

相同点：

1. 都是基于python进行IC验证
2. 都能使用python生态

不同点：

**1. cocotb是异步编程模式（coroutine），MCV提供的只是python接口，不和编程模式绑定**

由于cocotb为了兼容更多的后端（Icarus、Verilator、VCS等）采用VPI的方式和后端仿真器进行交互，DUT的实际驱动端在模拟器本身。例如对于verilator后端，cocotb的封装如下：
```
#from https://github.com/cocotb/cocotb/blob/master/cocotb/share/lib/verilator/verilator.cpp
...
    while (!Verilated::gotFinish()) {
        VerilatedVpi::callTimedCbs();
        settle_value_callbacks();
        bool again = true;
        while (again) {
            top->eval_step();
            again = settle_value_callbacks();
            again |= VerilatedVpi::callCbs(cbReadWriteSynch);
            again |= settle_value_callbacks();
        }
        top->eval_end_step();
...
```

上述基于VPI"回调"封装导致用户编写的cocotb-test不能直接控制程序主体，而是等待"被"调用。相对的，MCV直接基于c++接口提供python更底层接口，用户可以以最朴素，更直观的方式执行测试用例。基于MCV的pythond的test驱动示例如下（具体见 example/trace/tb.py）
```
...
    while not ctx.gotFinish():
        ctx.timeInc(1)
        top.clk = 0 if top.clk > 0 else 1
        if top.clk == 0:
            if ctx.time() > 1 and ctx.time() < 10:
                top.reset_l = 0
            else:
                top.reset_l = 1
            top.in_quad += 0x12
        top.eval()
...
```

**2. MCV对用户的要求更低**

由于MCV提供的是更底层的接口，没有和固定编程模式绑定，因此开发者不需要学习更高阶的coroutine就能进行用例开发。coroutine是python3提供的异步编程模式，在该模式下，用户需要学习coroutine、异步、await、task、trigger等概念。另外，MCV也可以在异步模式下使用（不和coroutine冲突）。


**3. MCV兼容性更好**

由于cocotb和coroutine进行了绑定，因此和一些其他框架兼容性不够。例如cocotb使用pytest软件测试框架时需要重新编译（[参考链接](https://docs.cocotb.org/en/stable/runner.html)）。相对的，MCV只需要一次加载对应os文件，的兼容性更好，可直接使用pytests进行DUT功能点测试。


**4. cocotb支持的后端仿真器更多**

由于cocotb采用VPI等systen verilog标准提供服务，因此只要支持对应标准的后端模拟器都有可能接入 cocotb，因此目前cocotb支持了目前大多数HDL仿真器。例如：Icarus、Verilator、VCS、Riviera、Active-HDL、Questa、Modelsim、Incisive、Xcelium、GHDL、CVC。 （[参考链接](https://docs.cocotb.org/en/stable/simulator_support.html)）。目前MCV仅支持verilator后端。

**5. MCV验证逻辑是先编译后写测试，而cocotb相反**

cocotb官方给的标准流程是：
```
# https://docs.cocotb.org/en/stable/rescap.html#the-cocotb-part-of-the-testbench
1 编写 test
2 编译DUT并运行 make SIM=xcelium TOPLEVEL=tb_rescap MODULE=test_rescap_minimalist 
```

MCV的测试流程为：
```
1. 编译 DUT 成 so文件
2. 编写test并执行 // 执行test时不需要原始设计文件 (.v)
```

在DUT不变的情况下，MCV中DUT的编译只需要一次，不用每次修改test时都编译，能缩减测试时间（当前基于system verilog的验证，每次test需改都需要重新编译）。

**6. MCV提供Python的wrapper IDE开发更友好**

基于cocotb在编写测试时，没有对应的DUT相关的python文件，查询dut中的输入输出时，需要人为的通过RTL设计文件获取。而MCV-python会根据dut生产对应的python wrapper，IDE（例如vscode）可以基于该wrapper进行变量名称等提示。
