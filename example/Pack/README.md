# Picker Pack 模式 - Python-UVM 混合验证完整指南

本目录包含 Picker Pack 功能的完整示例，展示如何使用 Python 驱动 UVM 验证环境，实现高效的芯片设计验证。

## 目录

- [概述](#概述)
- [快速开始](#快速开始)
- [核心概念](#核心概念)
- [示例说明](#示例说明)
- [主要功能](#主要功能)
- [使用场景](#使用场景)

---

## 概述

### 什么是 Picker Pack？

Picker Pack 是一个自动化工具，能够将 RTL 或 Transaction 定义转换为 Python 可调用的验证组件，实现以下功能：

- **Python DUT 类**：在 Python 中直接操作硬件/事物设计的输入输出
- **UVM Agent**：自动生成标准 UVM 组件（Driver、Monitor、Sequencer）
- **双向通信桥**：基于 UVMC 的 Python-SystemVerilog 数据交换机制

### 为什么使用 Picker Pack？

| 传统方法 | Picker Pack 方法 |
|---------|----------------|
| 用 SystemVerilog 写测试 | 用 Python 写测试（更简洁、灵活） |
| 手写 DPI/TLM 胶水代码 | 自动生成所有接口代码 |
| 受限于 UVM/SV 生态 | 利用 Python 丰富的库（NumPy、ML等） |
| 测试数据处理复杂 | Python 强大的数据处理能力 |

### 验证架构

```
Python 测试脚本 (*.py)
  - 生成激励（随机数、边界值、约束求解）
  - 调用 dut.Step() 驱动硬件
  - 读取输出并验证
  - 数据分析、可视化、机器学习
        |
        | TLM (UVMC)
        v
UVM 验证环境 (SystemVerilog)
  - xDriver: 接收 Python 数据并驱动 DUT
  - xMonitor: 采样 DUT 输出并回传 Python
  - xAgent: 完整的 UVM Agent 组件
        |
        | Virtual Interface
        v
DUT (硬件设计)
  - Verilog/SystemVerilog 模块
```

---

## 快速开始

### 方法一：使用自动化脚本（推荐）

```bash
# 在 picker 项目根目录执行
cd /path/to/picker

# 运行任意示例
bash example/Pack/release-pack.sh adder    # 完整 DUT 示例
bash example/Pack/release-pack.sh send     # Python → UVM 单向通信
bash example/Pack/release-pack.sh recv     # UVM → Python 单向通信
bash example/Pack/release-pack.sh dut      # 将事物封装为DUT 
bash example/Pack/release-pack.sh multi    # 多 Transaction 示例
```

### 方法二：手动运行示例

以 05_FullAdderDemo 为例：

```bash
# 1. 使用 --from-rtl 从 RTL 自动生成 Transaction
picker pack --from-rtl example/Adder/Adder.v -d -e

# 2. 进入生成的目录
cd uvmpy/

# 3. 复制测试文件
rm -rf example.{py,sv}
cp ../example/Pack/05_FullAdderDemo/example.{py,sv} .

# 4. 编译并运行
make clean comp copy_xspcomm run
```

### 预期输出

```
Initialized DUT Adder
[UVM_INFO] Received from Python: a=0x1234..., b=0x5678..., cin=0
[UVM_INFO] Sampled: a=0x1234..., b=0x5678... -> sum=0x68ac..., cout=0
Test Passed, destroy DUT Adder
```

---

## 核心概念

### 1. Transaction 定义

Transaction 是 UVM 验证中的核心数据结构，封装了一次完整的接口操作。Picker Pack 通过解析 Transaction 自动推断出所有输入/输出信号及其类型，从而生成 Python-UVM 通信接口。

```systemverilog
// adder_trans.sv
class adder_trans extends uvm_sequence_item;
    rand bit [127:0] a, b;  // 输入信号
    rand bit cin;
    bit [127:0] sum;        // 输出信号
    bit cout;

    `uvm_object_utils_begin(adder_trans)
        `uvm_field_int(a, UVM_ALL_ON)
        `uvm_field_int(b, UVM_ALL_ON)
        `uvm_field_int(cin, UVM_ALL_ON)
        `uvm_field_int(sum, UVM_ALL_ON)
        `uvm_field_int(cout, UVM_ALL_ON)
    `uvm_object_utils_end
endclass
```

### 2. Python DUT 接口

Picker 自动生成的 Python 类：

```python
from Adder import DUTAdder

# 创建 DUT 实例（自动启动仿真）
dut = DUTAdder()

# 设置输入
dut.a.value = 0x1234
dut.b.value = 0x5678
dut.cin.value = 0

# 推进仿真
dut.Step(10)

# 读取输出（已自动更新）
result = dut.sum.value
carry = dut.cout.value
```

### 3. 时序与数据流

**核心原理：** Python 通过 `Step()` 推进仿真时钟，每次 `Step(1)` 推进一个时钟周期。

**时序说明：**

```
Cycle N:
  Python 设置输入 (dut.input.value = x)
       |
       v
  Python 调用 Step(1)
       |
       +---> TLM 传输到 UVM
       +---> UVM Driver 接收并驱动 DUT
       +---> DUT 时钟上升沿，计算输出
       +---> UVM Monitor 采样输出
       +---> TLM 回传到 Python
       |
       v
  Python 读取输出 (result = dut.output.value)
Cycle N + 1:
```
---

## 数据流说明

### 01_Py2UVM - Python 到 UVM 的单向通信

**场景：** Python 发送数据到 UVM 环境

```python
from adder_trans import Agent, adder_trans

agent = Agent(monitor_callback=lambda t, o: print(f"Sent: {o}"))

for i in range(5):
    tr = adder_trans()
    tr.a.value = i * 10
    tr.b.value = i * 20
    agent.drive(tr)
    agent.run(1)
```

---

### 02_UVM2Py - UVM 到 Python 的单向通信

**场景：** UVM 内部生成数据，Python 接收并处理

```python
def monitor_callback(trans_type, trans_obj):
    print(f"Received: {trans_obj.a.value} + {trans_obj.b.value}")

agent = Agent(monitor_callback=monitor_callback)
agent.run(100)  # 运行 100 个周期，接收 UVM 数据
```

---

### 04_MultiTrans - 多 Transaction 双向通信

**场景：** 复杂设计中多种数据类型的交互（如 ALU 操作）

```python
from ALU import DUTALU

dut = DUTALU()

# 发送操作（alu_op transaction）
dut.opcode.value = 0      # ADD
dut.operand_a.value = 10
dut.operand_b.value = 20

dut.Step(1)

# 接收结果（alu_result transaction）
result = dut.result.value    # 30
status = dut.status.value    # 状态标志
```

---

## 主要功能

### 1. 从 RTL 自动生成 Transaction (`--from-rtl`)

**新功能！** 无需手写 Transaction，直接从 RTL 模块自动生成：

```bash
# 自动解析模块端口并生成 transaction
picker pack --from-rtl Adder.v -d
```

**生成内容：**
```systemverilog
// 自动生成的 Adder_trans.sv
class Adder_trans extends uvm_sequence_item;
    rand bit [127:0] a;      // 从 input [127:0] a 生成
    rand bit [127:0] b;      // 从 input [127:0] b 生成
    rand bit cin;            // 从 input cin 生成
    bit [127:0] sum;         // 从 output [127:0] sum 生成
    bit cout;                // 从 output cout 生成
    // ...
endclass
```

**优势：**
- 自动识别输入/输出端口
- 自动推断位宽
- 自动生成字段注册宏
- 减少手动错误

---

### 2. 时钟追踪 (`InitClock`)

**新功能！** 在 Python 中追踪仿真时钟周期：

```python
from adder_trans import Agent

agent = Agent()

# 初始化时钟追踪（默认域）
agent.InitClock()

# 或指定时钟域和频率
agent.InitClock(domain="sys_clk", frequency=100e6)

# 运行仿真
agent.run(100)

# 查询已执行的周期数
cycles = agent.GetCycleCount()
print(f"Executed {cycles} cycles")
```

**应用场景：**
- 性能测试（计算操作延迟）
- 时序验证（确保满足时序约束）
- 功耗分析（统计活跃周期）

---

### 3. 多 Transaction 支持 (`-f` + `-n`)

处理多个 Transaction 文件的复杂设计：

```bash
# 创建文件列表
cat > filelist.txt << EOF
alu_op.sv
alu_result.sv
EOF

# 生成统一的 Package
picker pack -f filelist.txt -n ALU -d
```

**生成结构：**
```
ALU/                          # 统一的 Package 名
├── ALU_trans.sv              # 包含所有 transaction
├── xagent.sv                 # 支持多类型 transaction 的 Agent
└── __init__.py               # Python 统一接口
```

---

### 4. 监控回调 (`SetUpdateCallback`)

实时追踪 Monitor 更新：

```python
def debug_callback(dut):
    print(f"[Monitor] a={dut.a.value}, sum={dut.sum.value}")

dut = DUTAdder()
dut.SetUpdateCallback(debug_callback)

# 每次 Monitor 更新时自动调用 callback
dut.Step(1)  # 触发回调
```


---

## 使用场景

### 场景 1：快速原型验证

**需求：** 新设计的 IP，需要快速验证基本功能

```bash
# 1. 从 RTL 生成验证环境
picker pack --from-rtl my_design.v -d

# 2. 编写 Python 测试（只需几分钟）
cat > test.py << 'EOF'
from my_design import DUTmy_design
import random

dut = DUTmy_design()
for i in range(1000):
    dut.input_port.value = random.randint(0, 255)
    dut.Step(1)
    # 简单验证
    assert dut.output_port.value >= 0
print("Basic test passed!")
EOF

# 3. 运行
python test.py
```

---

### 场景 2：大规模随机验证

**需求：** 使用 Python 的强大随机库进行约束随机测试

```python
import numpy as np
from scipy.stats import truncnorm

dut = DUTAdder()

# 使用正态分布生成更真实的激励
for i in range(100000):
    # 中间值附近的概率更高
    a = int(truncnorm.rvs(-2, 2, loc=2**127, scale=2**125))
    b = int(truncnorm.rvs(-2, 2, loc=2**127, scale=2**125))

    dut.a.value = a & ((1 << 128) - 1)
    dut.b.value = b & ((1 << 128) - 1)
    dut.Step(1)

    # 验证...
```

---

### 场景 3：机器学习辅助验证

**需求：** 使用 ML 模型预测 DUT 行为或生成智能测试用例

```python
import torch
from my_ml_model import DUTPredictor

dut = DUTAdder()
predictor = DUTPredictor()  # 预训练的神经网络

for i in range(10000):
    # ML 生成边界测试用例
    test_case = predictor.generate_edge_case()

    dut.a.value = test_case['a']
    dut.b.value = test_case['b']
    dut.Step(1)

    # 收集数据训练模型
    predictor.collect(test_case, dut.sum.value)
```

---

### 场景 4：协议监控和分析

**需求：** 监控总线协议，生成波形、统计报告

```python
import matplotlib.pyplot as plt

agent = Agent()
agent.InitClock(frequency=100e6)

transactions = []

def collect_transaction(trans_type, trans_obj):
    transactions.append({
        'cycle': agent.GetCycleCount(),
        'data': trans_obj.data.value,
        'valid': trans_obj.valid.value
    })

agent.monitor_callback = collect_transaction
agent.run(10000)

# 绘制带宽利用率
cycles = [t['cycle'] for t in transactions]
plt.plot(cycles)
plt.xlabel('Cycle')
plt.ylabel('Transactions')
plt.title('Protocol Activity')
plt.show()
```
