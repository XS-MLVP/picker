# Picker tests / Picker 测试

[English testing guide](../doc/en/development/contributing.md) | [中文测试指南](../doc/zh/development/contributing.md)

These scripts check the CLI and export/pack flows. Verilator and Python are the default integration path. Prepare a built Picker, CMake, a C++20 compiler, SWIG and matching Python development tools; see [installation](../doc/en/guides/installation.md).

这里验证 CLI 和 export/pack 流程，默认集成为 Verilator + Python。先准备 Picker、CMake、C++20 编译器、SWIG 和匹配的 Python 开发环境，见[安装](../doc/zh/guides/installation.md)。

| Command / 命令 | Scope / 范围 |
| --- | --- |
| `python3 test/scripts/test_python_simulator_metadata.py` | VCS/Verilator metadata generation; override binary with `PICKER_BIN`; no simulator license required / 仿真器元数据生成 |
| `python3 test/scripts/test_vcs_coverage_runtime.py` | Native VCS coverage, FSDB, relocation and post-Finish execution / VCS 原生覆盖率与结束行为 |
| `make -C test all` | Integration suite / 集成测试 |
| `make -C test smoke` | CLI smoke, without DUT builds / CLI 冒烟，不构建 DUT |
| `make -C test export_python` | Python Adder export / Python 加法器 |
| `make -C test export_vpi_python` | VPI export / VPI 导出 |
| `make -C test export_filelist` | File lists / 文件列表 |
| `make -C test export_complex_port` | Complex ports / 复杂端口 |
| `make -C test pack_transaction` | UVM transactions / UVM 事务 |
| `make -C test pack_from_rtl` | Transactions from RTL / 从 RTL 生成事务 |
| `make -C test pack_unsupported_type` | Unsupported types / 不支持的类型 |
| `make -C test pack_multiple_classes` | Multiple classes / 多类事务 |
| `make -C test pack_python` | Python packaging / Python 包装 |

Tests write generated directories such as picker_out_* and pack_example. `make -C test clean` removes test outputs. The preflight may rebuild missing bindings; use a separate checkout when preserving other build artifacts.

测试会生成 picker_out_*、pack_example 等目录；`make -C test clean` 清理测试产物。preflight 可能重建缺失绑定；需保留其他构建产物时使用独立 checkout。

For the VCS runtime regression, set `PICKER_VCS_DUT_DIR` to a freshly compiled coverage-enabled Adder package and run in the VCS environment. The test checks two independent snapshots and standalone helper queries. Metadata tests use temporary output directories.
