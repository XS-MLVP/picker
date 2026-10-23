# 用 Python 验证一个加法器

这篇教程会把一个 128-bit 加法器导出为 Python 模块，先计算 `2 + 3 + 1`，再检查进位和溢出。完成后，你会得到一个可以重复运行的测试脚本。

开始前，请完成[安装](../guides/installation.md)，并准备好 Picker、Verilator、SWIG 和 Python 环境。

## 1. 获取加法器

获取仓库并进入根目录。已有源码时，直接进入该目录：

```sh
git clone https://github.com/XS-MLVP/picker.git
cd picker
```

打开 `example/Adder/Adder.v`，找到 `Adder` 模块。它的端口如下：

| 端口 | 方向 | 位宽 | 含义 |
| --- | --- | --- | --- |
| `a`、`b` | 输入 | 128 | 两个加数 |
| `cin` | 输入 | 1 | 输入进位 |
| `sum` | 输出 | 128 | 结果的低 128 位 |
| `cout` | 输出 | 1 | 输出进位 |

模块的计算式是 `assign {cout, sum} = a + b + cin;`。它是组合电路，每次更新输入后即可计算结果。

## 2. 导出并编译

在仓库根目录执行：

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --sim verilator --tdir build/docs-adder/Adder --autobuild false
make -C build/docs-adder/Adder -j4
```

第一条命令选择 `Adder` 模块，在 `build/docs-adder/Adder` 生成工程。第二条命令编译 RTL 和 Python 接口。编译完成后，`Adder` 目录就是可以导入的 Python 包。

## 3. 计算一次加法

将下面的代码保存为 `build/docs-adder/check.py`：

<!-- runnable:python -->
```python
from Adder import DUTAdder

dut = DUTAdder()
dut.a.value = 2
dut.b.value = 3
dut.cin.value = 1
dut.Step(1)
print(f"sum={dut.sum.value}, cout={dut.cout.value}")
dut.Finish()
```

从包的父目录运行脚本：

```sh
cd build/docs-adder
python3 check.py
```

输出中应包含：

```text
sum=6, cout=0
```

`DUTAdder()` 创建一个仿真模型。每个端口通过同名属性访问，`.value` 用来读写端口值。`Step(1)` 将输入提交给模拟器并更新输出，`Finish()` 结束仿真。

把 `cin.value` 改为 `0`，再次运行脚本。现在应得到 `sum=5, cout=0`。

## 4. 检查进位和溢出

将 `check.py` 替换为下面的代码。这次用断言自动检查三组输入：普通加法、最大值加一，以及两个高位相加。

<!-- runnable:python -->
```python
from Adder import DUTAdder

mask = (1 << 128) - 1
cases = [(2, 3, 1), (mask, 1, 0), (1 << 127, 1 << 127, 1)]
dut = DUTAdder()
try:
    for a, b, cin in cases:
        dut.a.value = a
        dut.b.value = b
        dut.cin.value = cin
        dut.Step(1)
        expected = a + b + cin
        assert dut.sum.value == (expected & mask)
        assert dut.cout.value == (expected >> 128)
        print(f"sum={dut.sum.value}, cout={dut.cout.value}")
    print("Adder: 3 cases passed")
finally:
    dut.Finish()
```

再次执行 `python3 check.py`，输出中应包含：

```text
sum=6, cout=0
sum=0, cout=1
sum=1, cout=1
Adder: 3 cases passed
```

Python 整数保留完整计算结果。`expected & mask` 取低 128 位，与 `sum` 比较；`expected >> 128` 取进位，与 `cout` 比较。任意一项不符合预期，断言都会指出失败的位置。

你已经完成了导出、输入激励、推进仿真和检查输出。接下来可以[导出自己的 RTL](../guides/export.md)，或学习怎样给时序电路[设置时钟和复位](../guides/simulation.md)。
