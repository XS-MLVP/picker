# 导出 RTL 工程

导出时需要确定源文件、顶层模块、目标语言和模拟器。下面的命令在 Picker 仓库根目录运行；接入自己的工程时，替换文件路径和模块名即可。

## 导出单个文件

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --sim verilator --tdir build/Adder --autobuild false
make -C build/Adder -j4
```

`--sname` 选择 RTL 中的顶层模块。`--tdir` 指定生成目录，`--autobuild false` 让生成和编译分两步执行。保留默认的自动构建时，Picker 会在生成完成后直接编译。

如果希望 Python 包使用另一个名字，例如 `Arithmetic`，添加 `--tname Arithmetic`，并将输出目录改成 `build/Arithmetic`。对应的类名为 `DUTArithmetic`。

## 使用文件列表

工程包含多个文件时，通过 `--fs` 提供文件列表。仓库的 `example/MultiFileRTL/design.f` 包含头文件目录和多个源码文件：

```text
+incdir+inc
inc/defs.vh
inc/outer.svh
leaf.v
top.sv
```

这些路径相对于文件列表所在目录。在仓库根目录执行：

```sh
picker export --fs example/MultiFileRTL/design.f \
    --sname MultiFileRTLTop --lang python --sim verilator \
    --tdir build/MultiFileRTLTop --autobuild false
make -C build/MultiFileRTLTop -j4
```

文件列表支持嵌套 `-f` / `-F`、`+incdir+` 和 `+define+`。将工程使用的包含目录和宏定义写入列表，让解析和模拟器编译使用同一组输入。

## 传入编译选项

| 选项 | 作用阶段 | 常见用途 |
| --- | --- | --- |
| `--rtl-param` | RTL elaboration | 设计参数；求值结果固化到生成的外壳中 |
| `--rtl-define` | RTL 与模拟器 | RTL 使用的预处理宏 |
| `-S` | slang 解析 | 解析器选项 |
| `-V` | 模拟器编译 | 模拟器的诊断或优化选项 |
| `-C` | C++ 编译 | 编译器选项 |

含空格的参数需要加引号。例如，向 Verilator 传入警告选项：

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --tdir build/AdderDebug -V '-Wno-fatal' --autobuild false
```

Picker 会生成 `<module>_top.sv` 外壳并在其中实例化设计，因此模拟器看到的顶层是外壳。
这正是设计参数需要专用选项的原因：picker 用 RTL 前端求值 `--rtl-param`，再把得到的常量
写入实例化语句。

```sh
picker export t.sv --sname t --tdir out \
    --rtl-param A=8 --rtl-param C=2 --rtl-define DATA_WIDTH=16
```

```verilog
 t #(
    .A(8),
    .B(4),
    .C(2)
 ) t(
```

`--rtl-define` 以 `-D` 形式传给 RTL 前端，以 `+define+` 形式传给模拟器，并优先于
filelist 中的同名宏。

## 将多个模块组合导出

`--sname` 可以指定模块及实例数。假设 `rtl/a.v` 定义 `A`，`rtl/b.v` 定义 `B`，以下命令生成含两个 A 和三个 B 的组合模块 C：

```sh
picker export rtl/a.v,rtl/b.v --sname A,2,B,3 \
    --tname C --lang python --tdir build/C
```

生成后，查看接口文件中的实例和端口名称，再编写对应测试。

<a id="unpacked-array-ports"></a>

## 导出 unpacked 数组端口

