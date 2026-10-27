# Python DUT API

导出的 Python 包提供 `DUT<模块名>` 类。以下签名来自 Python 模板；其他语言见[多语言指南](../guides/languages.md)。波形、覆盖率和检查点的支持情况见[模拟器支持](compatibility.md)。

## 创建对象与访问端口

```python
from Adder import DUTAdder

dut = DUTAdder()
dut.a.value = 42
print(dut.a.value)
print(dut["a"].value)
dut.Finish()
```

`DUT<模块名>(*args, **kwargs)` 创建模型。位置参数传给模拟器；关键字参数 `waveform_filename` 和 `coverage_filename` 设置输出路径，需要在导出时启用对应记录功能。

`dut.pin` 和 `dut["pin"]` 返回同一个端口对象。通过 `.value` 或 `Set(value)` 写入，`.value` 读取完整位宽的无符号 Python 整数。端口属性本身只读。

## 时钟与执行

| 方法 | 行为与返回值 |
| --- | --- |
| `InitClock(name)` | 将指定端口登记为时钟引脚；返回 `None` |
| `Step(i=1)` | 推进 i 个完整周期并刷新信号；返回 `None` |
| `StepRis(callback, args=(), kwargs={})` | 注册上升沿回调，调用形式为 `callback(cycle, *args, **kwargs)` |
| `StepFal(callback, args=(), kwargs={})` | 注册下降沿回调，参数形式同上 |
| `RefreshComb()` | 调用组合求值，保持周期计数；输入应已提交给模型 |
| `Finish()` | 结束仿真、处理输出；使用完成后调用 |
| `AtClone()` | 在 fork 的子进程中重新初始化模拟器状态 |

回调在相应边沿的求值和信号刷新后执行。复位通过 RTL 复位端口驱动，示例见[设置时钟和复位](../guides/simulation.md)。

测试框架取得时钟控制权期间，`Step()` 会报告错误。此时通过框架的执行方法推进仿真。新生成的 VCS 包在子进程中完成结束流程，宿主进程可在 `Finish()` 后继续生成报告；旧包须重新生成并编译。UVS 的结束流程仍可能终止进程，其测试汇总应安排在 `Finish()` 前。

## 波形

| 方法 | 行为与返回值 |
| --- | --- |
| `SetWaveform(filename)` | 设置波形文件路径，格式与导出时的配置一致 |
| `GetWaveFormat()` | 返回扩展名，如 `fst`；未启用时返回空字符串 |
| `PauseWaveformDump()` | 暂停波形记录 |
| `ResumeWaveformDump()` | 恢复波形记录 |
| `WaveformPaused()` | 返回 1 表示暂停，0 表示记录中 |
| `FlushWaveform()` | 将波形缓冲写入文件 |

导出选项及完整脚本见[保存波形](../guides/simulation.md#recording)。频繁刷新会增加 I/O，也会影响 FST 压缩效率。

## 代码覆盖率与检查点

| 方法 | 行为与返回值 |
| --- | --- |
| `SetCoverage(filename)` | 设置模拟器覆盖率输出位置 |
| `ResetCoverage()` | 重置覆盖率计数；当前由 VCS 路径实现 |
| `GetCovMetrics()` | 返回启用的指标位掩码，0 表示关闭 |
| `CheckPoint(name)` | 将模拟器状态保存到文件；需要 `--checkpoints`，Python 返回 `None` |
| `Restore(name)` | 从检查点恢复模拟器状态；Python 返回 `None` |

`GetCovMetrics()` 中的位 0–5 分别表示 line、cond、fsm、toggle、branch、assert。

## 内部信号

| 方法 | 行为与返回值 |
| --- | --- |
| `GetInternalSignal(name, index=-1, is_array=False, use_vpi=False)` | 按名称返回信号对象；`is_array=True` 返回列表 |
| `GetInternalSignalList(prefix="", deep=99, use_vpi=False)` | 返回可访问名称；默认优先原生布局，随后 VPI |
| `VPIInternalSignalList(prefix="", deep=99)` | 通过 VPI 枚举层次名称 |

原生查询中的 `prefix` 按名称子串筛选。VPI 中的 `prefix` 和 `deep` 指定起始层次与搜索深度。`index`、`is_array` 用于原生布局路径。

查询结果按 `name` 缓存，后续调用复用首次查询的对象。普通查询未找到时返回 `None`；显式 `use_vpi=True` 查询失败会触发断言。具体示例见[访问内部信号](../guides/internal-signals.md)。

## Python 异步方法

| 方法 | 行为 |
| --- | --- |
| `await dut.AStep(n)` | 等待仿真推进 n 个周期 |
| `await dut.ACondition(fn)` | 在周期通知时检查 fn，条件成立后返回 |
| `await dut.RunStep(n)` | 推进 n 个周期，并在周期之间让其他协程执行 |

等待协程需要与推进仿真的协程一起运行。`AStep()` 和 `ACondition()` 使用时钟的 Python 周期通知。

## 框架集成

`GetXPort()` 返回 DUT 的端口集合 `XPort`，`GetXClock()` 返回时钟对象 `XClock`。框架可通过这两个对象接入信号和时钟管理。它们及内部信号对象在 DUT 存活期间使用。
