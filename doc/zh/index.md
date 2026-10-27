# Picker 文档

Picker 把 Verilog/SystemVerilog 设计生成为 Python、C++、Java、Scala、Go 或 Lua 模块。你可以在测试代码中给电路输入赋值、推进仿真、读取输出，并用断言检查结果。

第一次使用，从[安装 Picker](guides/installation.md)开始，然后跟随[加法器教程](tutorials/first-dut.md)完成一个可运行的测试。

## 使用 Picker

| 想做什么 | 文档 |
| --- | --- |
| 导出自己的 RTL | [导出 RTL 工程](guides/export.md) |
| 设置时钟、复位和采样 | [驱动仿真](guides/simulation.md) |
| 保存波形、收集代码覆盖率 | [记录仿真结果](guides/simulation.md#recording) |
| 观察内部寄存器或存储器 | [访问内部信号](guides/internal-signals.md) |
| 使用 C++、Java 等语言 | [多语言示例](guides/languages.md) |
| 在 Python 与 UVM 之间传递事务 | [生成 UVM 事务桥](guides/uvm-tlm.md) |

## 查询接口

[命令行参考](reference/cli.md)列出导出选项；[DUT API](reference/dut-api.md)说明生成对象的方法和信号访问方式。[模拟器与语言支持](reference/compatibility.md)可用于选择工具链，运行中遇到问题可查[故障排查](reference/troubleshooting.md)。

## 理解和修改 Picker

[生成与运行原理](explanation/architecture.md)介绍 RTL 如何成为可调用的仿真模块。[开发指南](development/contributing.md)包含源码目录和测试入口；[文档维护](development/documentation.md)介绍双语编辑、网站预览和 Wiki 导出。
