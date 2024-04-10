# Picker

> A codegen tool for chip verification, which can convert RTL design to C++/Python DUT lib.

## 介绍

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

# 使用方法

### 依赖安装

1.  [cmake](https://cmake.org/download/) ( >=3.11 )
2.  [gcc](https://gcc.gnu.org/) ( 支持c++20,至少为10, **最好为11及以上** )
3.  [python3](https://www.python.org/downloads/) ( >=3.8 )
4.  [verilator](https://verilator.org/guide/latest/install.html#git-quick-install) ( **==4.218** )
5.  [verible-verilog-format](https://github.com/chipsalliance/verible) ( >=0.0-3428-gcfcbb82b )
6.  [swig](http://www.swig.org/) ( >=**4.2.0**, 目前为master分支， 仅在需要python支持时使用 )

> 请注意，请确保`verible-verilog-format`等工具的路径已经添加到环境变量`$PATH`中，可以直接命令行调用。

### 下载源码

```bash
git clone https://github.com/XS-MLVP/picker.git
git clone https://github.com/XS-MLVP/xcomm.git
cd picker/dependence
rmdir xcomm/
ln -s ../../xcomm xcomm
```

### 构建并安装

```bash
cd picker
export BUILD_XSPCOMM_SWIG=python # 仅在需要python支持时使用
make
sudo -E make install
```

> 默认的安装的目标路径是 `/usr/local`， 二进制文件被置于 `/usr/local/bin`，模板文件被置于 `/usr/local/share/picker`。  
> 安装时会自动安装 `xspcomm` 基础库，该基础库是用于封装 `RTL` 模块的基础类型，位于 `/usr/local/lib/libxspcomm.so`。 **可能需要手动设置编译时的链接目录参数(-L)**
> 同时如果开启了python支持，还会安装 `xspcomm` 的python包，位于 `/usr/local/share/picker/python/xspcomm/`。 

### 安装测试

执行命令并检查输出：

```bash
➜  picker git:(master) picker
XDut Generate. 
Convert DUT(*.v/*.sv) to C++ DUT libs. Notice that [file] option allow only one file.

Usage:
  XDut Gen [file] [OPTION...] 

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
  -c, --coverage                Enable coverage, default is not selected as 
                                OFF
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

#### 参数解释

* `[file]`: 必需。DUT 的 Verilog 或 SystemVerilog 源文件，包含顶层模块
* `--filelist, -f`: 可选。DUT 的 Verilog 或 SystemVerilog 源文件，逗号分隔。也可以使用 `*.txt` 文件，每行指定一个 RTL 文件路径，来指定文件列表。
* `--sim`: 可选。模拟器类型，可以是 vcs 或 verilator，默认是 verilator。
* `--language, -l`: 可选。构建示例项目的语言，可以是 cpp 或 python，默认是 cpp。
* `--source_dir, -s`: 可选。模板文件目录，默认是 ${mcv_install_path}/../mcv/template。
* `--target_dir, -t`: 可选。渲染文件的目标目录，默认是 ./mcv_out。
* `--source_module_name, -S`: 可选。在 DUT 的 Verilog 文件中选择模块，默认是  标记的文件中的最后一个模块。
* `--target_module_name, -T`: 可选。设置目标 DUT 的模块名和文件名，默认与源相同。例如，-T top 将生成 UTtop.cpp 和 UTtop.hpp，并包含 UTtop 类。
* `--internal`: 可选。导出的内部信号配置文件，默认为空，表示没有内部引脚。
* `--frequency, -F`: 可选。设置 仅 VCS DUT 的频率，默认是 100MHz，可以使用 Hz、KHz、MHz、GHz 作为单位。
* `--wave_file_name, -w`: 可选。波形文件名，为空表示不导出波形。
* `--vflag, -V`: 可选。用户定义的模拟器编译参数，透传。例如：'-v -x-assign=fast -Wall --trace' 或 '-f filelist.f'。
* `--cflag, -C`: 可选。用户定义的 gcc/clang 编译参数，透传。例如：'-O3 -std=c++17 -I./include'。
* `--verbose, -v`: 可选。详细模式，保留生成的中间文件。
* `--example, -e`: 可选。构建示例项目，默认是 OFF。
* `--help, -h`: 可选。打印使用帮助。