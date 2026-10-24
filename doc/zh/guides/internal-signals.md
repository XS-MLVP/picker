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

## 访问原生数组

在提供原生布局的模型中，可以一次取得数组，再访问元素：

```python
memory = dut.GetInternalSignal("Top.memory", is_array=True)
print(memory[0].value)
memory[1].value = 42
```

将 `Top.memory` 替换为实际名称。查询结果按名称缓存，因此同一名称应保持一致的数组查询方式。`index`、`is_array` 由原生访问路径提供。内部信号对象在 DUT 存活期间使用，重新创建模型后重新查询。
