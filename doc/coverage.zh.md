# Picker 覆盖率功能

本文说明当前 Picker 的代码覆盖率采集与查询功能。`docs/` 下的材料对应早期方案，仅作为历史参考；接口和数据结构以本文及当前源码为准。

## 1. 支持范围

Picker 在仿真器原生覆盖率数据库之上提供统一的 Python 查询接口：

```text
RTL 仿真
  -> 仿真器生成原生覆盖率数据库
  -> Picker 覆盖率查询器读取数据库
  -> xspcomm 解析查询器标准输出
  -> Python 返回 CoverageReport
```

| 能力 | VCS | Verilator |
| --- | --- | --- |
| 原生数据库 | `<DUT>.vdb/` | `V<DUT>_coverage.dat` |
| 数据读取方式 | Synopsys UCAPI | Coverage-3 文本解析 |
| line | 支持 | 支持 |
| toggle / branch / condition / FSM | 支持 | 不支持 |
| assertion | 不支持查询 | 不支持查询 |
| module 过滤 | 支持 | 支持 |
| instance 过滤 | 支持 | 支持 |
| VCS 命名 testdata 查询及合并 | 支持 | 不适用 |
| 未覆盖对象明细 | 支持 | 支持 line |

VCS 默认编译参数采集 `line+cond+fsm+tgl+branch`。Verilator 的当前查询实现只处理 line coverage。`GetCoverage()` 对两种仿真器均默认查询 `line`；VCS 的其他指标需要通过 `kind` 显式指定。

## 2. 导出与构建

启用覆盖率：

```bash
picker export rtl/MyDut.sv \
  --sname MyDut \
  --sim vcs \
  --lang python \
  --coverage
```

导出目录中的覆盖率相关内容为：

```text
<export-dir>/
  coverage/
    coverage.cpp    覆盖率查询器源码
    coverage        构建后生成的查询器可执行文件
```

查询器直接使用仿真器记录的源码路径。当前实现不生成或维护额外的源码映射表，也不要求 `--source-root`。

VCS 可通过 `--coverage-dir <dir>` 指定 VDB 输出目录。Verilator 使用导出工程约定的 `coverage.dat` 路径，不接受 `--coverage-dir`。

## 3. 运行时接口

### 3.1 `SetCoverage(name)`

选择后续覆盖率快照名称。

- VCS 将 `name` 用作 VDB testdata 名称。
- Verilator 不支持命名 testdata；覆盖率持续累计到同一个 `coverage.dat`。

### 3.2 `FlushCoverage()`

将当前计数器提交到可查询的仿真器数据库，同时保留仿真实例和累计计数器。典型调用顺序为：

```python
dut.SetCoverage("reset_case")
# drive stimulus
assert dut.FlushCoverage() == 0
report = dut.GetCoverage()
```

VCS 的快照提交与 UCAPI 查询通过 `<database>.lock` 协调，避免同时修改和读取 VDB。

### 3.3 `GetCoverage()`

读取已经提交的覆盖率数据库，不触发隐式 flush：

```python
report = dut.GetCoverage(
    kind=["line", "branch"],
    module="MyDut",
    instance="TOP.u_dut",
    test="reset_case",
)
```

参数：

| 参数 | 含义 |
| --- | --- |
| `kind` | 字符串或字符串序列；缺省为 `line` |
| `module` | module 名称的子串过滤 |
| `instance` | 层次化 instance 名称的前缀过滤 |
| `test` | VCS testdata 名称；Verilator 不支持 |

VCS 不指定 `test` 时，查询器加载并合并数据库中可用的 testdata。部分 testdata 无法加载时，成功加载的数据仍参与统计，诊断信息写入 `report.errors` 并由 Python 发出 `RuntimeWarning`；没有任何可用 testdata 时查询失败。

### 3.4 `PrintCoverage()`

调用 `GetCoverage()`，打印指标汇总和未覆盖对象，并返回同一个 `CoverageReport`：

```python
report = dut.PrintCoverage(kind="line")
```

### 3.5 其他接口

| 接口 | 含义 |
| --- | --- |
| `GetCoveragePath()` | 返回当前仿真器覆盖率数据库路径 |
| `GetCovMetrics()` | 返回编译时启用的 coverage metric 位掩码 |
| `ResetCoverage()` | 清除当前仿真器累计覆盖率计数 |
| `Finish()` | 完成最后一次提交并结束仿真实例；可重复调用 |

## 4. 数据格式

### 4.1 仿真器数据库

VCS 使用目录形式的 VDB。查询器通过 UCAPI 读取 design、instance、metric、coverage object 和 testdata；相关实现位于：

```text
template/coverage/vcs/vcs_uncover.cpp
```

Verilator 使用文本格式 `coverage.dat`，首行为：

```text
# SystemC::Coverage-3
```

查询器解析 line record 中的文件、行号、module、instance、对象名称和 hit count；相关实现位于：

