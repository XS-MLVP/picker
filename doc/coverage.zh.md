# Picker 覆盖率功能说明

Picker 提供从 simulator-native coverage database 到统一 coverage data model 的适配链路：

```text
Simulator 采集 code coverage
  -> FlushCoverage()/Finish() finalize coverage snapshot
  -> Coverage provider adapter 读取 simulator-native database
  -> xspcomm 构造统一 CoverageReport
  -> Toffee 生成 coverage report artifacts
```

当前支持 VCS 和 Verilator。两者共用同一组 Python query API，但 native database format、provider capability 和 snapshot semantics 不同。

本文以当前代码实现为准。`docs/coverage/` 中的设计文档版本较老，只适合作为历史背景参考。

## 1. 功能概览

| 能力 | VCS | Verilator |
| --- | --- | --- |
| Simulator-native database | `<DUT>.vdb` | `V<DUT>_coverage.dat` |
| Provider adapter | VCS UCAPI adapter | Coverage-3 data adapter |
| line | 支持 | 支持 |
| toggle | 支持 | 查询暂不支持 |
| branch | 支持 | 查询暂不支持 |
| condition | 支持 | 查询暂不支持 |
| FSM | 支持 | 查询暂不支持 |
| assertion | 默认可采集，查询暂不支持 | 查询暂不支持 |
| module 过滤 | 支持 | 支持 |
| instance 过滤 | 支持 | 支持 |
| 命名 testdata | 支持 | 不支持 |
| 合并多个 testdata | 支持 | 不适用 |
| covered/uncovered 明细 | 支持 | line 支持 |
| 自定义数据库目录 | 支持 | 不支持 `--coverage-dir` |
| 稳定源码映射 | 支持 | 支持 |

需要区分两个概念：

- **采集指标**：模拟器编译时启用了哪些 coverage metric；
- **查询指标**：provider adapter 当前能够解析并映射到 `CoverageReport` 的 metric。

例如 VCS 默认会采集 assertion coverage，但当前 `GetCoverage()` 不支持查询 assertion。`GetCovMetrics()` 描述 compile-time collection configuration，不代表所有置位指标都已实现 provider-side extraction。

## 2. 工作方式

执行带 `--coverage` 的 export 后，Picker 除 DUT wrapper 外还会生成 coverage provider adapter 源码和 source identity map；构建导出目录后生成 adapter executable：

```text
<export-dir>/
  coverage/
    coverage.cpp       provider adapter 源码
    coverage           provider adapter executable
    source-map.tsv     source identity map
```

运行测试时，模拟器将 coverage counters 和 coverage object 状态提交到各自的 native database：

- VCS 写入 `.vdb`，VCS adapter 通过 `libucapi.so`/UCAPI 查询 design、metric 和 testdata；
- Verilator 写入 `coverage.dat`，Verilator adapter 解析 Coverage-3 records。

Python 的 `GetCoverage()` 创建 `xspcomm.CoverageClient`，通过子进程调用对应 provider adapter，并返回 simulator-agnostic `CoverageReport`。上层验证环境不需要直接依赖 UCAPI 或解析 `coverage.dat`。

### 2.1 Coverage Data Formats

| 数据 | 格式 | 用途 |
| --- | --- | --- |
| VCS native coverage database | `<DUT>.vdb/` 目录 | 保存 design、shape 和 testdata coverage records |
| Verilator native coverage database | `coverage.dat` 文本，头为 `# SystemC::Coverage-3` | 保存累计 line coverage records 和 hit counts |
| Source identity map | `coverage/source-map.tsv` | 将 simulator-reported path 映射为稳定 source identity |
| Provider IPC payload | stdout JSON，`schema_version=5` | provider adapter 与 `CoverageClient` 之间的进程边界序列化数据 |
| In-memory coverage model | `xspcomm.CoverageReport` | simulator-agnostic metric summary、coverage atoms 和 diagnostics |
| Coverage report artifacts | Toffee 管理的 JSON | 保存 cumulative coverage、testcase observation 和 catalog 等报告数据 |

