# 命令行参考

使用 `picker --help`、`picker export --help` 和 `picker pack --help` 查看命令帮助。以下按全局选项、RTL 导出和 UVM 打包列出参数。

## 全局命令

| 选项 | 作用 |
| --- | --- |
| `-h, --help` | 显示帮助 |
| `-v, --version` | 显示构建版本 |
| `--check` | 检查模板与各语言资源，阅读每一项结果 |
| `--show_default_template_path` | 输出模板路径 |
| `--show_xcom_lib_location_cpp` | 输出 C++ 运行库的 include/lib 路径 |
| `--show_xcom_lib_location_python/java/scala/golang/lua` | 查询对应语言资源；实际使用一个完整选项名称 |

## export

| 选项 | 参数/默认值 | 用途 |
| --- | --- | --- |
| `file` | RTL 文件，可多个 | 提供源文件；没有 `--sname` 时需要 |
| `--fs, --filelist` | 文件或列表，可多个 | 补充源文件、包含路径和宏定义 |
| `--sname, --source_module_name` | module 名，可多个 | 指定顶层选择 |
| `--tname, --target_module_name` | 名称 | 指定生成模块名，默认源模块名 |
| `--tdir, --target_dir` | 路径 | 输出目录；末尾 `/` 表示在其下按模块名建目录 |
| `--sdir, --source_dir` | 模板目录 | 覆盖安装模板来源 |
| `--lang, --language` | `python` | cpp/python/java/scala/golang/lua |
| `--sim` | `verilator` | verilator/vcs/gsim/uvs；依赖对应工具链 |
| `--rw, --access-mode` | `dpi` | dpi 或 mem_direct；支持范围见访问指南 |
| `--internal` | 配置文件 | 选定内部信号 |
| `--vpi` | flag，关闭 | 编入 VPI 支持 |
| `--checkpoints` | flag，关闭 | 编入受支持的保存/恢复能力 |
| `-w, --wave_file_name` | 文件名，空 | 配置波形格式/输出 |
| `-c, --coverage` | flag，关闭 | 编入模拟器覆盖率 |
| `-F, --frequency` | `100MHz` | VCS 时间配置，支持 Hz/KHz/MHz/GHz 单位 |
| `--verdi-mode` | `legacy` | VCS Verdi 集成；modern 对应较新的调试方式 |
| `--cp_lib, --copy_xspcomm_lib` | `true` | 复制配套运行库 |
| `-V, --vflag` | 字符串 | 传给模拟器的编译参数 |
| `-S, --sflag` | 字符串列表 | 传给 slang 的解析参数 |
| `-C, --cflag` | 字符串 | 传给 C++ 编译器的参数 |
| `-e, --example` | flag，关闭 | 生成/构建示例 |
| `--autobuild` | `true` | 生成后自动构建 |
| `-j, --build-threads` | 0 | 并行度；0 使用所有核心 |
| `--verbose` | flag，关闭 | 增加诊断输出 |

含空格的参数需要 shell 引号，例如 `-V '--trace -Wno-fatal'`。各模拟器的工具环境见[模拟器支持](compatibility.md)。

## pack

| 选项 | 用途 |
| --- | --- |
| `file` | 含事务声明的 SV 文件 |
| `-f, --filelist` | 事务文件列表 |
| `--from-rtl` | 从 RTL 端口生成事务 |
| `-d, --generate-dut` | 生成支持逐引脚读写的 DUT 包装 |
| `-r, --rename` | 重命名事务 |
| `-n, --name` | 生成包名 |
| `-e, --example` | 生成示例工程 |
| `-c, --force` | 删除已生成目录并重新生成 |

生成目录和使用示例见 [UVM 事务桥指南](../guides/uvm-tlm.md)。

## 环境变量

`DUMPVARS_OPTION` 传给 VCS 的 `$fsdbDumpvars` 选项，例如：

```sh
DUMPVARS_OPTION="+mda" picker export rtl/top.sv --sname Top --sim vcs -w trace.fsdb
```

该片段需要实际 RTL 和配置好的 VCS/Verdi 环境；`+mda` 用于相应环境中的数组波形。
