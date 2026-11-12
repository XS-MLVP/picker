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
| `-S` | slang 解析 | 解析器选项、顶层参数配置 |
| `-V` | 模拟器编译 | 模拟器的诊断或优化选项 |
| `-C` | C++ 编译 | 编译器选项 |

含空格的参数需要加引号。例如，向 Verilator 传入警告选项：

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --tdir build/AdderDebug -V '-Wno-fatal' --autobuild false
```

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

Python 数组视图提供 `bounds`（如 `((-1, 1), (2, 3))`）、`shape`、`indices`、`len()`，并支持按声明顺序遍历元素。负下标表示 RTL 下标，不表示倒数位置。每层视图保留该维声明的方向；例如 `data [3:2]` 的 `indices` 为 `[3, 2]`，迭代从左侧边界开始。越界抛出 `IndexError`；数组结构只读，不要替换视图或元素，应给元素的 `.value` 赋值。`dut["data"]` 返回同一视图，`dut["matrix[-1][2]"]` 与 `dut.matrix[-1][2]` 指向同一绑定信号。

| 语言 | 元素访问 | 容器 |
| --- | --- | --- |
| Python | `dut.matrix[-1][2]` | 保留边界的只读视图 |
| C++ | `dut.matrix[-1][2]` | 检查边界、返回 `XData&` 的视图 |
| Java | `dut.matrix.get(-1).get(2)` | 只读嵌套 `Map` |
| Scala | `dut.matrix(-1)(2)` | 不可变嵌套 `Map` |
| Go | `dut.Matrix[-1][2]` | 嵌套 `map[int]` |
| Lua | `dut.matrix[-1][2]` | 嵌套 table |

C++ 视图对超出声明边界的下标抛出 `std::out_of_range`。

各语言均使用 RTL 原始下标。Java、Scala、Go 和 Lua 的 map 容器保留所属语言的缺失键行为。`signals.json` 使用 `Unpacked: [[left, right], ...]` 按声明顺序记录各维边界。多实例导出沿用原有模块/实例前缀。六种绑定使用同一套数组元素绑定路径，因此各语言的元素访问遵循相同的后端规则和 RTL 下标规则。

Verilator `--rw mem_direct` 为每个数组只登记一个基地址和由 `sizeof` 得到的各维字节步长。
xcomm 在绑定叶子时根据 RTL 各维方向计算元素地址，不生成逐元素地址映射、SV 存储、
连续赋值桥接或端口 DPI 访问器。内部数组使用[原生布局路径](internal-signals.md#native-arrays)，不通过端口接口或 VPI 绕行。

`--rw dpi` 为每个输入数组只生成**一个带下标的 getter 和一个 setter**，输出数组只有
getter。这些函数直接访问原 SV 数组，不经过桥接信号。内部扁平下标按声明顺序排列，
最后一维变化最快；生成的 SV 代码将其换算为 RTL 下标。两种访问模式均使用 xcomm 中共享
的 C++ `XDataArray` 后端。每个叶子保存后端引用及 DPI 下标或已计算的原生地址，不保存
逐元素的捕获状态读写回调。对外访问仍使用原 RTL 边界。

Toffee 的 `SignalArray` 支持直接写
`Bundle(matrix=dut.matrix)`，或通过带 `Unpacked` 的 `signals.json` 绑定。Toffee 保留同一
批叶子和原始下标，不改写后端或自动登记其他 `XPort`；数组快照也保留边界。详见
[Toffee 数组指南](https://github.com/XS-MLVP/toffee/blob/main/docs/zh/guides/data.md)。

数组视图组织已绑定 `XData` 的引用。DPI 后端每次通过带下标的访问器读写一个元素，
mem_direct 则直接访问该元素的原生存储。生成的端口视图在初始化时创建叶子并注册到
`XPort`；C++ `XDataArray` 的 `At(indices)`/`AtFlat(index)` 按需创建并缓存叶子。

以下类型不支持：动态数组、队列、关联数组、unpacked `inout` 端口，以及上述标量/位向量以外的元素类型。

## 添加调试功能

- 使用 `-w trace.fst` 生成波形支持，具体操作见[记录仿真结果](simulation.md#recording)。
- 使用 `--coverage` 收集模拟器代码覆盖率。
- 使用 `--vpi` 或 `--internal` 访问内部信号，见[内部信号指南](internal-signals.md)。
- 使用 `--lang` 切换语言，见[多语言示例](languages.md)。

完整选项、默认值和输出目录规则见[命令行参考](../reference/cli.md)。
