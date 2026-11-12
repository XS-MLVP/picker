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

`--rw mem_direct` 会为生成的模型头构建一个小型地址生成器。如果 Verilator 同时使用
`--timing`，该头需要 C++20 协程；Picker 会自动检测并使用 `-std=c++20 -fcoroutines`。
未启用 `--timing` 时生成器仍使用 C++17，旧编译器继续支持非时序模型。

<a id="unpacked-array-versions"></a>

### Unpacked 数组与 Verilator 版本

定长 unpacked 数组的 **DPI 与 mem_direct** 路径面向 **Verilator 5.026 及更新版本**，回归版本为
5.026、5.048 和 5.052；未对所有中间版本或未来版本逐一认证。通用构建检查仍接受
5.020，但数组功能未验证 5.026 之前的版本。

Picker 与 xcomm 按配套工具链升级。升级后不再支持旧 DUT 和旧布局 YAML，使用当前工具链
重新生成并编译模型产物。本源码树的数组实现直接使用当前 xcomm `XDataArray` API，
不提供旧 SDK 兼容路径。

原生元素地址及步长由 C++ 编译器根据生成模型计算，不按 Verilator 版本硬编码。
更换 Verilator 或模型构建选项时，需要重新生成并一起编译访问代码、模型和布局元数据。

生成的辅助代码格式可以独立于存储布局发生变化。Picker 根据实际生成格式选择 trace
解析器，允许常量数组选择中的空白；DPI 只有在声明的 C 符号不存在时，才尝试旧版下划线
编码名称。不支持的 trace 格式应明确报错，不能当成有效原生布局。端口和内部数组的下标
接口不同，见[数组端口访问](../guides/export.md#unpacked-array-ports)和
[内部原生数组](../guides/internal-signals.md#native-arrays)。

<a id="vpi-array-versions"></a>

### VPI 数组（待实现）

新增 VPI 数组路径的最低版本定为 **Verilator 5.048（含）**；5.026 不纳入支持，
包括其能够暴露句柄的部分一维数组。这不改变已有普通信号的 VPI 查询，也不影响
5.026 上的 DPI 与 mem_direct 数组路径。

独立 VPI 探针已在 5.048 和 5.052 验证一维、二维和三维定长数组的逐维句柄索引及
元素读写，包含升降序、负下标和 65 位元素。5.026 对探针中的二维、三维数组没有注册
VPI 根句柄，因此不为其增加桥接信号或其他兼容路径。这些探针验证的是模拟器能力，
不代表 Picker/xcomm 的 VPI 数组集成已完成，也不代表所有中间版本或未来版本均已验证。

当前共享 `XDataArray` 后端仅实现 DPI 和 mem_direct；`GetInternalSignal()` 的
`is_array`、`index` 仍只适用于原生布局路径。后续 VPI 数组路径应独立查询元素句柄，
不按内存地址推算句柄，也不在明确选择 VPI 时静默切换到 mem_direct。缺少数组句柄或
索引能力时应明确报错，而不是返回伪造的数组视图。

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
