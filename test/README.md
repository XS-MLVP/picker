# Picker tests / Picker 测试

[English testing guide](../doc/en/development/contributing.md) | [中文测试指南](../doc/zh/development/contributing.md)

These scripts check the CLI and export/pack flows. Verilator and Python are the default integration path. Prepare a built Picker, CMake, a C++20 compiler, SWIG and matching Python development tools; see [installation](../doc/en/guides/installation.md).

这里验证 CLI 和 export/pack 流程，默认集成为 Verilator + Python。先准备 Picker、CMake、C++20 编译器、SWIG 和匹配的 Python 开发环境，见[安装](../doc/zh/guides/installation.md)。

| Command / 命令 | Scope / 范围 |
| --- | --- |
| `python3 test/scripts/test_python_simulator_metadata.py` | VCS/Verilator metadata generation; override binary with `PICKER_BIN` / 仿真器元数据生成 |
| `python3 test/scripts/test_config_database.py -v` | One XSCFG artifact and read-only JSON inspection / 单配置库交付与只读查看 |
| `python3 test/scripts/test_codegen_review.py -v` | Reserved member collisions and C++ packed/vector inout in DPI/mem_direct; `REVIEW_EXTRA_LANGUAGES=1` adds Go/Lua / 保留名冲突和 C++ inout 生成回归 |
| `python3 test/scripts/test_vcs_coverage_runtime.py` | Native VCS coverage, FSDB, relocation and post-Finish execution / VCS 原生覆盖率与结束行为 |
| `make -C test all` | Integration suite / 集成测试 |
| `make -C test smoke` | CLI smoke / CLI 冒烟 |
| `make -C test export_python` | Python Adder export / Python 加法器 |
| `make -C test export_vpi_python` | VPI export / VPI 导出 |
| `make -C test export_filelist` | File lists / 文件列表 |
| `make -C test export_complex_port` | Complex ports / 复杂端口 |
| `make -C test export_rtl_params` | RTL parameter and macro overrides: wrapper pinning, macro forwarding and precedence / RTL 参数与宏覆盖 |
| `make -C test export_unpacked_array` | Port arrays and native internal storage: Python/C++ DPI/mem_direct, optional Java / 端口数组与内部原生存储 |
| `python3 test/scripts/test_export_native_views.py -v` | Single-file XSCFG packaging and Slang/native alignment, shared slices, nested members and RTL/storage indices / 单配置库及通用内部类型与原生存储对齐 |
| `make -C test pack_transaction` | UVM transactions / UVM 事务 |
| `make -C test pack_from_rtl` | Transactions from RTL / 从 RTL 生成事务 |
| `make -C test pack_unsupported_type` | Transaction type validation / 事务类型校验 |
| `make -C test pack_multiple_classes` | Multiple classes / 多类事务 |
| `make -C test pack_python` | Python packaging / Python 包装 |

Python and C++ run full RTL regressions for array ports in both DPI and mem_direct
modes, and for native internal arrays through mem_direct; Java runs the port-array
regression when its toolchain is available. Set `ARRAY_EXTRA_LANGUAGES=1` to add
Go and Lua port-array RTL regressions in both access modes. All arrays use the
same recursive xcomm interface.

Python 和 C++ 会对 DPI、mem_direct 两种模式的数组端口执行完整 RTL 回归，并通过
mem_direct 覆盖内部原生数组；Java 工具链可用时运行数组端口回归。设置
`ARRAY_EXTRA_LANGUAGES=1` 可增加 Go 和 Lua 两种访问模式的端口数组 RTL 回归。
各语言数组使用相同的 xcomm 递归接口。

Tests write generated directories such as picker_out_* and pack_example. `make -C test clean` removes test outputs. Preflight builds bindings as needed; use a separate checkout when preserving other build artifacts.

测试会生成 picker_out_*、pack_example 等目录；`make -C test clean` 清理测试产物。preflight 按需构建绑定；需保留其他构建产物时使用独立 checkout。

For the VCS runtime regression, set `PICKER_VCS_DUT_DIR` to a freshly compiled coverage-enabled Adder package and run in the VCS environment. The test checks two independent snapshots and standalone helper queries. Metadata tests use temporary output directories.
