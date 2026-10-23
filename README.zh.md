# Picker

中文 | [English](README.md)

Picker 将 Verilog/SystemVerilog 设计导出为 Python、C++、Java、Scala、Go 或 Lua 模块，方便你用熟悉的语言编写硬件测试。测试代码可以读写信号、推进时钟、检查结果和保存波形。Picker 也支持生成连接 Python 与 UVM 的事务桥。

Python 发行包名为 `uc-picker`，命令仍为 `picker`，依赖 `uc-xcomm` 和 SWIG。

## 开始使用

先[安装 Picker](doc/zh/guides/installation.md)，再跟随[加法器教程](doc/zh/tutorials/first-dut.md)完成第一次验证。在仓库根目录导出 Python 模块：

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --sim verilator --tdir build/docs-adder/Adder --autobuild false
make -C build/docs-adder/Adder -j4
```

在生成包的父目录中创建测试，就可以通过端口名给输入赋值并检查输出。教程包含完整脚本和预期结果。

## 文档

[完整中文文档](doc/zh/index.md) · [English documentation](doc/en/index.md)

- 使用：[导出 RTL](doc/zh/guides/export.md)、[时钟与波形](doc/zh/guides/simulation.md)、[内部信号](doc/zh/guides/internal-signals.md)、[多语言](doc/zh/guides/languages.md)、[UVM](doc/zh/guides/uvm-tlm.md)。
- 查询：[命令行](doc/zh/reference/cli.md)、[DUT API](doc/zh/reference/dut-api.md)、[故障排查](doc/zh/reference/troubleshooting.md)。
- 开发：[工作原理](doc/zh/explanation/architecture.md)、[贡献与测试](CONTRIBUTING.md)、[文档与 Wiki](doc/zh/development/documentation.md)。

## 社区

问题和建议请提交到 [GitHub Issues](https://github.com/XS-MLVP/picker/issues)。社区 QQ 群：600480230。更多验证资料见 [MLVP 文档](https://open-verify.cc/mlvp/docs/)。

[行为准则](CODE_OF_CONDUCT.md) · [安全报告](SECURITY.md) · [许可证](LICENSE)