Verilator 后端的定长 unpacked 数组端口支持 5.026 及更新版本；回归锚点为 5.026、5.048 和 5.052。
见[版本兼容说明](../reference/compatibility.md#unpacked-array-versions)。

支持元素为标量 `bit`/`logic` 或一维 packed `bit`/`logic` 位向量的定长 unpacked 输入/输出数组。生成接口保留数组结构，每个元素仍是普通的 `XData`：

```systemverilog
input logic [7:0] data [0:3];
input logic [7:0] matrix [-1:1][2:3];
```

对于使用这些端口生成的 Python DUT，使用 RTL 原始下标：

```python
dut.data[0].value = 0x12
dut.matrix[-1][2].value = 0x34
for i in dut.data.indices:
    dut.data[i].value = i
dut.Step(1)
```

Python 数组视图提供 `bounds`（如 `((-1, 1), (2, 3))`）、`shape`、`indices`、`len()`，并支持按声明顺序遍历元素。负下标表示 RTL 下标。每层视图保留该维声明的方向；例如 `data [3:2]` 的 `indices` 为 `[3, 2]`，迭代从左侧边界开始。越界抛出 `IndexError`；通过元素的 `.value` 赋值。`dut["data"]` 返回同一视图，`dut["matrix[-1][2]"]` 与 `dut.matrix[-1][2]` 指向同一绑定信号。

| 语言 | 元素访问 | 容器 |
| --- | --- | --- |
| Python | `dut.matrix[-1][2]` | `XArray`，叶子为绑定的 `XData` |
| C++ | `dut.matrix->Array(-1)->Data(2)` | `shared_ptr<XArray>`，叶子共享所有权 |
| Java | `dut.matrix.Array(-1).Data(2)` | `XArray` |
| Scala | `dut.matrix.Array(-1).Data(2)` | `XArray` |
| Go | `dut.Matrix.Array(-1).Data(2)` | `xspcomm.XArray` |
| Lua | `dut.matrix:Array(-1):Data(2)` | `XArray` |

C++ 视图对超出声明边界的下标抛出 `std::out_of_range`。

各语言统一使用递归 `XArray` 和经过检查的 RTL 下标。
`Left()`、`Right()` 和 `Size()` 描述当前维度；`LeftBounds()`、`RightBounds()` 和
`Shape()` 描述连续数组维度。配置库的 `ports` 元数据使用 `Unpacked: [[left, right], ...]`
按声明顺序记录各维边界。多实例导出沿用原有模块/实例前缀，六种绑定使用同一套数组元素绑定路径。

Verilator `--rw mem_direct` 为每个数组只登记一个基地址和由 `sizeof` 得到的各维字节步长。
xcomm 在绑定叶子时根据 RTL 各维方向计算元素地址。内部数组使用
[原生布局路径](internal-signals.md#native-arrays)。

`--rw dpi` 为每个输入数组只生成**一个带下标的 getter 和一个 setter**，输出数组只有
getter。这些函数直接访问原 SV 数组。内部扁平下标按声明顺序排列，
最后一维变化最快；生成的 SV 代码将其换算为 RTL 下标。两种访问模式均使用 xcomm 中共享
的内部 C++ 数组后端。每个叶子保存后端引用及 DPI 下标或已计算的原生地址。
对外访问使用原 RTL 边界。

Toffee 的 `SignalArray` 支持直接写
`Bundle(matrix=dut.matrix)`，或将元数据绑定入口适配到配置库的 `ports` 记录。
Toffee 保留同一批叶子和原始下标，数组快照保留边界。详见
[Toffee 数组指南](https://github.com/XS-MLVP/toffee/blob/main/docs/zh/guides/data.md)。

数组视图组织已绑定 `XData` 的引用。DPI 后端每次通过带下标的访问器读写一个元素，
mem_direct 则直接访问该元素的原生存储。生成的端口视图在初始化时创建叶子并注册到
`XPort`；独立的 `XArray.FromDPI` 和 `XArray.FromNative` 工厂通过同一递归接口按需创建、
缓存叶子。共享绑定和扁平下标属于内部实现。

新接口替代原 `XDataArray` 类型和语言专用端口容器。将完整坐标或扁平
选择迁移为每层调用 `Array(index)`、叶子调用 `Data(index)`，Python 也可直接索引。
使用匹配的 Picker/xcomm SDK 重新生成 DUT；当前原生 ABI 为 6。

<a id="struct-packed"></a>

## Struct 与多维 packed 数组

Picker 从展开后的 RTL 类型自动生成递归 xcomm 布局。
这些聚合端口视图要求 **Verilator 5.048 及更新版本**。

| RTL 类型 | 真实绑定的值 | 生成接口 |
| --- | --- | --- |
| packed struct | 一个完整 `XData` | 原始信号，显式调用 `StructView()` |
| 多维 packed array | 一个完整 `XData` | 原始信号，显式调用 `ArrayView()` |
| unpacked array | 独立元素值 | 包含 `XData` 或嵌套容器的数组视图 |
| unpacked struct | 独立字段值 | `XStruct` |

packed 端口默认只生成一个完整 `XData`。初始化时从同一 `.xscfg` 中关联类型，用户显式
调用 `StructView()` 或 `ArrayView()`。unpacked struct/array 直接使用端口容器。
struct 与 array 可以递归嵌套：

```python
dut.packed_in.StructView().Data("tag").value = 29
dut.packed_in.StructView().Data("valid").value = 1
dut.matrix.ArrayView()[1][-2].value = 1
dut.record_in.entries[-1].header.StructView().Data("tag").value = 17
dut.record_in.entries[-1].payload[3].value = (1 << 64) + 0x1234
dut.Step(1)
```

struct 的 `Data(name)` 返回完整字段 `XData`，`Struct(name)` 和 `Array(name)` 返回
嵌套容器。Python 也支持 `view[name]` 和字段属性；packed struct/array 字段及 unpacked
数组中的 packed 元素默认返回 `XData`，再显式调用其 `StructView()` / `ArrayView()`。
也可直接显式调用容器的 `Struct(name)`、`Array(name)`。与方法重名的字段仍可通过 `view[name]`
访问。packed 数组保留各维 RTL 边界与方向，声明左端的元素位于最高有效位。
Python 的 `bounds`、`shape`、`indices` 与 unpacked 数组遵循相同规则；标量叶子使用
普通 `XData` 的零位宽表示。

`XData` 的普通位索引仍以最低有效位为 0；显式数组视图才使用 RTL 声明的边界。
例如 `logic [1:0][7:4] mixed [3:2]` 中，`mixed[3]` 是完整 8 位 Data，
`mixed[3][1]` 是其位偏移 1，而 `mixed[3].ArrayView()[1][7]` 才是 RTL 的数组选择。
详见
[XData 类型布局与显式视图](../explanation/data-layouts.md)。

packed 字段写入合并到父信号，遵循父信号原有写模式与边沿提交策略。
unpacked 字段独立绑定，数组中的同类字段共享一对索引式 DPI
getter/setter，或一份由 C++ 编译器计算的原生基址与字节步长。原生访问使用实际生成的
C++ 成员。配置库 `ports` 记录的 `Layout` 描述递归类型，
包括字段名、packed 位偏移、边界、位宽、有符号性和二态/四态类型。
同一文件的 `port_types` 将精确逻辑端口名映射到去重类型表；xcfg 为已有 Data 关联类型。
类型与按需视图共享，字段或元素 Data 保留子类型。
DPI 与 mem_direct 均直接访问选中的独立字段或数组元素。

运行 `make -C test export_aggregate_views` 执行基于 `test/data/aggregate_views.sv` 的
原有 Python/C++/Java RTL 回归。此次无参显式视图在 Verilator 5.052 上完成六语言聚合
fixture 的 DPI、mem_direct、VPI 回归（Scala 使用 2.13.16）。Picker 须与包含无参显式视图、原生 ABI 6 的匹配
xcomm SDK 配套使用，并用该 SDK 重新编译生成的 DUT。

## 生成成员名

端口叶子、unpacked 容器和层次子端口统一分配成员名。普通标识符保持原拼写（Go 首字母大写），
封装成员、方法及目标语言关键字受到保护；冲突端口添加 `pin_` 前缀，如果改名后仍有
冲突，则添加数字后缀。例如同时存在 `vpi_backend` 和 `pin_vpi_backend` 两个端口时，
Python 将前者暴露为 `pin_vpi_backend_2`，后者使用原名。

这里只修改封装属性名。RTL 路径、`XPort` 键和配置库端口元数据仍使用逻辑原名。
消费元数据时使用 `dut["vpi_backend"]` 或对应语言的 `XPort` 查询。
Verilator 的原生标识符转义与这些宿主语言成员名相互独立。

所有语言绑定都对 packed 和向量 `inout` 端口生成各自的 `InOut` 方向（C++ 为
`IOType::InOut`，Go 为 `IOType_InOut`，其余为 `XData.InOut`）。
方向元数据使信号在 DUT 驱动后可读。

## 添加调试功能

- 使用 `-w trace.fst` 生成波形支持，具体操作见[记录仿真结果](simulation.md#recording)。
- 使用 `--coverage` 收集模拟器代码覆盖率。
- 使用 `--vpi` 或 `--internal` 访问内部信号，见[内部信号指南](internal-signals.md)。
- 使用 `--lang` 切换语言，见[多语言示例](languages.md)。

完整选项、默认值和输出目录规则见[命令行参考](../reference/cli.md)。
