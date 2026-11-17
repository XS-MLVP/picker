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

packed struct 和多维 packed 数组在端口原名下保留完整 `XData`，通过 `StructView()` /
`ArrayView()` 显式访问结构。unpacked struct 直接使用 `XStruct`，数组与 struct 字段
递归组合。例如 `dut.record.entries[-1].flag`
仍是普通 `XData`。`Data(name)`、`Struct(name)`、`Array(name)` 选择字段接口。
四种存储模型、原始 RTL 下标与示例见[聚合类型导出](../guides/export.md#struct-packed)。

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

新生成的 Python、C++、Java、Scala、Go 和 Lua 包将 `RefreshComb()` 统一交给
`XClock.RefreshComb()`。框架持有时钟控制权期间，通过框架请求组合求值。
旧生成包需重新导出、编译。Verilator 刷新使用 `eval_step()`；纯组合输入使用
`AsImmWrite()` 提交。

新生成的 VCS 包在子进程中完成结束流程，宿主进程可在 `Finish()` 后继续生成报告。

## 波形

| 方法 | 行为与返回值 |
| --- | --- |
| `SetWaveform(filename)` | 设置波形文件路径，格式与导出时的配置一致 |
| `GetWaveFormat()` | 返回扩展名，如 `fst` |
| `PauseWaveformDump()` | 暂停波形记录 |
| `ResumeWaveformDump()` | 恢复波形记录 |
| `WaveformPaused()` | 返回 1 表示暂停，0 表示记录中 |
| `FlushWaveform()` | 将波形缓冲写入文件 |

导出选项及完整脚本见[保存波形](../guides/simulation.md#recording)。

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
| `GetInternalSignal(name, index=-1, is_array=False, use_vpi=False)` | 按名称返回信号对象；`is_array=True` 返回使用零基存储坐标的递归 `XArray` |
| `GetInternalSignalView(name, use_vpi=False)` | 返回使用 RTL 坐标的递归 `XView`；默认优先原生元数据，`use_vpi=True` 强制 VPI |
| `dut.xcfg.Metadata("ports")` | 从 `.xscfg` 配置库读取端口元数据 JSON |
| `GetInternalSignalList(prefix="", deep=99, use_vpi=False)` | 返回可访问名称；默认优先原生布局，随后 VPI |
| `VPIInternalSignalList(prefix="", deep=99)` | 通过 VPI 枚举层次名称 |

原生查询中的 `prefix` 按名称子串筛选。VPI 中的 `prefix` 和 `deep` 指定起始层次与搜索深度。
原生单元素 `index` 是扁平存储位置；VPI 数组选择使用该层的 RTL 下标。

成功查询的结果按名称和查询选项缓存，参数相同的后续调用复用首次查询的对象。具体示例见[访问内部信号](../guides/internal-signals.md)。

## Packed 信号类型与显式视图

以下方法属于生成端口或内部查询返回的 `XData`，使用原生 ABI 6 的匹配 SDK。
packed 信号默认是完整值。

| 方法 | 行为 |
| --- | --- |
| `data.HasLayout()` | 检查是否关联类型；转换时仍检查位宽和来源有效性 |
| `data.ArrayView()` | 用自身关联的 packed 数组布局取得共享值的 `XArray` |
| `data.StructView()` | 用自身关联的 packed struct 布局取得共享值的 `XStruct` |
| `data.SetLayout(layout_json)` | 为手工 Data 显式设置类型；生成的聚合端口和可用的原生类型由 xcfg 关联 |
| `data.ArrayView(layout_json)` / `data.StructView(layout_json)` | 显式提供布局 |

无参视图及选中的叶子按需缓存。Python 默认字段访问和 unpacked 数组元素中的 packed struct/array 返回 Data，
需要结构化访问时再显式转换。普通 Data 位索引仍是最低位为 0；数组视图使用 RTL 声明下标。
`ReInit()` 清除关联类型及缓存。详见
[类型布局与显式视图](../explanation/data-layouts.md)。

## Python 异步方法

| 方法 | 行为 |
| --- | --- |
| `await dut.AStep(n)` | 等待仿真推进 n 个周期 |
| `await dut.ACondition(fn)` | 在周期通知时检查 fn，条件成立后返回 |
| `await dut.RunStep(n)` | 推进 n 个周期，并在周期之间让其他协程执行 |

等待协程需要与推进仿真的协程一起运行。`AStep()` 和 `ACondition()` 使用时钟的 Python 周期通知。

## 框架集成

`GetXPort()` 返回 DUT 的端口集合 `XPort`，`GetXClock()` 返回时钟对象 `XClock`。框架可通过这两个对象接入信号和时钟管理。它们及内部信号对象在 DUT 存活期间使用。
