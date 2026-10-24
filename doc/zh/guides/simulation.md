# 驱动仿真

本页使用生成的 Python DUT 接口，说明怎样设置时钟、施加复位、采样和记录结果。完整的模型创建示例见[加法器教程](../tutorials/first-dut.md)。

## 设置时钟和复位

对于包含 `clock` 和高有效 `reset` 端口的 DUT，在创建对象后执行：

```python
dut.InitClock("clock")
dut.reset.value = 1
dut.Step(2)
dut.reset.value = 0
dut.Step(1)
```

`InitClock()` 登记时钟引脚，随后每次 `Step(n)` 推进 n 个完整周期。这段代码让复位保持两个周期，然后释放复位并执行一个周期。端口名、复位极性和保持时间应按 RTL 协议设置。

运行测试时，先写输入，再调用 `Step()`，最后读取输出。例如：

```python
dut.valid.value = 1
dut.data.value = 42
dut.Step(1)
print(dut.ready.value)
```

这里假设 DUT 包含 `valid`、`data` 和 `ready` 端口。

## 在边沿采样

用 `StepRis()` 注册上升沿回调，用 `StepFal()` 注册下降沿回调。下面在每次上升沿后读取 `ready`：

```python
def observe(cycle):
    print(cycle, dut.ready.value)

dut.StepRis(observe)
dut.Step(3)
```

回调的第一个参数是周期计数。将采样、记录或断言放在回调中，在测试主流程中调用 `Step()` 推进仿真。

## 只更新组合逻辑

对需要立即生效的输入设置 `AsImmWrite()`，写入后调用 `RefreshComb()`：

```python
dut.a.AsImmWrite()
dut.b.AsImmWrite()
dut.cin.AsImmWrite()
dut.a.value = 2
dut.b.value = 3
dut.cin.value = 1
dut.RefreshComb()
assert dut.sum.value == 6
```

这段代码适用于教程中的 Adder。输入会立即提交，组合逻辑重新求值，周期计数保持不变。

<a id="recording"></a>

## 保存波形和代码覆盖率

导出时启用需要的记录功能：

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --sim verilator --tdir build/docs-trace/Adder --autobuild false \
    -w trace.fst --coverage
make -C build/docs-trace/Adder -j4
```

把下面的代码保存为 `build/docs-trace/record.py`：

<!-- runnable:python -->
```python
from Adder import DUTAdder

dut = DUTAdder(waveform_filename="trace.fst", coverage_filename="coverage.dat")
try:
    dut.b.value = 1
    dut.cin.value = 0
    for value in range(4):
        dut.a.value = value
        dut.Step(1)
        assert dut.sum.value == value + 1
finally:
    dut.Finish()
```

运行脚本并查看生成文件：

```sh
cd build/docs-trace
python3 record.py
ls -lh trace.fst coverage.dat
```

用 GTKWave 等波形查看器打开 `trace.fst`，可以看到 `a` 从 0 到 3 变化，`sum` 对应为 1 到 4。代码覆盖率文件可交给模拟器配套工具处理。

较长的仿真可以通过 `PauseWaveformDump()` 暂停记录，在感兴趣的区间前调用 `ResumeWaveformDump()` 恢复。需要在仿真结束前查看文件时，调用 `FlushWaveform()` 刷新缓冲。`Finish()` 负责结束仿真和输出收尾。

## 保存和恢复检查点

Verilator 工程可以在导出时添加 `--checkpoints`，然后调用：

```python
dut.CheckPoint("state.bin")
dut.Step(10)
dut.Restore("state.bin")
```

检查点保存模拟器状态。Python 测试中的参考模型、队列和随机数状态由测试程序一同保存与恢复。接口返回值及模拟器支持情况见 [DUT API](../reference/dut-api.md)。
