# 安装 Picker

运行 Python + Verilator 示例需要 Picker、Verilator、SWIG、C++ 编译器和 Python 开发环境。下面给出源码安装步骤；已有编译环境时，也可以使用页面末尾的发行包。

## 准备工具

| 工具 | 要求或用途 |
| --- | --- |
| C++ 编译器 | 支持 C++20，例如 GCC 11 或更新版本 |
| CMake、make、Git | 配置、编译和获取依赖 |
| Python 和开发头文件 | 构建 Python 扩展；编译和运行使用同一个 Python 环境 |
| SWIG | 4.2 或更新版本，用于生成语言绑定 |
| Verilator | 5.020 或更新版本，用于编译 RTL |
| bison、flex、gperf | 编译依赖工具 |
| zlib、lz4 开发包 | 压缩库依赖 |

Verilator 的安装步骤见[官方安装指南](https://verilator.org/guide/latest/install.html)，SWIG 见[官方下载页](https://www.swig.org/download.html)。安装完成后检查：

```sh
c++ --version
cmake --version
python3 --version
swig -version
verilator --version
```

macOS 可以用 Homebrew 安装工具：

```sh
brew install cmake pkgconf python3 swig verilator zlib lz4
```

## 编译并安装

获取源码和依赖，安装到用户目录：

```sh
git clone https://github.com/XS-MLVP/picker.git
cd picker
make init
make install NPROC=4 BUILD_XSPCOMM_SWIG=python \
    ARGS="-DCMAKE_INSTALL_PREFIX=$HOME/.local/picker"
export PATH="$HOME/.local/picker/bin:$PATH"
```

`make init` 下载 `dependence/` 中的依赖。`NPROC=4` 使用四个编译任务，内存较少时可以调低。将上面的 `export PATH=...` 加入 shell 配置后，新终端也能找到 Picker。

只在源码目录使用时，可以将 `make install` 换成 `make build`，随后通过 `./build/bin/picker` 执行命令。

## 检查安装

```sh
picker --version
picker --check
```

`--check` 列出模板和各语言资源的检查结果。运行本教程需要模板和 Python 项检查成功；其他语言可以在需要时添加。

接下来按[加法器教程](../tutorials/first-dut.md)导出并运行一个模型，检查整个编译和仿真过程。

## 使用发行包

[GitHub Releases](https://github.com/XS-MLVP/picker/releases) 提供 Linux AppImage、安装目录压缩包和 Python wheel。按主机架构选择文件；wheel 还需要匹配 Python 小版本。

使用 AppImage 时，赋予下载文件执行权限后运行。使用安装目录压缩包时，完整解压并将其中的 `bin` 加入 `PATH`。模板和运行资源通过安装目录定位，分发时一起保留。

安装 wheel 时，将 Picker 和发行说明中列出的配套依赖 wheel 放入 `wheels/`，再执行：

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install --no-index --find-links=wheels picker
python -m pip check
picker --check
```

导出新模型仍需要本页列出的 Verilator、SWIG 和编译工具。

## 自行构建 wheel

在已执行 `make init` 的源码目录中运行：

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install build
make wheel PYTHON="$VIRTUAL_ENV/bin/python"
python -m pip install --no-index --find-links=dist picker
python -m pip check
```

生成的 wheel 位于 `dist/`，可复制到使用相同 Python 小版本和平台的环境中安装。
