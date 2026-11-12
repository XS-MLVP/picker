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

VPI 查询返回单个 `XData` 信号。原生布局查询还通过下文的 `is_array` 和 `index` 参数
提供数组访问。

<a id="native-arrays"></a>

## 访问原生数组

在 Verilator 上，原生数组访问使用 `--rw mem_direct` 布局路径，版本范围与数组端口相同，
为 5.026 及更新版本；回归锚点为 5.026、5.048 和 5.052。见[版本兼容说明](../reference/compatibility.md#unpacked-array-versions)。

在提供原生布局的模型中，可以一次取得定长数组，再访问元素：

```python
memory = dut.GetInternalSignal("Top.memory", is_array=True)
print(memory[0].value)
memory[1].value = 42
```

将 `Top.memory` 替换为 `GetInternalSignalList()` 列出的实际名称。可写内部数组需要由 Verilator 保留，例如在声明后加 `/*verilator public_flat_rw*/`。不需要 VPI 或 SV 桥接信号。`AsImmWrite()` 模式下写入会立即修改模型存储，再调用 `Step()` 或 `RefreshComb()` 计算读取该存储的电路逻辑。

`is_array=True` 返回按**原生存储顺序**排列的扁平列表，最后一维变化最快。这里的下标是从零开始的存储位置，不是导出端口视图使用的 RTL 边界。例如 `logic [64:0] memory [2:3][-1:1]` 对应六个元素，位置 `row * 3 + column` 对应 RTL 的 `memory[row + 2][column - 1]`。只需访问一个元素时，使用 `dut.GetInternalSignal(name, index=position)`。Python 和 C++ 会在绑定前检查扁平下标，并按各个下标分别缓存。

布局生成器记录各维尺寸和由 `sizeof` 得出的实际步长，在提供扁平接口前检查连续存储。例如 Verilator 的 65 位元素占 12 字节，不是 9 字节。存在 trace 元数据时，还可能列出 `Top.memory[2][-1]` 这样的 RTL 下标名称；直接元素选择会绑定原生偏移，而非表达式后端。应以模型实际列出的名称为准。计算表达式仍是只读派生信号，不能当成可写原生存储。

xcomm 通过共享的原生 `XDataArray` 后端消费维度和步长。
`dut.xcfg.NewXDataArrayView(name)` 返回提供 `Shape()`、`Strides()`、`At(indices)` 和
`AtFlat(index)` 的视图，按需创建叶子。这些内部视图使用从零开始的**存储坐标**，
不是恢复出的 RTL 原始边界。扁平列表及下标查询也走同一后端，不安装逐元素回调。
共享后端只持有元数据，不持有模型内存或 DPI 动态库的生命周期。

内部信号对象在 DUT 存活期间使用，重新创建模型后重新查询。
