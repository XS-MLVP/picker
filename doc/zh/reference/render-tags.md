# 模板渲染变量

<!-- 由 scripts/check_render_tags.py 从 include/codegen/render_tags.hpp 生成，请勿手工编辑。 -->

模板由 [inja](https://github.com/pantor/inja) 渲染。本页列出代码生成模板可以使用的全部变量
（`template/` 与 `src/codegen/` 内嵌模板）；`include/codegen/render_tags.hpp` 中的
`PICKER_RENDER_TAGS` 列表是唯一声明处，`scripts/check_render_tags.py` 负责校验声明、
生产者与本页是否一致。UVM 事务模板由 `src/parser/uvm.cpp` 用自有的 `TemplateVars` 渲染，
不在此契约内。

## 约定

| 类别 | 约定 |
|---|---|
| 公共变量 | `__UPPER_SNAKE__`，模板只能引用这一类名字 |
| `bool` | 渲染为 `true` / `false` |
| `onoff` | 渲染为字符串 `ON` / `OFF` |
| `fragment` | 预渲染源码文本，模板只能注入，不能重排 |
| 内部键 | 普通 `snake_case`，不会出现在模板中 |

## 维护流程

1. 在 `include/codegen/render_tags.hpp` 的 `PICKER_RENDER_TAGS` 中增加一行。
2. 在 `src/codegen/` 中用 `data[tags::<id>]` 赋值，在 `template/` 中用 `{{__NAME__}}` 引用。
3. 运行 `python scripts/check_render_tags.py --docs` 重新生成本页。
4. 提交前运行 `python scripts/check_render_tags.py`，CI 会执行同一检查。

当前共有 58 个登记项。

## meta — 项目身份 (7)

| 变量 | 类型 | 说明 | 生产者 | 使用方 |
|---|---|---|---|---|
| `__TOP_MODULE_NAME__` | `string` | 导出顶层模块名，用于生成代码中的类名、文件名和目标名。 | `cpp`, `golang`, `java` +5 | `sv`, `cpp/CMakeLists.txt`, `cpp/Makefile` +40 |
| `__SHARED_LIB_SUFFIX__` | `string` | 平台动态库后缀（`so`、`dylib` 或 `dll`）。 | `cpp`, `golang`, `java` +2 | `cpp/Makefile`, `golang/Makefile`, `java/cmake/verilator.cmake` +4 |
| `__JNI_LIB_SUFFIX__` | `string` | JNI 库后缀；macOS 上会把 `dylib` 改写为 `jnilib`。 | `java`, `scala` | `java/dut.java`, `scala/dut.java` |
| `__LIB_DPI_FUNC_NAME_HASH__` | `string` | 每次导出生成的随机哈希，嵌入 DPI 函数名以避免多实例冲突。 | `firrtl`, `lib`, `sv` | `sv`, `lib/dut_base.cpp`, `lib/dut_base.hpp` |
| `__CONFIG_MODEL_ID__` | `string` | 生成模型及其唯一 `.xscfg` 配置数据库的身份标识。 | `lib` | `cpp/dut.cpp`, `golang/dut.go`, `java/dut.java` +3 |
| `__GENERATOR_PICKER_PATH__` | `path` | 生成 Makefile 中调用的 picker 可执行文件绝对路径。 | `lib` | `lib/Makefile` |
| `__GENERATOR_TEMPLATE_PATH__` | `path` | 本次导出使用的模板目录绝对路径。 | `lib` | `lib/CMakeLists.txt`, `lib/Makefile` |

## build — 构建与运行配置 (13)

| 变量 | 类型 | 说明 | 生产者 | 使用方 |
|---|---|---|---|---|
| `__SIMULATOR__` | `enum` | 所选模拟器：`verilator`、`vcs`、`uvs` 或 `gsim`。 | `golang`, `java`, `lib` +4 | `golang/dut.i`, `java/dut.i`, `lib/Makefile` +10 |
| `__TARGET_LANGUAGE__` | `enum` | 生成宿主工程的目标语言。 | `lib` | `lib/Makefile` |
| `__VFLAG__` | `string` | 传给生成工程构建的模拟器参数。 | `lib` | `lib/Makefile` |
| `__CFLAG__` | `string` | 传给生成工程构建的 C/C++ 编译参数。 | `lib` | `lib/Makefile` |
| `__BUILD_THREADS__` | `string` | 并行构建任务数；未指定时是探测 CPU 核数的 shell 表达式。 | `lib` | `lib/Makefile` |
| `__FILELIST__` | `string` | 生成的 RTL filelist 内容。 | `lib` | `lib/filelist.f` |
| `__VCS_CLOCK_PERIOD_HIGH__` | `string` | VCS 时钟高电平时间，单位 ps。 | `lib` | `lib/dut_base.cpp` |
| `__VCS_CLOCK_PERIOD_LOW__` | `string` | VCS 时钟低电平时间，单位 ps。 | `lib` | `lib/dut_base.cpp` |
| `__TRACE__` | `enum` | 波形格式（`vcd`、`fst`、`fsdb`、`usdb`）或 `OFF`。 | `firrtl`, `sv` | `lib/Makefile`, `lib/dut_base.cpp` |
| `__VERDI_MODE__` | `enum` | Verdi 集成模式：`legacy` 或 `modern`。 | `lib`, `python` | `lib/cmake/vcs.cmake`, `python/dut.py` |
| `__VERBOSE__` | `onoff` | 保留中间构建文件并输出详细构建信息。 | `lib` | `lib/Makefile` |
| `__EXAMPLE__` | `onoff` | 构建后生成并运行目标语言示例。 | `lib` | `lib/Makefile` |
| `__CHECKPOINTS__` | `onoff` | 启用模拟器存档/恢复（checkpoint）支持。 | `lib` | `lib/Makefile` |

## feature — 功能开关 (7)

| 变量 | 类型 | 说明 | 生产者 | 使用方 |
|---|---|---|---|---|
| `__HAS_VPI_VIEWS__` | `bool` | 是否启用 VPI；同时控制 VPI 构建开关和生成的视图接口。 | `lib` | `cpp/dut.cpp`, `cpp/dut.hpp`, `golang/dut.go` +5 |
| `__HAS_ARRAY_BINDINGS__` | `bool` | 是否至少导出了一个非压缩数组端口。 | `lib` | `golang/CMakeLists.txt`, `java/CMakeLists.txt`, `lib/CMakeLists.txt` +4 |
| `__HAS_AGGREGATE_VIEWS__` | `bool` | 是否至少导出了一个结构体或多维压缩数组端口。 | `lib` | `golang/CMakeLists.txt`, `java/CMakeLists.txt`, `lib/CMakeLists.txt` +5 |
| `__HAS_UNPACKED_ARRAYS__` | `bool` | 仅 Python 使用：内部视图缓存类型检查是否包含 `xsp.XArray`。 | `python` | `python/dut.py` |
| `__COVERAGE__` | `onoff` | 是否启用覆盖率收集。 | `lib` | `lib/Makefile`, `lib/dut_base.cpp` |
| `__COVERAGE_METRICS__` | `int` | 启用的覆盖率指标位掩码（line、cond、fsm、toggle、branch、assert）。 | `lib` | `lib/dut_base.cpp` |
| `__RW_TYPE__` | `enum` | 信号访问模式：`DPI` 或 `MEM_DIRECT`。 | `lib` | `lib/Makefile`, `lib/dut_base.cpp`, `lib/dut_base.hpp` |

## dependency — xspcomm 与工具链位置 (7)

| 变量 | 类型 | 说明 | 生产者 | 使用方 |
|---|---|---|---|---|
| `__XSPCOMM_LIB__` | `path` | xspcomm 动态库所在目录。 | `cpp`, `golang`, `java` +4 | `cpp/CMakeLists.txt`, `cpp/Makefile`, `cpp/cmake/uvs.cmake` +20 |
| `__XSPCOMM_INCLUDE__` | `path` | xspcomm C++ 头文件目录。 | `cpp`, `lib`, `mem_direct` | `cpp/CMakeLists.txt`, `cpp/Makefile`, `golang/CMakeLists.txt` +6 |
| `__XSPCOMM_JAR__` | `path` | xspcomm 的 Java/Scala jar 路径。 | `java`, `scala` | `java/Makefile`, `java/cmake/vcs.cmake`, `java/cmake/verilator.cmake` +3 |
| `__XSPCOMM_PYTHON__` | `path` | xspcomm 的 Python 包路径。 | `python` | `python/Makefile` |
| `__XSPCOMM_LUA__` | `path` | xspcomm 的 Lua 模块路径。 | `lua` | `lua/Makefile` |
| `__XSPCOMM_GOLANG__` | `path` | xspcomm 的 Go 包路径。 | `golang` | `golang/Makefile` |
| `__COPY_XSPCOMM_LIB__` | `bool` | 是否把 xspcomm 库和头文件复制到生成工程中。 | `cpp`, `golang`, `java` +3 | `cpp/Makefile`, `golang/Makefile`, `java/Makefile` +3 |

## data — 模板循环使用的结构化数据 (4)

| 变量 | 类型 | 说明 | 生产者 | 使用方 |
|---|---|---|---|---|
| `__MODULE_EXTERNAL_PINS__` | `list` | 导出的外部端口列表，包含名字、方向、位宽范围和 native 名字。 | `lib` | `lib/dut_base.cpp`, `lib/dut_base.hpp`, `lib/dut_type.hpp` |
| `__ARRAY_PORTS__` | `list` | 非压缩数组绑定列表，包含元素位宽、维度和 native 步长。 | `lib` | `lib/dut_base.cpp`, `lib/dut_base.hpp` |
| `__AGGREGATE_LAYOUTS_JSON__` | `json` | 按端口名索引的聚合类型布局 JSON 字符串（已转义为源码字面量）。 | `lib` | `cpp/dut.hpp`, `golang/dut.go`, `java/dut.java` +3 |
| `__NATIVE_TYPE_IDS__` | `list` | 复制到 mem_direct 配置数据库中的 native 类型标识列表。 | `mem_direct` | `mem_direct/gen_addr.cpp` |

## fragment — 预渲染源码片段 (16)

| 变量 | 类型 | 说明 | 生产者 | 使用方 |
|---|---|---|---|---|
| `__COMMENTS__` | `fragment` | 生成的端口清单注释块。 | `cpp` | `cpp/dut.cpp`, `cpp/dut.hpp` |
| `__LOGIC_PIN_DECLARATION__` | `fragment` | 生成的 wrapper 端口 `logic` 声明。 | `firrtl`, `sv` | `lib/top.sv` |
| `__WIRE_PIN_DECLARATION__` | `fragment` | 生成的 wrapper 端口 `wire` 声明。 | `firrtl`, `sv` | `lib/top.v` |
| `__INNER_MODULES__` | `fragment` | 生成的子模块实例化代码。 | `firrtl`, `sv` | `lib/top.sv`, `lib/top.v` |
| `__DPI_FUNCTION_EXPORT__` | `fragment` | 生成的 DPI 导出声明。 | `firrtl`, `sv` | `lib/top.sv` |
| `__DPI_FUNCTION_IMPLEMENT__` | `fragment` | 生成的 DPI 函数实现。 | `firrtl`, `sv` | `lib/top.sv` |
| `__EXTEND_SV__` | `fragment` | 生成的附加 SystemVerilog 代码，目前是 finish 钩子。 | `firrtl`, `sv` | `lib/top.sv` |
| `__SV_DUMP_WAVE__` | `fragment` | 生成的波形与覆盖率控制代码。 | `firrtl`, `sv` | `lib/top.sv` |
| `__XDATA_DECLARATION__` | `fragment` | 生成的宿主语言信号成员声明。 | `cpp`, `golang`, `java` | `cpp/dut.hpp`, `golang/dut.go`, `java/dut.java` |
| `__XDATA_INIT__` | `fragment` | 生成的信号初始化代码（构造或 ReInit）。 | `cpp`, `golang`, `java` +3 | `cpp/dut.cpp`, `golang/dut.go`, `java/dut.java` +3 |
| `__XDATA_BIND__` | `fragment` | 生成的 DPI 或 native 绑定语句。 | `cpp`, `golang`, `java` +3 | `cpp/dut.cpp`, `golang/dut.go`, `java/dut.java` +3 |
| `__XPORT_ADD__` | `fragment` | 生成的 XPort 注册语句。 | `cpp`, `golang`, `java` +3 | `cpp/dut.cpp`, `golang/dut.go`, `java/dut.java` +3 |
| `__XPORT_CASCADED_DEC__` | `fragment` | 生成的级联子端口声明。 | `cpp`, `golang`, `java` | `cpp/dut.hpp`, `golang/dut.go`, `java/dut.java` |
| `__XPORT_CASCADED_SGN__` | `fragment` | 生成的级联子端口赋值语句。 | `cpp`, `golang`, `java` +3 | `cpp/dut.cpp`, `golang/dut.go`, `java/dut.java` +3 |
| `__RENDER_VARIABLE_INFO_SUB__` | `fragment` | 生成的 native 变量偏移注册调用。 | `mem_direct` | `mem_direct/gen_addr.cpp` |
| `__RENDER_VARIABLE_INFO_SUB_DEFINE__` | `fragment` | 生成的 native 变量偏移函数声明。 | `mem_direct` | `mem_direct/gen_addr.cpp` |

## 内嵌模板变量

仅用于 `src/codegen/` 内嵌 inja 模板的变量，模板目录不引用。

| 变量 | 类型 | 说明 | 生产者 | 使用方 |
|---|---|---|---|---|
| `__WAVE_FILE_NAME__` | `string` | C++ 内嵌 SystemVerilog 模板使用的波形文件名。 | `firrtl`, `sv` | `sv` |
| `__DUMP_VAR_OPTIONS__` | `string` | 来自 `DUMPVARS_OPTION` 环境变量的额外 `$dumpvars` 参数。 | `firrtl`, `sv` | `sv` |

## 内部键

只在 C++ 内部传递的 JSON 键，不属于模板契约。

| 键 | 类型 | 说明 | 文件 |
|---|---|---|---|
| `signal_tree` | `json` | 序列化信号树，供生成配置数据库时回读。 | `firrtl`, `lib`, `sv` |
| `single_config_database` | `bool` | 标记使用单一 `.xscfg` 数据库的渲染，并抑制旧的 sidecar 文件。 | `lib` |