`source-map.tsv` 每行有四列：

```text
native_path<TAB>source_id<TAB>logical_file<TAB>resolved_file
```

Provider adapter 通过 stdout 返回 IPC JSON payload，由 `CoverageClient` 在子进程边界接收并反序列化。对验证用户有直接意义的部分是查询条件和覆盖率报告：

```json
{
  "query": {
    "kinds": ["line", "branch"],
    "module": "MyDut",
    "instance": "TOP.u_dut",
    "tests": ["phase_1"],
    "detail_mode": "all"
  },
  "summary": {
    "metrics": {
      "line": {
        "covered": 18,
        "total": 20,
        "uncovered": 2,
        "rate": 90.0,
        "available": true,
        "details_available": true
      }
    }
  },
  "items": [
    {
      "kind": "branch",
      "logical_file": "rtl/MyDut.sv",
      "line": 42,
      "module": "MyDut",
      "instance": "TOP.u_dut",
      "object": "if_enable",
      "count": 0,
      "detail": "else",
      "atom_kind": "branch_arm"
    }
  ],
  "covered_items": [],
  "errors": []
}
```

协议和数据模型的定义位置如下：

| 内容 | 定义位置 |
| --- | --- |
| VCS provider serialization | `template/coverage/vcs/vcs_uncover.cpp` 中的 `render_json()` |
| Verilator provider serialization | `template/coverage/verilator/verilator_coverage.cpp` 中的 `make_report_json()` |
| `CoverageReport` data model | `dependence/xcomm/include/xspcomm/xcoverage.h` |
| IPC payload deserialization | `dependence/xcomm/src/xcoverage.cpp` |

该 JSON 是 Picker/xspcomm 进程边界上的 IPC representation。调试 provider 时可以手工重定向 stdout：

```bash
./coverage/coverage --database MyDut.vdb --kind line > coverage-result.json
```

面向回归分析和结果归档的 coverage report artifacts 由 Toffee 生成。

### 2.2 `CoverageQuery`

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `kinds` | string array | 实际查询的指标，如 `line`、`toggle`、`branch`、`condition`、`fsm` |
| `module` | string/null | module 子串过滤；未设置时为 `null` |
| `instance` | string/null | instance 前缀过滤；未设置时为 `null` |
| `tests` | string array | 实际成功加载并参与查询的 VCS testdata；Verilator 为空数组 |
| `detail_mode` | string | `summary`、`all`、`covered` 或 `uncovered` |

VCS 的 `tests` 记录实际参与 merge 的 testdata。某个请求的 testdata 加载失败时，它不会出现在 `tests` 中，失败原因进入 `CoverageReport.errors`。

### 2.3 `CoverageReport`

`GetCoverage()` 返回 `xspcomm.CoverageReport`。验证流程主要使用以下字段：

| 字段 | 含义 |
| --- | --- |
| `success` | 查询是否成功 |
| `metrics` | 各 coverage kind 的统计结果 |
| `items` | 未覆盖 coverage atoms |
| `covered_items` | 已覆盖 coverage atoms |
| `errors` | Provider diagnostics，例如部分 VCS testdata 加载失败 |
| `design_revision` | Coverage design/inventory 的版本标识 |
| `inventory_revision` | 当前 uncovered inventory 的版本标识 |

每个 metric 包含：

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `covered` | integer | 已覆盖且计入覆盖率的对象数 |
| `total` | integer | 计入覆盖率分母的对象总数 |
| `uncovered` | integer | `total - covered` |
| `rate` | number | `covered / total * 100`，单位为百分比 |
| `available` | boolean | 数据库中是否存在该指标 |
| `details_available` | boolean | 是否能够返回该指标的对象级明细 |

`items` 和 `covered_items` 使用相同的 coverage atom 结构。重要字段如下：

