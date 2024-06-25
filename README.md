
# <image src="/image/picker-logo.png" width="32px" height="32px" />Picker: Pick your favorite language to verify your chip.

> A codegen tool for chip verification, which can provide C++/Python interfaces for the RTL designs.

English | [中文](README.zh.md)

## Introduction

Picker is a chip verification auxiliary tool. There are two functions: the first is aimed at encapsulating RTL design verification modules (.v/.scala/.sv) and exposing Pin-Level operations through other programming languages, with plans to support automated Transaction-Level primitive generation in the future. the second is pack uvm transaction as a uvm agent and python class, you can use them and xcomm tool to enable communication between UVM and Python.

Other programming languages include interfaces for C++ (natively supported), Python (supported), Java (supported), Scala (in progress), and Golang (in progress).

This auxiliary tool allows users to perform chip UT verification based on existing software testing frameworks, such as pytest, junit, TestNG, go test, etc.

The advantages of verifying with Picker include:

1. No leakage of RTL design. After conversion by Picker, the original design files (.v) are transformed into binary files (.so), allowing for verification without the original design files and preventing verifiers from accessing the RTL source code.
2. Reduced compilation time. When the DUT (Design Under Test) is stable, it only needs to be compiled once (packaged into .so).
3. Broad user base. The provided programming interfaces cover developers of different languages (traditional IC verification only uses System Verilog).
4. Rich software ecosystem. It can utilize ecosystems like Python3, Java, Golang, etc.
5. Automate the packaging of UVM transactions to enable communication between UVM and Python.

Currently, Picker supports the following backend RTL simulators:

1. Verilator
2. Synopsys VCS

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
git clone https://github.com/XS-MLVP/picker.git
cd picker
make init
```

### Build and Install

```bash
cd picker
make
sudo -E make install
```

> The default installation path is `/usr/local`, with binary files placed in `/usr/local/bin` and template files in `/usr/local/share/picker`.  
> The installation will automatically install the `xspcomm` base library (https://github.com/XS-MLVP/xcomm), which is used to encapsulate the basic types of `RTL` modules, located at `/usr/local/lib/libxspcomm.so`. **You may need to manually set the link directory parameters (-L) during compilation.**   
> Additionally, if Python support is enabled (default), the `xspcomm` Python package will also be installed, located at `/usr/local/share/picker/python/xspcomm/`.  


### Installation Test

Now picker has two subcommands: pack and export. You can run the following commands to check their output:

```bash
Export RTL Projects Sources as Software libraries such as C++/Python
Usage: ./build/bin/picker export [OPTIONS] file

Positionals:
  file TEXT REQUIRED          DUT .v/.sv source file, contain the top module

Options:
  -h,--help                   Print this help message and exit
  --fs,--filelist TEXT        DUT .v/.sv source files, contain the top module, split by comma.
                              Or use '*.txt' file  with one RTL file path per line to specify the file list
  --sim TEXT [verilator]      vcs or verilator as simulator, default is verilator
  --lang,--language TEXT [python] 
                              Build example project, default is python, choose cpp, java or python
  --sdir,--source_dir TEXT [/home/yaozhicheng/workspace/picker/template] 
                              Template Files Dir, default is ${picker_install_path}/../picker/template
  --tdir,--target_dir TEXT [./picker_out] 
                              Codegen render files to target dir, default is ./picker_out
  --sname,--source_module_name TEXT
                              Pick the module in DUT .v file, default is the last module in the -f marked file
  --tname,--target_module_name TEXT
                              Set the module name and file name of target DUT, default is the same as source. For example, -T top, will generate UTtop.cpp and UTtop.hpp with UTtop class
  --internal TEXT             Exported internal signal config file, default is empty, means no internal pin
  -F,--frequency TEXT [100MHz] 
                              Set the frequency of the **only VCS** DUT, default is 100MHz, use Hz, KHz, MHz, GHz as unit
  -w,--wave_file_name TEXT    Wave file name, emtpy mean don't dump wave
  -c,--coverage               Enable coverage, default is not selected as OFF
  -V,--vflag TEXT             User defined simulator compile args, passthrough. Eg: '-v -x-assign=fast -Wall --trace' || '-C vcs -cc -f filelist.f'
  -C,--cflag TEXT             User defined gcc/clang compile command, passthrough. Eg:'-O3 -std=c++17 -I./include'
  --verbose                   Verbose mode
  -e,--example                Build example project, default is OFF
  --autobuild [1]             Auto build the generated project, default is true
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

### Test Examples
After picker compilation, execute the following commands in the picker directory to test the examples:

```
bash example/Adder/release-verilator.sh --lang cpp
bash example/Adder/release-verilator.sh --lang python
bash example/Adder/release-verilator.sh --lang java
bash example/RandomGenerator/release-verilator.sh --lang cpp
bash example/RandomGenerator/release-verilator.sh --lang python
bash example/RandomGenerator/release-verilator.sh --lang java
```


### More Documents

For guidance on chip verification with picker, please refer to: [https://open-verify.cc/mlvp/en/docs/](https://open-verify.cc/mlvp/en/docs/)
