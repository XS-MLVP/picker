# Picker

[![PyPI: not published](https://img.shields.io/badge/pypi-not%20published-lightgrey?style=flat-square&logo=pypi&logoColor=white)](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/guides/installation.md)
[![Python](https://img.shields.io/badge/python-3.11%20%7C%203.12-blue?style=flat-square&logo=python&logoColor=white)](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/guides/installation.md)
[![PR CI](https://img.shields.io/github/actions/workflow/status/XS-MLVP/picker/ci.yml?event=pull_request&style=flat-square&label=PR%20CI&logo=github)](https://github.com/XS-MLVP/picker/actions/workflows/ci.yml?query=event%3Apull_request)
[![License](https://img.shields.io/badge/license-MulanPSL--2.0-blue?style=flat-square)](https://github.com/XS-MLVP/picker/blob/master/LICENSE)
[![Docs](https://img.shields.io/badge/docs-Guide-0A7EA4?style=flat-square)](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/index.md)
[![GitHub Stars](https://img.shields.io/github/stars/XS-MLVP/picker?style=social)](https://github.com/XS-MLVP/picker)

中文 | [English](https://github.com/XS-MLVP/picker/blob/master/README.md)

Picker 将 Verilog/SystemVerilog 设计导出为 Python、C++、Java、Scala、Go 或 Lua 模块，方便你用熟悉的语言编写硬件测试。测试代码可以读写信号、推进时钟、检查结果和保存波形。Picker 也支持生成连接 Python 与 UVM 的事务桥。

Python 发行包名为 `uc-picker`，命令仍为 `picker`，依赖 `uc-xcomm` 和 SWIG。

## 开始使用

先[安装 Picker](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/guides/installation.md)，再跟随[加法器教程](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/tutorials/first-dut.md)完成第一次验证。在仓库根目录导出 Python 模块：

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --sim verilator --tdir build/docs-adder/Adder --autobuild false
make -C build/docs-adder/Adder -j4
```

在生成包的父目录中创建测试，就可以通过端口名给输入赋值并检查输出。教程包含完整脚本和预期结果。

## 文档

[完整中文文档](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/index.md) · [English documentation](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/index.md)

- 使用：[导出 RTL](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/guides/export.md)、[时钟与波形](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/guides/simulation.md)、[内部信号](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/guides/internal-signals.md)、[多语言](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/guides/languages.md)、[UVM](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/guides/uvm-tlm.md)。
- 查询：[命令行](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/reference/cli.md)、[DUT API](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/reference/dut-api.md)、[故障排查](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/reference/troubleshooting.md)。
- 开发：[工作原理](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/explanation/architecture.md)、[贡献与测试](https://github.com/XS-MLVP/picker/blob/master/CONTRIBUTING.md)、[文档与 Wiki](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/development/documentation.md)。

## 社区

问题和建议请提交到 [GitHub Issues](https://github.com/XS-MLVP/picker/issues)。社区 QQ 群：600480230。更多验证资料见 [MLVP 文档](https://open-verify.cc/mlvp/docs/)。

[行为准则](https://github.com/XS-MLVP/picker/blob/master/CODE_OF_CONDUCT.md) · [安全报告](https://github.com/XS-MLVP/picker/blob/master/SECURITY.md) · [许可证](https://github.com/XS-MLVP/picker/blob/master/LICENSE)
