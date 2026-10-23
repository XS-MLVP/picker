# 模拟器与语言支持

## 模拟器

`picker export --sim` 接受以下选项：

| 选项 | 工具环境 | 说明 |
| --- | --- | --- |
| `verilator` | Verilator 与 C++ 编译器 | 默认选项；本文教程使用此环境 |
| `vcs` | VCS 及其许可证；FSDB 需要 Verdi | 提供 DPI/VPI、波形和代码覆盖率适配 |
| `gsim` | GSIM 配套工具链 | 提供导出、仿真推进和原生信号访问适配 |
| `uvs` | UVS 配套工具链及许可证 | 提供仿真推进和 USDB 波形适配 |

波形和检查点需在导出时启用：

| 功能 | 导出选项 | 支持情况 |
| --- | --- | --- |
| FST / VCD 波形 | `-w trace.fst` / `-w trace.vcd` | Verilator |
| FSDB 波形 | `-w trace.fsdb` | VCS + Verdi |
| USDB 波形 | `-w trace.usdb` | UVS |
| 代码覆盖率 | `--coverage` | Verilator、VCS；具体指标由编译配置决定 |
| 检查点 | `--checkpoints` | Verilator |
| 原生内存读写 | `--rw mem_direct` | Verilator、GSIM |

## 语言

| `--lang` | 运行或编译环境 |
| --- | --- |
| `python` | Python 及匹配的开发头文件 |
| `cpp` | C++20 编译器 |
| `java` | JDK |
| `scala` | JDK、Scala |
| `golang` | Go 和 C++ 编译器 |
| `lua` | Lua 及开发头文件 |

生成时需要已安装对应语言的绑定资源。检查方法和示例命令见[多语言指南](../guides/languages.md)。

## 平台与分发

Linux 是主要构建环境。macOS 可使用 Verilator 和源码构建，动态多实例受平台加载机制限制。Python wheel 按 Python 小版本、操作系统和架构区分，例如 `cp312` 表示 CPython 3.12。

生成的模型与导出时的工具链和运行库一起使用。更换这些依赖后，重新生成并编译模型。分发前在目标环境运行[加法器教程](../tutorials/first-dut.md)中的短测试，可以检查导入、宽信号和仿真推进。
