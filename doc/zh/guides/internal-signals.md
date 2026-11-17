# 访问内部信号

调试电路时，可以通过层次名称读取内部寄存器或修改信号。本页先用仓库中的 `vpi` 示例演示查询，再介绍其他访问方式。

## 导出带 VPI 的模型

在仓库根目录运行：

```sh
picker export example/InternalSignals/vpi.v --sname vpi --lang python \
    --sim verilator --vpi --tdir build/docs-internal/vpi --autobuild false
make -C build/docs-internal/vpi -j4
```

`--vpi` 在模型中启用运行时信号查询。这个设计有一个时钟 `clk`，以及控制计数步长的内部信号 `_v2_base`。

## 查询并修改信号

将以下代码保存为 `build/docs-internal/inspect.py`：

<!-- runnable:python -->
```python
from vpi import DUTvpi

dut = DUTvpi()
try:
    dut.InitClock("clk")
    dut.Step(1)
    for name in dut.VPIInternalSignalList():
        print(name)
    increment = dut.GetInternalSignal("vpi._v2_base", use_vpi=True)
    assert increment is not None
    increment.AsImmWrite()
    increment.value = 9
    dut.Step(1)
    before = dut.v2.value
    dut.Step(1)
    assert dut.v2.value == (before + 10) & 0xFF
    print("v2 increment: 10")
finally:
    dut.Finish()
```

运行：

```sh
cd build/docs-internal
python3 inspect.py
```

`Step(1)` 先执行电路初始化，`AsImmWrite()` 让内部信号赋值立即提交。输出列出可访问的层次名称，最后应包含 `v2 increment: 10`。RTL 每周期加上 `1 + _v2_base`，所以写入 9 后，每周期增加 10。

接入自己的设计时，先用 `VPIInternalSignalList()` 查询名称，再将实际层次路径传给 `GetInternalSignal()`。可以用 `prefix` 和 `deep` 缩小查询范围。

## 选择访问方式

| 方式 | 配置 | 适用情况 |
| --- | --- | --- |
| 指定内部信号导出 | `--internal 配置文件` | 已知需要观察的信号，随端口一起生成访问接口 |
| VPI 查询 | `--vpi` | 调试时按层次名称枚举和访问 |
| 原生内存访问 | `--rw mem_direct` | Verilator/GSIM 的直接读写路径，需要对应模型的布局信息 |

`GetInternalSignal()` 默认优先使用模型生成的原生布局信息，否则通过 VPI 查询。`use_vpi=True` 明确选择 VPI。仓库 `example/CacheSignalCFG` 展示了原生访问的完整工程。

`GetInternalSignal(..., use_vpi=True)` 对普通 VPI 叶子返回 `XData`。自动发现时，只有一维
packed bit-vector（包括单比特向量）作为一个 `XData`；此类向量组成的 unpacked 数组
作为 `XArray`，其元素为 `XData`。Python 中 packed 聚合也默认返回 `XData`，可显式调用
其 `StructView()` / `ArrayView()`。unpacked 容器返回 `XStruct`/`XArray`。
递归聚合也可在六语言中显式使用 `GetInternalSignalView(name)`。Picker 为 packed
聚合端口提供 RTL 布局，unpacked 数组与 struct 成员则解析为经过检查的 VPI 句柄。
packed 聚合和 unpacked 数组视图要求 Verilator 5.048 或更新版本，unpacked struct 的
VPI 视图要求 Verilator 5.052 或更新版本。内部 unpacked struct 可通过模拟器提供的
完整 VPI 类型与成员信息发现。

## RTL 类型与原生存储视图

Verilator 的 `--rw mem_direct` 导出会复用本次 Slang 展开的内部类型，与生成的 C++
存储逐项核对。`GetInternalSignalView(name, use_vpi=False)` 始终存在：默认在有原生
配置时使用原生视图，否则使用编入的 VPI；`use_vpi=True` 强制选择 VPI。

例如 RTL 中有以下内部声明：

```systemverilog
typedef struct packed { logic [7:0] tag; logic [23:0] data; } packet_t;
packet_t packet;
logic [1:0][31:0] lanes;
logic [64:0] memory [3:2][-1:1];
```

将下面的路径替换为 `GetInternalSignalList()` 返回的实际完整名称：

```python
packet = dut.GetInternalSignalView("Top.packet").Struct()
packet.Data("tag").value = 0x12  # 只改共享父值的高 8 位
lanes = dut.GetInternalSignalView("Top.lanes").Array()
lanes.Data(0).value = 0x12345678
memory = dut.GetInternalSignalView("Top.memory").Array()
memory.Array(3).Data(-1).value = (1 << 64) | 0x55
```