| 字段 | 含义 |
| --- | --- |
| `kind`、`atom_kind` | Coverage metric 和原子类型，例如 `branch/branch_arm` |
| `location` | `source_id`、`logical_file`、`resolved_file`、`line`、`column` |
| `module`、`instance` | RTL definition 和 elaborated hierarchy |
| `object`、`detail` | Coverage object 及其 arm、transition 或 bin 信息 |
| `count` | Simulator-reported hit count |
| `native_id`、`native_name` | Simulator-native atom identity |
| `from_state`、`to_state` | FSM transition 的起止状态 |
| `counts_toward_rate` | 是否计入 coverage numerator/denominator |

Python 访问示例：

```python
report = dut.GetCoverage(kind="line")

print(report.success)
print(list(report.query.tests))
print(list(report.errors))

line = report.metric("line")
print(line.covered, line.total, line.uncovered, line.rate)

for item in report.items:
    # JSON 的 source_id/logical_file/resolved_file 位于 C++ location 子对象中。
    print(item.location.source_id)
    print(item.location.logical_file)
    print(item.location.resolved_file)
    print(item.location.line, item.location.column)
    print(item.module, item.instance, item.object)
    print(item.atom_kind, item.detail, item.count)
```

`PrintCoverage()` 内部会把 `CoverageItem` 转换成便于 console rendering 的 Python `UncoverItem`。

## 3. 启用覆盖率

### 3.1 VCS

基本导出方式：

```bash
picker export \
  <rtl-files> \
  --sim vcs \
  --lang python \
  --coverage \
  ...
```

未指定 `-cm` 时，Picker 默认增加：

```text
-cm line+cond+fsm+tgl+branch+assert
```

可以通过 `--vflag` 限制实际采集的指标。例如只采集 line、toggle 和 branch：

```bash
picker export \
  <rtl-files> \
  --sim vcs \
  --coverage \
  --vflag '"-cm line+tgl+branch"' \
  ...
```

### 3.2 指定 VCS 数据库目录

使用 `--coverage-dir` 指定 `.vdb` 的父目录：

```bash
picker export \
  <rtl-files> \
  --sim vcs \
  --coverage \
  --coverage-dir /tmp/project-coverage \
  ...
```

最终数据库路径为：

```text
/tmp/project-coverage/<DUT>.vdb
```

约束如下：

- 仅支持 VCS；
- 必须同时启用 `--coverage`；
- 不能再通过 `--vflag` 传入 `-cm_dir`，否则数据库路径会冲突；
- 相对路径会在 export 时解析成规范化绝对路径。

不指定 `--coverage-dir` 时，VDB 默认放在导出的 DUT 动态库附近。

### 3.3 Verilator

```bash
picker export \
  <rtl-files> \
  --sim verilator \
  --lang python \
  --coverage \
  ...
```

默认数据库名为：

```text
V<DUT>_coverage.dat
```

也可以在构造 DUT 时传入 `.dat` 文件名，或调用 `SetCoverage("name.dat")` 修改输出文件。Verilator 不支持 `--coverage-dir`。

## 4. 稳定源码路径

模拟器数据库中的源码路径可能来自编译目录、打包目录或绝对路径。为了让覆盖率对象在不同构建目录间保持稳定身份，Picker export 时生成 `coverage/source-map.tsv`。

可以重复使用 `--source-root` 指定源码根目录：

```bash
picker export \
  <rtl-files> \
  --coverage \
  --source-root /workspace/project \
  --source-root /workspace/common-rtl \
  ...
```

位于 source root 下的文件使用相对逻辑路径，例如：

```text
rtl/core/alu.sv
```

不属于任何 source root 的文件使用带内容哈希的 `external/...` 路径，避免只依赖机器上的绝对路径。

报告中的源码相关字段为：

| 字段 | 含义 |
| --- | --- |
| `source_id` | 稳定源码身份，例如 `src:rtl/core/alu.sv` |
| `logical_file` | 面向报告和跨工作区比较的逻辑路径 |
| `resolved_file` | 当前 DUT package 中解析后的本地源码路径 |

