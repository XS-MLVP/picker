
# <image src="/image/picker-logo.png" width="32px" height="32px" />Picker: Pick your favorite language to verify your chip.

> A codegen tool for chip verification, which can provide C++/Python interfaces for the RTL designs.

中文 | [English](README.md)

## 介绍

picker是一个芯片验证辅助工具，有两个功能，其一是将RTL设计验证模块(.v/.scala/.sv)进行封装，并使用其他编程语言暴露Pin-Level的操作，未来计划支持自动化的Transaction-Level原语生成。其二是将UVM的的的transaction打包为一个uvm的agent和一个python类，通过xcomm和生成的文件可以实现UVM和Python的通信

支持的编程语言包括 C++, Python, Java, Scala, Golang 等。

该辅助工具让用户可以基于现有的软件测试框架，例如 pytest, junit，TestNG, go test等，进行芯片UT验证。

基于picker进行验证具有如下优点：

1. 不泄露RTL设计。经过picker转换后，原始的设计文件(.v)被转化成了二进制文件(.so)，脱离原始设计文件后，依旧可进行验证，且验证者无法获取RTL源代码。
2. 减少编译时间。当DUT(Design Under Test)稳定时，只需要编译一次（打包成so）。
3. 用户面广。提供的编程接口多，可覆盖不同语言的开发者（传统IC验证，只用System Verilog）。
4. 可使用软件生态丰富。能使用python3, java, golang等生态。
5. 自动化的打包UVM的transaction,实现UVM和Python的通信
目前picker支持的后端rtl仿真器如下：

1. verilator
2. synopsys vcs

# 使用方法

### 依赖安装

