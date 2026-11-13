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

定长 unpacked 数组的 **DPI 与 mem_direct** 路径支持 **Verilator 5.026 及更新版本**。回归锚点为
5.026、5.048、5.052；其他功能的通用构建最低版本为 5.020。

| 数组访问路径 | RTL 回归覆盖 | 已验证的 Verilator 版本 |
| --- | --- | --- |
| DPI 端口 | Python、C++、Java | 5.026、5.048、5.052 |
| mem_direct 端口 | Python、C++、Java | 5.026、5.048、5.052 |
| mem_direct 内部数组 | Python、C++ | 5.026、5.048、5.052 |

设置 `ARRAY_EXTRA_LANGUAGES=1` 可运行额外的 Go/Lua 端口数组 RTL 回归；
Scala 数组运行时测试尚未验证。

六种语言生成器使用同一套共享数组绑定实现，公开数组统一使用 xcomm 的递归 `XArray` API。
Picker 与 xcomm 使用匹配的工具链；升级后重新生成并编译 DUT 和数组布局 YAML。

原生元素地址及步长由 C++ 编译器根据生成模型计算，不按 Verilator 版本硬编码。
更换 Verilator 或模型构建选项时，需要重新生成并一起编译访问代码、模型和布局元数据。

生成的辅助代码格式可以独立于存储布局发生变化。Picker 根据实际生成格式选择 trace
解析器，允许常量数组选择中的空白；DPI 只有在声明的 C 符号不存在时，才尝试旧版下划线
编码名称。不支持的 trace 格式应明确报错，不能当成有效原生布局。端口和内部数组的下标
接口不同，见[数组端口访问](../guides/export.md#unpacked-array-ports)和
[内部原生数组](../guides/internal-signals.md#native-arrays)。

### Struct 与 packed 数组端口视图

packed/unpacked struct 与多维 packed 数组使用[聚合类型导出](../guides/export.md#struct-packed)
中介绍的递归 xcomm 容器。packed 对象绑定一个完整 `XData`，unpacked 对象绑定独立
字段或元素。布局从 RTL 自动派生，原生成员地址与步长由生成的 C++ 模型计算。
这些功能要求 **Verilator 5.048 及更新版本**，生成的 CMake 配置会对更旧版本报错。
上文普通 unpacked 数组仍保留 5.026 的最低版本。

| 访问路径 | RTL 回归语言 | 已验证的 Verilator 版本 |
| --- | --- | --- |
| DPI 与 mem_direct | Python、C++、Java | 5.048、5.052 |
| DPI 与 mem_direct | Go、Lua | 5.048、5.052 |
| VPI packed 聚合与 unpacked 数组 | Python、C++、Java、Go、Lua | 5.048、5.052 |
| VPI 递归访问 unpacked struct | Python、C++、Java、Go、Lua | 5.052 |

测试覆盖嵌套 struct、struct 数组、混合方向边界、65 位字段及原始 packed 整体值。
Python 还覆盖 packed struct 数组、多维 packed/unpacked 组合、多实例、enum 与整数
字段。VPI 视图使用 VPI 句柄和 Picker 从 RTL 得到的类型布局，不会推测模拟器内存偏移。
Verilator 5.048 可访问已验证的 packed 聚合和 unpacked 数组，但没有足够的 VPI 类型/成员
信息来递归访问 unpacked struct；请求会明确报出无法访问的成员路径。访问 unpacked struct
需要 Verilator 5.052 或更新版本。VPI 结果已在 Python、C++、Java、Go 和 Lua 绑定上验证；
Scala 的聚合 RTL 运行时行为（DPI、mem_direct、VPI）尚未验证。VPI 回归使用聚合端口 fixture；
Picker 当前只为端口生成 packed 类型布局，不为内部 packed struct 生成布局，因此会明确
拒绝对内部 packed struct 的请求，而不会猜测字段偏移。这些结果不代表 VCS、UVS、GSIM
上的聚合 RTL 支持。

使用原生 ABI 4 的匹配 xcomm SDK，并重新编译生成的 DUT。DPI setter 与 mem_direct
写入均直接访问所选独立字段或数组元素，不复制整个 unpacked 数组。

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
