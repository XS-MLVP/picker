# Multi-language-based Chip Verification （picker）

#### 介绍

picker是一个芯片验证辅助工具，其目标是将RTL设计验证模块(.v/.scala/.sv)进行封装，并使用其他编程语言暴露Pin-Level的操作，未来计划支持自动化的Transaction-Level原语生成。

其他编程语言包括 c++ (原生支持), python(已支持), java(todo), golang(todo) 等编程语言接口。

该辅助工具让用户可以基于现有的软件测试框架，例如 pytest, junit，TestNG, go test等，进行芯片UT验证。

基于picker进行验证具有如下优点：

1. 不泄露RTL设计。经过picker转换后，原始的设计文件(.v)被转化成了二进制文件(.so)，脱离原始设计文件后，依旧可进行验证，且验证者无法获取RTL源代码。
2. 减少编译时间。当DUT(Design Under Test)稳定时，只需要编译一次（打包成so）。
3. 用户面广。提供的编程接口多，可覆盖不同语言的开发者（传统IC验证，只用System Verilog）。
4. 可使用软件生态丰富。能使用python3, java, golang等生态。

目前picker支持以下几种模拟器：

1. verilator
2. synopsys vcs

#### 使用方法

1.安装

**源码安装**
确保依赖 cmake(>=3.11)，gcc(支持c++20)，python3(>=3.8)，verilator(==4.218) 已经安装

```
# 下载源码
git clone <picker_git_url> #例如：https://gitee.com/yaozhicheng/picker.git

# 安装
make
sudo make install
```



2.使用案例

```
# 测试 examples
cd picker
./example/Cache/release-verilator.sh -l cpp -e -v 
```

picker命令参数说明如下：

```
XDut Generate. 
Convert DUT(*.v/*.sv) to C++ DUT libs. Notice that [file] option allow only one file.

Usage:
  XDut Gen [OPTION...] [file]

  -f, --filelist arg            DUT .v/.sv source files, contain the top 
                                module, split by comma.
                                Or use '*.txt' file  with one RTL file path 
                                per line to specify the file list (default: 
                                "")
      --sim arg                 vcs or verilator as simulator, default is 
                                verilator (default: verilator)
  -l, --language arg            Build example project, default is cpp, 
                                choose cpp or python (default: cpp)
  -s, --source_dir arg          Template Files Dir, default is 
                                ${picker_install_path}/../picker/template 
                                (default: /usr/local/share/picker/template)
  -t, --target_dir arg          Render files to target dir, default is 
                                ./picker_out (default: ./picker_out)
  -S, --source_module_name arg  Pick the module in DUT .v file, default is 
                                the last module in the -f marked file 
                                (default: "")
  -T, --target_module_name arg  Set the module name and file name of target 
                                DUT, default is the same as source. For 
                                example, -T top, will generate UTtop.cpp 
                                and UTtop.hpp with UTtop class (default: 
                                "")
      --internal arg            Exported internal signal config file, 
                                default is empty, means no internal pin 
                                (default: "")
  -F, --frequency arg           Set the frequency of the **only VCS** DUT, 
                                default is 100MHz, use Hz, KHz, MHz, GHz as 
                                unit (default: 100MHz)
  -w, --wave_file_name arg      Wave file name, emtpy mean don't dump wave 
                                (default: "")
  -V, --vflag arg               User defined simulator compile args, 
                                passthrough. Eg: '-v -x-assign=fast -Wall 
                                --trace' || '-C vcs -cc -f filelist.f' 
                                (default: "")
  -C, --cflag arg               User defined gcc/clang compile command, 
                                passthrough. Eg:'-O3 -std=c++17 
                                -I./include' (default: "")
  -v, --verbose                 Verbose mode
  -e, --example                 Build example project, default is OFF
  -h, --help                    Print usage

```