# 开发与测试

在源码目录完成[安装步骤](../guides/installation.md)后，即可修改、编译和运行测试。先阅读[生成与运行原理](../explanation/architecture.md)，有助于确定改动位置。

## 源码目录

| 路径 | 内容 |
| --- | --- |
| `src/picker.cpp` | 命令行参数和 export/pack 流程 |
| `src/parser/` | RTL、事务、文件列表和信号布局解析 |
| `src/codegen/` | 整理解析结果，生成模板数据 |
| `template/lib/` | 模拟器包装和构建配置 |
| `template/python/` 等语言目录 | 生成的语言接口及构建模板 |
| `test/` | 单元测试、导出集成测试和发布脚本测试 |
| `example/` | 可运行的设计与测试示例 |
| `doc/zh/`、`doc/en/` | 中英文文档 |

修复生成代码时，在对应模板中修改，然后重新导出示例验证。`dependence/` 下的库各自维护，修改依赖时在其仓库单独提交。

## 运行测试

构建并执行 C++ 单元测试：

```sh
make unit_tests NPROC=4 BUILD_XSPCOMM_SWIG=python,java
```

按改动选择集成测试：

| 修改内容 | 测试命令 |
| --- | --- |
| Python 导出与模板 | `make -C test export_python` |
| 文件列表 | `make -C test export_filelist` |
| 复杂端口解析 | `make -C test export_complex_port` |
| 内部信号与 VPI | `make -C test export_vpi_python` |
| 事务打包 | `make -C test pack_transaction` |
| RTL 转事务 | `make -C test pack_from_rtl` |
| 发布脚本 | `python3 -m unittest discover -s test/release -v` |
| 文档 | [检查与预览](documentation.md) |

完整集成测试使用 `make -C test all`，需要对应语言和模拟器环境。测试的 preflight 会检查绑定，缺失时可能重新构建；需要保留其他构建产物时使用独立 checkout。记录实际执行的测试和工具缺失导致的跳过项。

## 提交改动

在独立分支修改并向 `master` 提交 PR。说明问题的触发方式、修改后的行为和运行过的测试。接口变更同步更新中英文参考和示例。

每个 PR 选择一个 `release:patch`、`release:minor` 或 `release:major` 标签。合并后的发布过程见[发行维护](releases.md)。问题报告需要的材料见[故障排查](../reference/troubleshooting.md)。
