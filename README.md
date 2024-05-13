
# <image src="/image/picker-logo.png" width="32px" height="32px" />Picker: Pick your favorite language to verify your chip.

> A codegen tool for chip verification, which can provide C++/Python interfaces for the RTL designs.

English | [中文](README.zh.md)

## Introduction

Picker is a chip verification auxiliary tool aimed at encapsulating RTL design verification modules (.v/.scala/.sv) and exposing Pin-Level operations through other programming languages, with plans to support automated Transaction-Level primitive generation in the future.

Other programming languages include interfaces for C++ (natively supported), Python (supported), Java/Scala (in progress), and Golang (in progress).

This auxiliary tool allows users to perform chip UT verification based on existing software testing frameworks, such as pytest, junit, TestNG, go test, etc.

The advantages of verifying with Picker include:

1. No leakage of RTL design. After conversion by Picker, the original design files (.v) are transformed into binary files (.so), allowing for verification without the original design files and preventing verifiers from accessing the RTL source code.
2. Reduced compilation time. When the DUT (Design Under Test) is stable, it only needs to be compiled once (packaged into .so).
3. Broad user base. The provided programming interfaces cover developers of different languages (traditional IC verification only uses System Verilog).
4. Rich software ecosystem. It can utilize ecosystems like Python3, Java, Golang, etc.

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

Run the command and check the output:

```bash
➜  picker git:(master) picker
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
      --version                 Print version
  -e, --example                 Build example project, default is OFF
      --autobuild               Auto build the generated project, default 
                                is true (default: true)
  -h, --help                    Print usage
```