## 5. Python Verification Flow

推荐的 simulator/coverage lifecycle：

```python
dut = DUTMyDut()
dut.InitClock("clk")

# 运行激励
dut.Step(100)

# VCS：设置下一次 testdata name；Verilator：仅接受 .dat output path
dut.SetCoverage("phase_1")

# Finalize 当前 coverage snapshot，同时保持 simulator instance 可继续运行
assert dut.FlushCoverage() == 0

# 返回结构化对象，不打印
report = dut.GetCoverage(kind=["line", "toggle", "branch"])

# 查询并打印，同时返回同一个 CoverageReport 类型
line_report = dut.PrintCoverage(kind="line", module="MyDut")

# Finalize 最后一个 snapshot，并释放 simulator runtime
assert dut.Finish() == 0
```

`GetCoverage()` 不会隐式调用 `FlushCoverage()`。查询运行中的 DUT 前，应先调用 `FlushCoverage()`，保证 native coverage database 已完成 snapshot finalization；`Finish()` 后可直接查询最终数据库状态。

## 6. Python API

### 6.1 `SetCoverage(name)`

设置覆盖率输出名称。

VCS 中，该名称表示下一次 `FlushCoverage()` 或 `Finish()` 对应的 testdata 名。名称按文件 stem 处理，并会转换不适合作为 testdata 名的字符。

Verilator 中：

- `SetCoverage("result.dat")` 设置累计数据库文件名；
- `SetCoverage("phase_1")` 不满足 `.dat` output path 约束，Picker 发出 warning，且不更新当前 coverage output path；
- Verilator 不会因为多次调用 `SetCoverage()` 产生独立命名快照。

### 6.2 `ResetCoverage()`

重置覆盖率计数器。

- VCS 支持；
- Verilator 当前不支持，调用会报错退出。

不调用 `ResetCoverage()` 时，多个阶段的覆盖率是累计关系。

### 6.3 `DumpCoverage()`

请求 simulator runtime 导出当前 coverage counters。验证流程通常优先使用 `FlushCoverage()`，因为它定义了完整的 snapshot finalization 语义并返回状态码。

### 6.4 `FlushCoverage()`

Finalize 当前 coverage snapshot，同时保留 simulator instance，成功返回 `0`。

- VCS 在 fork child 中执行 `$finish`，触发 UCAPI-readable testdata commit；parent process 中的 simulator instance 继续运行；
- Verilator 将当前累计 counters 写入 `coverage.dat`。

Flush 不会自动重置计数器。

### 6.5 `GetCoveragePath()`

返回当前覆盖率数据库路径：

- VCS：`<DUT>.vdb`；
- Verilator：配置的 `.dat` 路径，未配置时为 `V<DUT>_coverage.dat`；
- 未启用 coverage 时通常返回空字符串。

### 6.6 `GetCoverageHelperPath()`

返回生成的 provider adapter executable `coverage/coverage` 路径。adapter 未生成或不存在时返回空字符串。

### 6.7 `GetCovMetrics()`

返回编译阶段启用的覆盖率指标 bitmask：

| Bit | 指标 |
| --- | --- |
| 0 | line |
| 1 | condition |
| 2 | FSM |
| 3 | toggle |
| 4 | branch |
| 5 | assertion |

返回 `0` 表示未启用覆盖率采集。

### 6.8 `GetCoverage(...)`

接口形式：

```python
dut.GetCoverage(
    kind=None,
    module=None,
    instance=None,
    test=None,
    report=None,
)
```

参数含义：

| 参数 | 含义 |
| --- | --- |
| `kind` | 单个 kind、kind 列表、`"all"` 或 `None` |
| `module` | module 名称过滤 |
| `instance` | instance 层级过滤 |
| `test` | VCS testdata 名称；Verilator 不支持 named testdata，发出 warning 后忽略该过滤条件 |
| `report` | 当前不接受报告路径；JSON 持久化由 Toffee 负责 |

