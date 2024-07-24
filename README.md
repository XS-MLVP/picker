
# <image src="/image/picker-logo.png" width="32px" height="32px" />Picker: Pick your favorite language to verify your chip.

> A codegen tool for chip verification, which can provide C++/Python interfaces for the RTL designs.

English | [中文](README.zh.md)

## Introduction

**picker** is a chip verification auxiliary tool with two main functions:

1. **Expot RTL designs into verification modules:** Picker can convert RTL design verification modules (.v/.scala/.sv) into dynamic libraries and provide programming interfaces in multiple high-level languages (currently supporting C++, Python, Java, Scala, Golang) to drive the circuits.
1. **Pack UVM-TLM transaction with other languages:** Picker can automatically encapsulate TLM code based on user-provided UVM sequence_items, providing communication interfaces between UVM and other high-level languages (such as Python).

This tool allows users to perform chip unit testing based on existing software testing frameworks, such as pytest, junit, TestNG, go test, etc.

Advantages of verification based on picker:
1. No RTL design leakage: After conversion by picker, the original design files (.v) are transformed into binary files (.so). Verification can still be performed without the original design files, and the verifier cannot access the RTL source code.
1. Reduced compilation time: When the DUT (Design Under Test) is stable, it only needs to be compiled once (packaged into a .so file).
1. Wide user range: Provides multiple programming interfaces, covering developers of different languages.
1. Utilizes rich software ecosystems: Supports ecosystems such as Python3, Java, Golang.
1. Automated UVM transaction encapsulation: Achieves communication between UVM and Python through automated UVM transaction encapsulation.

Currently supported RTL simulators by picker:
1. Verilator
1. Synopsys VCS

## How to Use

### Dependency Installation

1. [CMake](https://cmake.org/download/) (>=3.11)
2. [GCC](https://gcc.gnu.org/) (needs to support C++20, at least version 10, **recommended version 11 or above**)
3. [Python3](https://www.python.org/downloads/) (>=3.8)
4. [Verilator](https://verilator.org/guide/latest/install.html#git-quick-install) (**==4.218**)
5. [Verible Verilog Format](https://github.com/chipsalliance/verible) (>=0.0-3428-gcfcbb82b)
6. [SWIG](http://www.swig.org/) (>=**4.2.0**, multi-language support)

> Please ensure that the path to tools like `verible-verilog-format` is added to the environment variable `$PATH` for direct command-line invocation.

### Source Code Download

```bash
git clone https://github.com/XS-MLVP/picker.git --depth=1
cd picker
make init
```

### Build and Install

```bash
cd picker
make
# You can enable support for other languages by 
#   using `make BUILD_XSPCOMM_SWIG=python,java,scala,golang`.
# Each language requires its own development environment, 
#   which needs to be configured separately, such as `javac` for Java.
sudo -E make install
```

> The default installation path is `/usr/local`, with binary files placed in `/usr/local/bin` and template files in `/usr/local/share/picker`.  
> The installation will automatically install the `xspcomm` base library (https://github.com/XS-MLVP/xcomm), which is used to encapsulate the basic types of `RTL` modules, located at `/usr/local/lib/libxspcomm.so`. **You may need to manually set the link directory parameters (-L) during compilation.**   
> If support for languages such as Java is enabled, the corresponding `xspcomm` multi-language packages will also be installed.  

After installation, execute the `picker` command to except the flow output:

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

### Installation Test

picker currently has two subcommands: `export` and `pack`.

The `export` subcommand is used to convert RTL designs into "libraries" corresponding to other high-level programming languages, which can be driven through software.

> $picker export --help

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

The `pack` subcommand is used to convert UVM `sequence_item` into other languages and then communicate through TLM (currently supports Python, other languages are under development).

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

### Test Examples
After picker compilation, execute the following commands in the picker directory to test the examples:

```
bash example/Adder/release-verilator.sh --lang cpp
bash example/Adder/release-verilator.sh --lang python

# Default enable cpp and python
#  for other languages support：make BUILD_XSPCOMM_SWIG=python,java,scala,golang
bash example/Adder/release-verilator.sh --lang java
bash example/Adder/release-verilator.sh --lang scala
bash example/Adder/release-verilator.sh --lang golang

bash example/RandomGenerator/release-verilator.sh --lang cpp
bash example/RandomGenerator/release-verilator.sh --lang python
bash example/RandomGenerator/release-verilator.sh --lang java
```


### More Documents

For guidance on chip verification with picker, please refer to: [https://open-verify.cc/mlvp/en/docs/](https://open-verify.cc/mlvp/en/docs/)
