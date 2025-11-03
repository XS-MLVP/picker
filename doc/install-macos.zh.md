# 在 macOS 上安装 Picker

## 已知限制

由于 macOS 与 Linux 在系统内核和开发工具链方面存在差异，Picker 在 macOS 上使用时会有一些功能限制：

1. 仿真器仅支持 Verilator
2. 不支持动态多实例，如需多实例请使用静态多实例

因此，我们强烈建议在 Linux 环境下使用 Picker，以获得更全面的功能支持。

## 依赖安装

> [!WARNING]
> 我们建议通过 [Homebrew](https://brew.sh/) 来安装 macOS 上的 Picker 依赖。如果您没有使用 Homebrew 进行依赖配置，遇到的问题需要您自行排查解决。
>
> 以下说明均基于已正确配置 Homebrew 的环境。

### 配置 Verible

请确保已经正确配置了 [Verible](https://github.com/chipsalliance/verible) 的环境变量。

### 安装依赖项

```bash
brew install cmake lcov lld llvm pkgconf python3 swig systemc verilator zlib
```

上述依赖都是 Picker 在导出 C++ 和 Python 的 DUT 时所需的。其中：

- `llvm` 不是必选项，您也可以使用系统自带的 `clang`（AppleClang），但在编译较大规模 DUT 时，AppleClang 的编译速度通常较慢。
- `lld` 也非必选，但它能够加快链接速度。如果不安装，系统将使用默认的 `ld` （Apple Linker）。
- 如果您的系统存在多个 Python 版本，请确保 `python`、`python-config` 等工具不会在不同版本间混用，以避免因 ABI 不匹配引起的编译错误。

如果您想启用对其他语言的支持（例如 Java、Go、Lua、Scala2 等），请自行配置相关的工具链，这里不再详细说明。

## 构建

> [!TIP]
> 请参考项目根目录中的 [README.md](README.md) 了解构建步骤。

在这里，将演示把 picker 安装到 `~/Applications/Picker` 文件夹下。

首先，执行下面的命令进行编译安装:

```shell
make init
make install ARGS="-DCMAKE_INSTALL_PREFIX=$HOME/Applications/Picker" 
```

之后配置好环境变量:

```shell
export PATH="$HOME/Applications/Picker:$PATH"
```

最后，执行 `picker --check` 检查是否完成安装。
