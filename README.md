# Multi-language-based Chip Verification （MCV）

#### 介绍

MCV是一个芯片验证辅助工具，其目标是将RTL设计验证模块(.v/.scala/.sv)进行封装，并使用其他编程语言暴露Pin-Level的操作，未来计划支持自动化的Transaction-Level原语生成。

其他编程语言包括 c++ (原生支持), python(已支持), java(todo), golang(todo) 等编程语言接口。

该辅助工具让用户可以基于现有的软件测试框架，例如 pytest, junit，TestNG, go test等，进行芯片UT验证。

基于MCV进行验证具有如下优点：

1. 不泄露RTL设计。经过MCV转换后，原始的设计文件(.v)被转化成了二进制文件(.so)，脱离原始设计文件后，依旧可进行验证，且验证者无法获取RTL源代码。
2. 减少编译时间。当DUT(Design Under Test)稳定时，只需要编译一次（打包成so）。
3. 用户面广。提供的编程接口多，可覆盖不同语言的开发者（传统IC验证，只用System Verilog）。
4. 可使用软件生态丰富。能使用python3, java, golang等生态。

目前MCV支持以下几种模拟器：

1. verilator
2. synopsys vcs

#### 使用方法

1.安装

**源码安装**
确保依赖 cmake(>=3.11)，gcc(支持c++20)，python3(>=3.8)，verilator(==4.218) 已经安装

```
# 下载源码
git clone <mcv_git_url> #例如：https://gitee.com/yaozhicheng/mcv.git

# 安装
make
sudo make install
```

# 测试 examples
cd mcv
./example/Cache/release-vcs.sh -e

2.使用案例

```

# 查看结果
tree temp
temp
├── build
│   ├── cpp.mk
│   ├── libVCache.a
│   ├── VCache___024root__DepSet_h420baa86__0.cpp
│   ├── VCache___024root__DepSet_h420baa86__0__Slow.cpp
│   ├── VCache___024root__DepSet_hc6909642__0.cpp
│   ├── VCache___024root__DepSet_hc6909642__0__Slow.cpp
│   ├── VCache___024root.h
│   ├── VCache___024root__Slow.cpp
│   ├── VCache__ALL.a
│   ├── VCache__ALL.cpp
│   ├── VCache__ALL.d
│   ├── VCache__ALL.o
│   ├── VCache_classes.mk
│   ├── VCache.cpp
│   ├── VCache__Dpi.cpp
│   ├── VCache__Dpi_Export__0.cpp
│   ├── VCache__Dpi.h
│   ├── VCache.h
│   ├── VCache.mk
│   ├── VCache__Syms.cpp
│   ├── VCache__Syms.h
│   ├── VCache__ver.d
│   ├── VCache__verFiles.dat
│   ├── verilated.d
│   ├── verilated_dpi.d
│   ├── verilated_dpi.o
│   ├── verilated.o
│   ├── verilated_threads.d
│   └── verilated_threads.o
├── Cache
├── Cache_top.sv
├── Cache_top.v
├── Cache.v
├── cpp.mk
├── data.cpp
├── data.hpp
├── dut_base.cpp
├── dut_base.hpp
├── dut.cpp
├── dut.hpp
├── libCache.so
├── libVCache.a
├── libVCache.S
├── Makefile
├── release
│   ├── Cache
│   ├── Cache_top.sv
│   ├── Cache_top.v
│   ├── Cache.v
│   ├── data.hpp
│   ├── dut_base.hpp
│   ├── dut.hpp
│   ├── libCache.so
│   └── util.hpp
├── util.hpp
├── VCache__Dpi.h
├── VCache.h
└── xdut_main.cpp

1 directories, 2 files
```

mcv命令参数说明如下：

```
XDut Generate. 
Convert DUT(*.v) to C++ DUT libs.

Usage:
  XDut Gen [OPTION...]

  -f, --file arg                DUT .v file
  -s, --source_dir arg          Template Files
  -t, --target_dir arg          Gen Files in the target dir
  -S, --source_module_name arg  Pick the module in DUT .v file, default is 
                                the last module (default: "")
  -T, --target_module_name arg  Set the module name and file name of target 
                                DUT, default is the same as source 
                                (default: "")
  -i, --internal arg            Exported internal signal config file, 
                                default is empty, means no internal pin 
                                (default: "")
  -F, --frequency arg           Set the frequency of the **only VCS** DUT, 
                                default is 100MHz, use Hz, KHz, MHz, GHz as 
                                unit (default: 100MHz)
  -w, --wave_file_name arg      Wave file name, emtpy mean don't dump wave 
                                (default: "")
      --sim arg                 VCS or verilator as simulator, default is 
                                verilator (default: verilator)
  -V, --vflag arg               User defined simulator compile args, split 
                                by comma. Eg: -v 
                                -x-assign=fast,-Wall,--trace. Or a file 
                                contain params. (default: "")
  -h, --help                    Print usage

```