```text
template/coverage/verilator/verilator_coverage.cpp
```

### 4.2 覆盖率查询器标准输出

两个查询器均将 JSON 写到标准输出。该 JSON 是查询器与 xspcomm 之间的数据交换格式。其数据结构在以下位置定义：

| 内容 | 源码位置 |
| --- | --- |
| C++ 数据模型 | `dependence/xcomm/include/xspcomm/xcoverage.h` |
| JSON 解析及查询器进程调用 | `dependence/xcomm/src/xcoverage.cpp` |
| VCS JSON 生成 | `template/coverage/vcs/vcs_uncover.cpp` 的 `render_json()` |
| Verilator JSON 生成 | `template/coverage/verilator/verilator_coverage.cpp` 的 `make_report_json()` |

重要字段示例：

```json
{
  "schema_version": 1,
  "success": true,
  "simulator": "vcs",
  "query": {
    "kinds": ["line", "branch"],
    "module": "MyDut",
    "instance": "TOP.u_dut",
    "tests": ["reset_case"]
  },
  "metrics": {
    "line": {
      "covered": 18,
      "total": 20,
      "uncovered": 2,
      "rate": 90.0
    }
  },
  "items": [
    {
      "kind": "line",
      "file": "rtl/MyDut.sv",
      "line": 42,
      "column": 0,
      "module": "MyDut",
      "instance": "TOP.u_dut",
      "object": "if_enable",
      "count": 0,
      "detail": "else"
    }
  ],
  "errors": []
}
```

`schema_version` 的当前值及 C++ 常量定义在 `dependence/xcomm/include/xspcomm/xcoverage.h`。查询器输出不同版本时，`CoverageClient` 拒绝解析。

### 4.3 查询字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `kinds` | string array | 实际查询的 coverage kind |
| `module` | string/null | module 过滤条件 |
| `instance` | string/null | instance 过滤条件 |
| `tests` | string array | 实际成功加载并参与查询的 VCS testdata；Verilator 为空 |

### 4.4 报告字段

| 字段 | 含义 |
| --- | --- |
| `schema_version` | 查询器标准输出格式版本 |
| `success` | 查询是否成功 |
| `simulator` | 数据来源，当前为 `vcs` 或 `verilator` |
| `query` | 本次实际执行的查询条件 |
| `metrics` | 各 coverage kind 的统计结果 |
| `items` | 未覆盖对象明细；不包含已覆盖对象列表 |
| `errors` | 查询诊断；可包含 VCS testdata 部分加载失败信息 |

每个 metric 的重要字段：

| 字段 | 含义 |
| --- | --- |
| `kind` | C++ `CoverageMetricSummary` 中的指标类型；JSON 中作为 `metrics` 的键 |
| `covered` | 已覆盖并计入统计的对象数 |
| `total` | 计入覆盖率分母的对象总数 |
| `uncovered` | `total - covered` |
| `rate` | `covered / total * 100`，单位为百分比 |

每个未覆盖 item 的重要字段：

| 字段 | 含义 |
| --- | --- |
| `kind` | `line`、`toggle`、`branch`、`condition` 或 `fsm` |
| `file` / `line` / `column` | 仿真器数据库记录的源码位置 |
| `module` / `instance` | RTL 定义及层次化实例位置 |
| `object` | 覆盖率对象名称 |
| `count` | 命中次数；未覆盖对象通常为 0 |
| `detail` | 仿真器对象的补充描述，如 branch arm 或 FSM transition |

Python 中 `report.metrics` 和 `report.items` 分别映射到 `CoverageMetricSummary` 与 `CoverageItem`。可使用：

```python
line = report.metric("line")
print(line.covered, line.total, line.rate)

for item in report.items:
    print(item.location.file, item.location.line, item.module, item.instance)
```

## 5. 直接诊断查询器

VCS：

```bash
./coverage/coverage \
  --database MyDut.vdb \
  --kind line \
  --kind branch \
  --module MyDut \
  --instance TOP.u_dut \
  --test reset_case
```

Verilator：

```bash
./coverage/coverage \
  --database VMyDut_coverage.dat \
  --kind line \
  --module MyDut
```

标准输出只包含 JSON；参数错误、数据库读取失败等信息写到标准错误并返回非零状态。

## 6. 示例与验证

完整示例位于：

```text
example/Coverage/
```

示例对同一个 DUT 依次执行 reset、port 0、port 1 激励，每个阶段调用 `SetCoverage()`、`FlushCoverage()` 和 `PrintCoverage()`，并检查累计 line coverage 不下降。

聚焦回归：

```bash
bash test/scripts/test_verilator_coverage_helper.sh
bash test/scripts/test_vcs_coverage_dir_codegen.sh
make test_vcs_Coverage
make test_Coverage
```

VCS 查询器的编译和实际数据库查询要求可用的 VCS/UCAPI 环境；Verilator 查询器的解析回归不依赖正在运行的仿真器。