1.  [cmake](https://cmake.org/download/) ( >=3.11 )
2.  [gcc](https://gcc.gnu.org/) ( 需要支持c++20, 版本至少为10, **建议11及以上版本** )
3.  [python3](https://www.python.org/downloads/) ( >=3.8 )
4.  [verilator](https://verilator.org/guide/latest/install.html#git-quick-install) ( **==4.218** )
5.  [verible-verilog-format](https://github.com/chipsalliance/verible) ( >=0.0-3428-gcfcbb82b )
6.  [swig](http://www.swig.org/) ( >=**4.2.0**, 多语言支持 )

> 请注意，请确保`verible-verilog-format`等工具的路径已经添加到环境变量`$PATH`中，可以直接命令行调用。

### 下载源码

```bash
git clone https://github.com/XS-MLVP/picker.git
make init
```

### 构建并安装

```bash
cd picker
make
sudo -E make install
```

> 默认的安装的目标路径是 `/usr/local`， 二进制文件被置于 `/usr/local/bin`，模板文件被置于 `/usr/local/share/picker`。  
> 安装时会自动安装 `xspcomm`基础库（[https://github.com/XS-MLVP/xcomm](https://github.com/XS-MLVP/xcomm)），该基础库是用于封装 `RTL` 模块的基础类型，位于 `/usr/local/lib/libxspcomm.so`。 **可能需要手动设置编译时的链接目录参数(-L)**  
> 同时如果开启了python支持，还会安装 `xspcomm` 的python包，位于 `/usr/local/share/picker/python/xspcomm/`。 

安装完成后，执行`picker`命令可以得到以下输出:

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

### 安装测试

当前picker有pack和export两个子命令，你可以运行如下两条命令来检查他们的输出：

```bash
Export RTL Projects Sources as Software libraries such as C++/Python Usage: picker
export [OPTIONS] file

Positionals:
  file TEXT REQUIRED          DUT .v/.sv source file, contain the top module

Options:
  -h,--help                   Print this help message and exit
  --fs,--filelist TEXT        DUT .v/.sv source files, contain the top module, 
                              split by comma. Or use '*.txt' file  with one RTL 
                              file path per line to specify the file list
  --sim TEXT [verilator]      vcs or verilator as simulator, default is verilator
  --lang,--language TEXT:{python,cpp,java,scala,golang} [python] 
                              Build example project, default is python, choose 
                              cpp, java or python
  --sdir,--source_dir TEXT [/usr/local/share/picker/template] 
                              Template Files Dir, default is ${picker_install_path}/../picker/template
  --tdir,--target_dir TEXT [./picker_out] 
                              Codegen render files to target dir, default is ./picker_out
  --sname,--source_module_name TEXT
                              Pick the module in DUT .v file, default is the last 
                              module in the --fs marked file
  --tname,--target_module_name TEXT
                              Set the module name and file name of target DUT, 
                              default is the same as source. For example, --tname top, 
                              will generate UTtop.cpp and UTtop.hpp with UTtop class
  --internal TEXT             Exported internal signal config file, default is empty, 
                              means no internal pin
  -F,--frequency TEXT [100MHz] 
                              Set the frequency of the **only VCS** DUT, default 
                              is 100MHz, use Hz, KHz, MHz, GHz as unit
  -w,--wave_file_name TEXT    Wave file name, emtpy mean don't dump wave
  -c,--coverage               Enable coverage, default is not selected as OFF
  -V,--vflag TEXT             User defined simulator compile args, passthrough. 
                              Eg: '-v -x-assign=fast -Wall --trace' || '-C vcs -cc -f filelist.f'
  -C,--cflag TEXT             User defined gcc/clang compile command, passthrough. 
                              Eg:'-O3 -std=c++17 -I./include'
  --verbose                   Verbose mode
  -e,--example                Build example project, default is OFF
  --autobuild BOOLEAN [1]     Auto build the generated project, default is true
```

```bash
Pack uvm transaction as a uvm agent and python class
Usage: picker pack [OPTIONS] file...

Positionals:
  file TEXT ... REQUIRED      Sv source file, contain the transaction define

Options:
  -h,--help                   Print this help message and exit
  -e,--example                Generate example project based on transaction, default is OFF
  -c,--force                  Force delete folder when the code has already generated by picker
  -r,--rename TEXT ...        Rename transaction name in picker generate code

```

#### 参数解释
export: 
* `file`: 必需。DUT 的 Verilog 或 SystemVerilog 源文件，包含顶层模块
* `--filelist, -f`: 可选。DUT 的 Verilog 或 SystemVerilog 源文件，逗号分隔。也可以使用 `*.txt` 文件，每行指定一个 RTL 文件路径，来指定文件列表。
* `--sim`: 可选。模拟器类型，可以是 vcs 或 verilator，默认是 verilator。
* `--language, -l`: 可选。构建示例项目的语言，可以是 cpp 或 python，默认是 cpp。
* `--source_dir, -s`: 可选。模板文件目录，默认是 ${picker_install_path}/../picker/template。
* `--target_dir, -t`: 可选。渲染文件的目标目录，默认是 ./picker_out。
* `--source_module_name, -S`: 可选。在 DUT 的 Verilog 文件中选择模块，默认是  标记的文件中的最后一个模块。
* `--target_module_name, -T`: 可选。设置目标 DUT 的模块名和文件名，默认与源相同。例如，-T top 将生成 UTtop.cpp 和 UTtop.hpp，并包含 UTtop 类。
* `--internal`: 可选。导出的内部信号配置文件，默认为空，表示没有内部引脚。
* `--frequency, -F`: 可选。设置 仅 VCS DUT 的频率，默认是 100MHz，可以使用 Hz、KHz、MHz、GHz 作为单位。
* `--wave_file_name, -w`: 可选。波形文件名，为空表示不导出波形。
* `--vflag, -V`: 可选。用户定义的模拟器编译参数，透传。例如：'-v -x-assign=fast -Wall --trace' 或 '-f filelist.f'。
* `--cflag, -C`: 可选。用户定义的 gcc/clang 编译参数，透传。例如：'-O3 -std=c++17 -I./include'。
* `--verbose, -v`: 可选。详细模式，保留生成的中间文件。
* `--example, -e`: 可选。构建示例项目，默认是 OFF。
* `--autobuild`: 可选。自动构建生成的项目，默认是 true。
* `--help, -h`: 可选。打印使用帮助。

pack: 
* `file`: 必需。待解析的UVM transaction文件
* `--example, -e`: 可选。根据UVM的transaction生成示例项目。
* `--force， -c`: 可选。若以存在picker根据当前transaction解析出的文件，通过该命令可强制删除该文件，并重新生成
* `--rename, -r`: 可选。配置生成文件以及生成的agent的名称，默认为transactio名。

### 测试Examples

编译完成后，在picker目录执行以下命令，进行测试：
```
bash example/Adder/release-verilator.sh --lang cpp
bash example/Adder/release-verilator.sh --lang python

# 默认仅开启 cpp 和 Python 支持
#   支持其他语言编译命令为：make BUILD_XSPCOMM_SWIG=python,java,scala,golang
bash example/Adder/release-verilator.sh --lang java
bash example/Adder/release-verilator.sh --lang scala
bash example/Adder/release-verilator.sh --lang golang

bash example/RandomGenerator/release-verilator.sh --lang cpp
bash example/RandomGenerator/release-verilator.sh --lang python
bash example/RandomGenerator/release-verilator.sh --lang java
```

### 参考材料

如何基于picker进行芯片验证，可参考：[https://open-verify.cc/mlvp/docs/](https://open-verify.cc/mlvp/docs/)
