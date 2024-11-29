# <image src="/image/picker-logo.png" width="32px" height="32px" />Picker: 用你擅长的编程语言进行芯片验证.

> A codegen tool for chip verification, which can provide C++/Python/Java/Scala/Golang interfaces for the RTL designs.

中文 | [English](README.md)

## 介绍

**picker** 是一个芯片验证辅助工具，具有两个主要功能：

1. **打包 RTL 设计验证模块：** picker 可以将 RTL 设计验证模块（.v/.scala/.sv）打包成动态库，并提供多种高级语言（目前支持 C++、Python、Java、Scala、Golang）的编程接口来驱动电路。
1. **UVM-TLM 代码自动生成：** picker 能够基于用户提供的 UVM sequence_item 进行自动化的 TLM 代码封装，提供 UVM 与其他高级语言（如 Python）的通信接口。

该工具允许用户基于现有的软件测试框架，例如 pytest、junit、TestNG、go test 等，进行芯片单元测试。

基于 Picker 进行验证的优点

1. 不泄露 RTL 设计：经过 Picker 转换后，原始的设计文件（.v）被转化成了二进制文件（.so），脱离原始设计文件后，依旧可进行验证，且验证者无法获取 RTL 源代码。
1. 减少编译时间：当 DUT（设计待测）稳定时，只需要编译一次（打包成 .so 文件）。
1. 用户范围广：提供的编程接口多，覆盖不同语言的开发者。
1. 使用丰富的软件生态：支持 Python3、Java、Golang 等生态系统。
1. 自动化的 UVM 事务封装：通过自动化封装 UVM 事务，实现 UVM 和 Python 的通信。

Picker 目前支持的 RTL 仿真器：

1. Verilator
1. Synopsys VCS

# 使用方法

### 依赖安装