VCS 的 `kind=None` 或 `kind="all"` 默认查询：

```text
line, toggle, branch, condition, fsm
```

Verilator 默认且当前唯一支持的查询类型是 `line`。

返回值为 `xspcomm.CoverageReport`。该接口只查询，不打印。

### 6.9 `dut.Coverage`

`dut.Coverage` 是 `dut.GetCoverage()` 的惰性别名：

```python
report = dut.Coverage
```

该别名通过 `__getattr__` 实现，不会出现在普通 DUT 属性枚举中，从而避免 Toffee 绑定信号时意外查询尚未完成的数据库。

### 6.10 `PrintCoverage(...)`

参数与 `GetCoverage()` 相同。它先查询，再调用 xspcomm 的打印逻辑输出人类可读摘要，并返回 `CoverageReport`：

```python
report = dut.PrintCoverage(kind="line", instance="TOP.u_core")
```

## 7. VCS 指标语义

| kind | 报告原子 | 说明 |
| --- | --- | --- |
| `line` | `statement` | 可覆盖语句或代码行 |
| `branch` | `branch_arm` | 分支对象的具体 arm |
| `toggle` | `toggle_transition` | 信号 bit 的 `0 -> 1` 或 `1 -> 0` 转换 |
| `condition` | `condition_bin` | VCS UCAPI 原生 truth-vector bin，例如 `00/01/10/11` |
| `fsm` | `fsm_state`、`fsm_transition` | FSM 状态和状态转移 |

FSM state 会作为明细返回，但 `counts_toward_rate=false`，不进入覆盖率分母；FSM transition 才参与覆盖率统计，并保留 `from_state` 和 `to_state`。

Coverage inventory 会排除 Picker 生成的 `<DUT>_top.sv` testbench wrapper，同时保留 DUT RTL；DUT RTL 即使以 `build/` 路径写入 simulator database，也继续计入 coverage denominator。

## 8. VCS Named Snapshots and Testdata

VCS `.vdb` 是目录数据库，核心结构可概括为：

```text
<DUT>.vdb/snps/coverage/db/
  design/      设计层级等静态信息
  shape/       可覆盖对象集合
  testdata/    每个测试实际命中的对象
  auxiliary/   编译和工具辅助信息
```

`SetCoverage()` 与 `FlushCoverage()` 的关系是：

```text
SetCoverage("phase_1")
  -> 继续运行激励
  -> FlushCoverage()
  -> Finalize VCS 当前 coverage counters
  -> Commit 为名为 phase_1 的 testdata
```

VCS code coverage 的 native testdata name 由启动参数 `-cm_name` 控制，运行时 `$coverage_dump(name)` 不会直接创建同名 testdata directory。因此 Picker 先提交 VCS runtime testdata，再复制为 `SetCoverage()` 指定的 testdata name。

未调用 `ResetCoverage()` 时：

```text
phase_2 = phase_1 已有覆盖 + phase_2 新增覆盖
```

需要相互独立的阶段时，应在两个阶段之间调用 `ResetCoverage()`。

## 9. VCS testdata 查询与合并

指定 test：

```python
report = dut.GetCoverage(kind="line", test="phase_1")
```

只查询对应 testdata。

不指定 test：

```python
report = dut.GetCoverage(kind="line")
```

Picker 会发现并合并 VDB 中所有可用的命名 testdata。若存在命名 testdata，内部默认 testdata 不参与合并，避免重复统计。

失败策略：

- 某个 testdata 加载或合并失败时，继续使用其余成功数据；
- 失败信息放入 `CoverageReport.errors`，Python 同时发出 `RuntimeWarning`；
- 所有 testdata 都不可用时，查询失败并抛出 `RuntimeError`。

## 10. 并发访问

VCS provider 使用 `<DUT>.vdb.lock` 对 native database access 进行进程级同步：

- `FlushCoverage()` 和 `Finish()` 使用独占锁；
- Provider query 使用共享锁。