这里的数组使用 **RTL 坐标**，普通一维 bit-vector 是 `XData` 叶子；原来的
`GetInternalSignal(..., is_array=True)` 仍使用零基**存储坐标**。原生视图的动态叶子
默认立即写入；调用 `Step()` 或 `RefreshComb()` 后计算相应电路逻辑。
完整 packed 值仍可用原有 `GetInternalSignal(name)` 或视图的 `PackedData()` 读写。

原生 `GetInternalSignal(name)` 返回的 Data 会关联可用的类型；例如先取得 packet Data，
再调用 `StructView()`。视图共享这个 Data 的后端和原有写入模式。Data 位索引以最低
有效位为 0，显式数组视图使用 RTL 下标。

固定 unpacked struct 及其数组也可生成视图：字段必须能在模型的真实 C++ struct 中
逐一匹配，地址差和 `sizeof` 决定 padding、大小和数组步长。packed 字段只对完整
父值切片；unpacked 字段绑定独立存储。

每个导出工程只交付一份 `<module>.xscfg` 二进制配置库，统一保存端口元数据、去重类型、
原生存储、字段绑定及派生信号。Java/Scala 把同一份配置库打入 JAR，运行时每个类提取
一次临时资源。

构建阶段的候选类型也位于同一个文件，原生地址生成完成后去除候选记录，仅保留成功绑定
以及端口类型引用可达的类型。普通值和同构数组复用 offset/stride，unpacked struct 按同构字段组记录
`bindings`。

配置库采用路径前缀共享、紧凑整数、内置只读查询索引及独立压缩块。POSIX 上直接映射
文件，首次查询按需解压数据块，解压块缓存最多 2 MiB。
列出全部名称仍需遍历数据块；`String()` 是显式的全量调试操作。文件使用独立格式版本，
当前原生 ABI 为 6；运行时核对模型标识、文件边界和访问块的校验和。

可显式查看配置，命令向标准输出打印 JSON：

```sh
picker config Top.xscfg
picker config Top.xscfg --section metadata --name ports
picker config Top.xscfg --section variables --name Top.packet
```

Python 也可用 `json.loads(dut.xcfg.Metadata("ports"))` 获取端口元数据对象，第三方工具可
通过该入口适配配置库。

<a id="native-arrays"></a>

## 访问原生数组

在 Verilator 上，原生数组访问使用 `--rw mem_direct` 布局路径，版本范围与数组端口相同，
为 5.026 及更新版本；回归锚点为 5.026、5.048 和 5.052。见[版本兼容说明](../reference/compatibility.md#unpacked-array-versions)。

在提供原生布局的模型中，可以一次取得定长数组，再访问元素：

```python
memory = dut.GetInternalSignal("Top.memory", is_array=True)
print(memory[0][0].value)  # 以二维数组为例
memory[1][0].value = 42
```

将 `Top.memory` 替换为 `GetInternalSignalList()` 列出的实际名称。可写内部数组需要由 Verilator 保留，例如在声明后加 `/*verilator public_flat_rw*/`。`AsImmWrite()` 模式下写入会立即修改模型存储，再调用 `Step()` 或 `RefreshComb()` 计算读取该存储的电路逻辑。

`is_array=True` 返回递归 `XArray`，下标使用从零开始的**存储坐标**。
例如 `logic [64:0] memory [2:3][-1:1]` 的
`memory.shape` 为 `(2, 3)`，`memory[row][column]` 对应 RTL 的
`memory[row + 2][column - 1]`。根节点的 `Size()` 为 2，每行的 `Size()` 为 3。
单元素接口 `dut.GetInternalSignal(name, index=position)` 仍接受经过检查的扁平存储位置，
最后一维变化最快。

布局生成器记录各维尺寸和由 `sizeof` 得出的实际步长。例如 Verilator 的 65 位元素占 12 字节。存在 trace 元数据时，还可能列出 `Top.memory[2][-1]` 这样的 RTL 下标名称；直接元素选择绑定原生偏移。应以模型实际列出的名称为准。

xcomm 通过一份内部原生后端消费维度和步长。`dut.xcfg.NewXArray(name)` 返回同一缓存
递归视图，`Array(index)` 选择子数组、`Data(index)` 选择叶子，按需创建节点。
`LeftBounds()/RightBounds()/Shape()` 描述存储坐标，需要布局元数据时可查询
`cfg = dut.xcfg.At(name)` 并保留该配置对象，再读取 `cfg.array_strides`。

内部信号对象在 DUT 存活期间使用，重新创建模型后重新查询。
