# 故障排查

先保留完整导出命令和第一条错误。解析器、编译器或模拟器通常会在后续 `make` 报错之前给出具体原因。

## 导出和编译

| 检查项 | 检查与处理 |
| --- | --- |
| 模板路径 | 运行 `picker --check` 和 `picker --show_default_template_path`；使用完整安装目录，或通过 `--sdir` 指定模板 |
| 顶层 module | 检查 `--sname` 拼写、源码列表和条件编译宏 |
| include 路径 | 将头文件目录写入 filelist 的 `+incdir+`，检查相对路径 |
| 端口位宽 | 检查顶层参数、宏定义和展开后的接口文件 |
| SWIG 与编译器 | 检查 PATH，并按[安装指南](../guides/installation.md)准备工具 |
| 语言资源 | 查看 `picker --check` 对应条目，构建所需语言绑定 |

## Python 导入和信号访问

| 检查项或错误原文 | 检查与处理 |
| --- | --- |
| `No module named 'Adder'` | 从生成包的父目录运行，或把父目录加入 PYTHONPATH |
| 原生扩展加载 | 核对 Python 小版本、平台和架构，使用同一环境重新编译 |
| `libxspcomm` 与库符号 | 恢复完整的生成包和配套运行资源；更换依赖后重新导出编译 |
| `XData attributes of DUT are read-only` | 通过 `dut.pin.value = value` 写信号值 |
| 写入与输出刷新 | 写入输入后调用 `Step()`；组合求值可参考[驱动仿真](../guides/simulation.md) |
| 宽信号读取 | Python 使用 `.value`；C++ 使用完整字节接口，检查测试代码中的临时变量位宽 |
| 内部信号查询 | 导出时启用 VPI 或原生布局，先枚举实际名称，再查询对象 |

## 波形和 UVM

波形文件为空时，检查导出命令中的 `-w`、运行时输出路径和暂停状态，并调用 `Finish()` 完成输出。文件格式由导出配置决定。完整命令见[保存波形](../guides/simulation.md#recording)。

UVM 编译需检查 VCS、UVM、UVMC、Verdi 和 SDK 路径。事务传输需检查两端通道名和
driver/monitor 的执行情况，见[UVM 事务桥](../guides/uvm-tlm.md)。

## 报告问题

提交 [GitHub Issue](https://github.com/XS-MLVP/picker/issues) 时，附上最小 RTL、文件列表、完整导出命令和错误日志，以及 `picker --version`、操作系统、Python 和模拟器版本。私有设计可以简化成保留相关语法或接口的公开例子。