该机制保证 query 不会观察到未完成的 VDB update，并串行化多个 snapshot finalization。直接绕过 Picker 调用 URG、UCAPI 或其他工具时不受该锁协议约束，仍需由验证环境自行协调并发访问。

## 11. CoverageReport

当前 provider adapter 通过 stdout 输出 IPC JSON payload，xspcomm 将其反序列化为 simulator-agnostic `xspcomm.CoverageReport`。验证环境通常使用：

- `report.metric(kind)` 获取指定 metric summary；
- `report.items` 定位 uncovered coverage atoms；
- `report.covered_items` 分析 testcase 的 coverage contribution；
- `report.errors` 检查 provider diagnostics；
- `report.design_revision` 和 `report.inventory_revision` 校验 coverage inventory 是否一致。

Query 和 report 的关键字段见“Coverage Data Formats”章节。

## 12. Report Ownership

Picker provider 负责：

- 读取 simulator-native coverage database；
- 规范化 source identity 和 coverage atom identity；
- 返回 `CoverageReport`；
- 提供 human-readable console rendering。

Picker 当前不负责生成持久化 coverage report artifacts。向 `GetCoverage(report=...)` 传入非空路径会抛出 `ValueError`。Cumulative coverage、testcase observation 和 catalog 等 report persistence 由 Toffee 负责。

因此推荐上层代码直接消费结构化对象：

```python
report = dut.GetCoverage()
for metric in report.metrics:
    print(metric.kind, metric.covered, metric.total)
```

## 13. 常见用法

查询所有 VCS 默认指标：

```python
dut.FlushCoverage()
report = dut.GetCoverage()
```

只查询 line：

```python
report = dut.GetCoverage(kind="line")
```

查询多个指标：

```python
report = dut.GetCoverage(kind=["line", "toggle", "branch"])
```

限定 module 和 instance：

```python
report = dut.GetCoverage(
    kind="branch",
    module="Core",
    instance="TOP.u_core",
)
```

查询指定 VCS testdata：

```python
report = dut.GetCoverage(
    kind=["line", "toggle"],
    test="phase_1",
)
```

打印未覆盖信息：

```python
dut.PrintCoverage(kind="line")
```

## 14. 示例和测试

覆盖率流程示例位于：

```text
example/Coverage/
```

示例已使用当前 API，不向 Picker 传递 `report=`。为了让同一测试同时适用于 VCS 和 Verilator，示例显式查询两者都支持的 line coverage；VCS 的五类默认查询能力可通过单独调用 `GetCoverage()` 验证。

运行 VCS 示例：

```bash
make test_vcs_Coverage
```

运行 Verilator 示例：

```bash
make test_Coverage
```

也可以直接生成示例：

```bash
bash example/Coverage/release-vcs.sh
bash example/Coverage/release-verilator.sh
```

相关轻量回归：

```bash
make -C test vcs_coverage_dir_codegen
make -C test coverage_helper
```

前者验证 VCS 数据库目录、生成代码、默认指标、source map 和参数约束；后者使用固定 `coverage.dat` 验证 Verilator line coverage 解析。

## 15. 当前限制

1. Verilator provider adapter 当前只实现 line coverage extraction。
2. Verilator 不支持命名 testdata，也不支持 `ResetCoverage()`。
3. VCS assertion coverage 可以被采集，但尚未实现查询。
4. Python API 当前没有直接暴露 provider adapter 的 `summary/all/covered/uncovered` detail mode，默认由 `CoverageClient` 按内部 query contract 使用。
5. `GetCoverage()` 不会自动执行 snapshot finalization；若验证环境未先调用 `FlushCoverage()`，查询结果可能对应上一次已提交的 database state。
6. VCS 的 condition/FSM 依赖 UCAPI 提供的原生对象结构，不同 VCS 版本仍需通过真实数据库回归验证。
7. `GetCovMetrics()` 表示采集配置，不是查询能力列表。
8. Coverage report artifacts 由 Toffee 持久化，Picker 的 `report=` 参数当前不用于生成 report file。
