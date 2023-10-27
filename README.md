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
bash ./example/XDummyCache/release.sh

# 安装
# TBD, dont make install
```

2.使用案例


```
bash ./example/XDummyCache/release.sh

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