1.  [cmake](https://cmake.org/download/) ( >=3.11 )
2.  [gcc](https://gcc.gnu.org/) ( 需要支持 c++20, 版本至少为 10, **建议 11 及以上版本** )
3.  [python3](https://www.python.org/downloads/) ( >=3.8 )
4.  [verilator](https://verilator.org/guide/latest/install.html#git-quick-install) ( >=4.218 )
5.  [verible-verilog-format](https://github.com/chipsalliance/verible) ( >=0.0-3428-gcfcbb82b )
6.  [swig](http://www.swig.org/) ( >=**4.2.0**, 多语言支持 )

> 请注意，请确保`verible-verilog-format`等工具的路径已经添加到环境变量`$PATH`中，可以直接命令行调用。

### 下载源码

```bash
git clone https://github.com/XS-MLVP/picker.git --depth=1
cd picker
make init
```

### 构建并安装

```bash
cd picker
make
# 可通过 make BUILD_XSPCOMM_SWIG=python,java,scala,golang 开启其他语言支持。
# 各语言需要自己的开发环境，需要自行配置，例如javac等
sudo -E make install
```

> 默认的安装的目标路径是 `/usr/local`， 二进制文件被置于 `/usr/local/bin`，模板文件被置于 `/usr/local/share/picker`。
> 如果需要修改安装目录，可以通过指定 ARGS 给 cmake 传递参数，例如`make ARGS="-DCMAKE_INSTALL_PREFIX=your_instal_dir"`
> 安装时会自动安装 `xspcomm`基础库（[https://github.com/XS-MLVP/xcomm](https://github.com/XS-MLVP/xcomm)），该基础库是用于封装 `RTL` 模块的基础类型，位于 `/usr/local/lib/libxspcomm.so`。 **可能需要手动设置编译时的链接目录参数(-L)**  
> 如果开启了 Java 等语言支持，还会安装 `xspcomm` 对应的多语言软件包。

**picker 也可以编译为 wheel 文件，通过 pip 安装**

通过以下命令把 picker 打包成 wheel 安装包：

```bash
make wheel # or BUILD_XSPCOMM_SWIG=python,java,scala,golang make wheel
```

编译完成后，wheel 文件位于 dist 目录，然后通过 pip 安装，例如：

```bash
pip install dist/xspcomm-0.0.1-cp311-cp311-linux_x86_64.whl
pip install dist/picker-0.0.1-cp311-cp311-linux_x86_64.whl
```

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

当前 picker 有 export 和 pack 两个子命令。

export 子命令用于将 RTL 设计转换成其他高级编程语言对应的“库”，可以通过软件的方式进行驱动。

> $picker export --help

```bash
Export RTL Projects Sources as Software libraries such as C++/Python
Usage: picker export [OPTIONS] file...

Positionals:
  file TEXT ... REQUIRED      DUT .v/.sv source file, contain the top module

Options:
  -h,--help                   Print this help message and exit
  --fs,--filelist TEXT ...    DUT .v/.sv source files, contain the top module, split by comma.
                              Or use '*.txt' file  with one RTL file path per line to specify the file list
  --sim TEXT [verilator]      vcs or verilator as simulator, default is verilator
  --lang,--language TEXT:{python,cpp,java,scala,golang} [python]
                              Build example project, default is python, choose cpp, java or python
  --sdir,--source_dir TEXT    Template Files Dir, default is ${picker_install_path}/../picker/template
  --sname,--source_module_name TEXT ...
                              Pick the module in DUT .v file, default is the last module in the -f marked file
  --tname,--target_module_name TEXT
                              Set the module name and file name of target DUT, default is the same as source.
                              For example, -T top, will generate UTtop.cpp and UTtop.hpp with UTtop class
  --tdir,--target_dir TEXT    Target directory to store all the results. If it ends with '/' or is empty,
                              the directory name will be the same as the target module name
  --internal TEXT             Exported internal signal config file, default is empty, means no internal pin
  -F,--frequency TEXT [100MHz]
                              Set the frequency of the **only VCS** DUT, default is 100MHz, use Hz, KHz, MHz, GHz as unit
  -w,--wave_file_name TEXT    Wave file name, emtpy mean don't dump wave
  -c,--coverage               Enable coverage, default is not selected as OFF
  --cp_lib,--copy_xspcomm_lib BOOLEAN [1]
                              Copy xspcomm lib to generated DUT dir, default is true
  -V,--vflag TEXT             User defined simulator compile args, passthrough.
                              Eg: '-V -x-assign=fast -Wall --trace' || '-V '"-cm line -cm_dir /abs_path_to_store_coverage_data"''
  -C,--cflag TEXT             User defined gcc/clang compile command,   passthrough.
                              Eg:'-O3 -std=c++17 -I./include' || '-C vcs -cc -f filelist.f'||
  --verbose                   Verbose mode
  -e,--example                Build example project, default is OFF
  --autobuild BOOLEAN [1]     Auto build the generated project, default is true
```

pack 子命令用于将 UVM 中的 sequence_item 转换为其他语言，然后通过 TLM 进行通信（目前支持 Python，其他语言在开发中）

> $picker pack --help

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

##### export:

- `file TEXT ... REQUIRED`：必须。位置参数，DUT.v/.sv 源文件，包含顶层模块
- `-h,--help`: 可选。打印此帮助信息并退出
- `--fs,--filelist TEXT ...`: 可选。DUT .v/.sv 源文件，包含顶层模块，逗号分隔。或使用 '\*.txt' 文件，每行指定一个 RTL 文件路径来指定文件列表
- `--sim TEXT [verilator]`: 可选。使用 vcs 或 verilator 作为模拟器，默认是 verilator
- `--lang,--language TEXT:{python,cpp,java,scala,golang} [python]`: 可选。构建示例项目，默认是 python，可选择 cpp、java 或 python
- `--sdir,--source_dir TEXT`: 可选。模板文件目录，默认是 ${picker_install_path}/../picker/template
- `--sname,--source_module_name TEXT ...`: 可选。在 DUT .v 文件中选择模块，默认是 -f 标记的文件中的最后一个模块
- `--tname,--target_module_name TEXT`: 可选。设置目标 DUT 的模块名和文件名，默认与源相同。例如，-T top 将生成 UTtop.cpp 和 UTtop.hpp，并包含 UTtop 类
- `--tdir,--target_dir TEXT`: 可选。代码生成渲染文件的目标目录，默认为 DUT 的模块名。如果该参数以'/'结尾，则在该参数指定的目录中创建以 DUT 模块名的子目录。
- `--internal TEXT`: 可选。导出的内部信号配置文件，默认为空，表示没有内部引脚
- `-F,--frequency TEXT [100MHz]`: 可选。设置 **仅 VCS** DUT 的频率，默认是 100MHz，可以使用 Hz、KHz、MHz、GHz 作为单位
- `-w,--wave_file_name TEXT`: 可选。波形文件名，空表示不导出波形
- `-c,--coverage`: 可选。启用覆盖率，默认不选择为 OFF
- `--cp_lib,--copy_xspcomm_lib BOOLEAN [1]`: 可选。将 xspcomm 库复制到生成的 DUT 目录，默认是 true
- `-V,--vflag TEXT`: 可选。用户定义的模拟器编译参数，透传。例如：'-V -x-assign=fast -Wall --trace';模拟器使用 vcs 时，导出行覆盖率: '-V '"-cm line -cm_dir /abs_path_to_store_coverage_data"''
- `-C,--cflag TEXT`: 可选。用户定义的 gcc/clang 编译命令，透传。例如：'-O3 -std=c++17 -I./include'或'-C vcs -cc -f filelist.f'
- `--verbose`: 可选。详细模式
- `-e,--example`: 可选。构建示例项目，默认是 OFF
- `--autobuild BOOLEAN [1]`: 可选。自动构建生成的项目，默认是 true

静态多模块支持：

picker 在生成 dut_top.sv/v 的封装时，可以通过`--sname`参数指定多个模块名称和对应的数量。例如在 a.v 和 b.v 设计文件中分别有模块 A 和 B，需要 DUT 中有 2 个 A，3 个 B，生成的模块名称为 C（若不指定，默认名称为 A_B），则可执行如下命令：

```bash
picker path/a.v,path/b.v --sname A,2,B,3 --tname C
```

环境变量：

- `DUMPVARS_OPTION`: 设置`$dumpvars`的 option 参数。例如`DUMPVARS_OPTION="+mda" picker ....` 开启 vcs 中数组波形的支持。

##### pack:

- `file`: 必需。待解析的 UVM transaction 文件
- `--example, -e`: 可选。根据 UVM 的 transaction 生成示例项目。
- `--force， -c`: 可选。若已存在 picker 根据当前 transaction 解析出的文件，通过该命令可强制删除该文件，并重新生成
- `--rename, -r`: 可选。配置生成文件以及生成的 agent 的名称，默认为 transaction 名。

### 测试 Examples

编译完成后，在 picker 目录执行以下命令，进行测试：

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

如何基于 picker 进行芯片验证，可参考：[https://open-verify.cc/mlvp/docs/](https://open-verify.cc/mlvp/docs/)
