#pragma once

#include <array>

// Every variable a codegen template may reference is declared exactly once
// here. `template/` uses the wire names (`{{__NAME__}}`); C++ writes them
// through `tags::<id>` so a typo is a compile error instead of a silently
// missing key. The UVM transaction generator in src/parser/uvm.cpp renders its
// own templates from a separate TemplateVars enum and is out of scope.
//
// `scripts/check_render_tags.py` parses the list below and verifies that
// templates, producers and the generated reference pages agree.

namespace picker::codegen::tags {

    // How the value is rendered into the template.
    enum class value_type {
        string,       // free-form text or a path
        boolean,      // JSON true / false
        onoff,        // the string "ON" or "OFF"
        integer,      // JSON number
        enumeration,  // one of a fixed set of strings
        path,         // filesystem path
        json,         // structured value consumed by a loop
        list,         // ordered collection consumed by a loop
        fragment,     // pre-rendered source text, injected verbatim
    };

    // Where the wire name is allowed to appear.
    enum class scope {
        template_tag,  // public: referenced from template/
        embedded,      // only inside inja strings embedded in src/codegen/
        internal,      // plain snake_case JSON key, never visible to templates
    };

    // Grouping used by the generated reference page.
    enum class category { meta, build, feature, dependency, data, fragment };

    struct definition {
        const char *name;
        category group;
        value_type type;
        scope where;
        const char *summary;
        const char *summary_zh;
    };

// id, wire name, category, type, scope, English summary, Chinese summary
#define PICKER_RENDER_TAGS(X) \
    X(top_module_name, "__TOP_MODULE_NAME__", meta, string, template_tag, \
      "Name of the exported top module; used for generated class, file and target names.", \
      "导出顶层模块名，用于生成代码中的类名、文件名和目标名。") \
    X(shared_lib_suffix, "__SHARED_LIB_SUFFIX__", meta, string, template_tag, \
      "Platform shared-library suffix (`so`, `dylib` or `dll`).", \
      "平台动态库后缀（`so`、`dylib` 或 `dll`）。") \
    X(jni_lib_suffix, "__JNI_LIB_SUFFIX__", meta, string, template_tag, \
      "JNI library suffix; `dylib` is rewritten to `jnilib` on macOS.", \
      "JNI 库后缀；macOS 上会把 `dylib` 改写为 `jnilib`。") \
    X(lib_dpi_func_name_hash, "__LIB_DPI_FUNC_NAME_HASH__", meta, string, template_tag, \
      "Per-export hash embedded in generated DPI function names.", \
      "每次导出生成的随机哈希，嵌入 DPI 函数名以避免多实例冲突。") \
    X(config_model_id, "__CONFIG_MODEL_ID__", meta, string, template_tag, \
      "Identity of the generated model and its single `.xscfg` database.", \
      "生成模型及其唯一 `.xscfg` 配置数据库的身份标识。") \
    X(generator_picker_path, "__GENERATOR_PICKER_PATH__", meta, path, template_tag, \
      "Absolute path of the picker executable used by the generated Makefile.", \
      "生成 Makefile 中调用的 picker 可执行文件绝对路径。") \
    X(generator_template_path, "__GENERATOR_TEMPLATE_PATH__", meta, path, template_tag, \
      "Absolute path of the template directory used for this export.", \
      "本次导出使用的模板目录绝对路径。") \
    X(simulator, "__SIMULATOR__", build, enumeration, template_tag, \
      "Selected simulator: `verilator`, `vcs`, `uvs` or `gsim`.", \
      "所选模拟器：`verilator`、`vcs`、`uvs` 或 `gsim`。") \
    X(target_language, "__TARGET_LANGUAGE__", build, enumeration, template_tag, \
      "Language of the generated host project.", \
      "生成宿主工程的目标语言。") \
    X(vflag, "__VFLAG__", build, string, template_tag, \
      "Simulator flags forwarded to the generated build.", \
      "传给生成工程构建的模拟器参数。") \
    X(cflag, "__CFLAG__", build, string, template_tag, \
      "C/C++ compiler flags forwarded to the generated build.", \
      "传给生成工程构建的 C/C++ 编译参数。") \
    X(build_threads, "__BUILD_THREADS__", build, string, template_tag, \
      "Parallel build job count, or a shell expression when the option is unset.", \
      "并行构建任务数；未指定时是探测 CPU 核数的 shell 表达式。") \
    X(filelist, "__FILELIST__", build, string, template_tag, \
      "Content of the generated RTL filelist.", \
      "生成的 RTL filelist 内容。") \
    X(vcs_clock_period_high, "__VCS_CLOCK_PERIOD_HIGH__", build, string, template_tag, \
      "VCS clock high period in picoseconds.", \
      "VCS 时钟高电平时间，单位 ps。") \
    X(vcs_clock_period_low, "__VCS_CLOCK_PERIOD_LOW__", build, string, template_tag, \
      "VCS clock low period in picoseconds.", \
      "VCS 时钟低电平时间，单位 ps。") \
    X(trace, "__TRACE__", build, enumeration, template_tag, \
      "Waveform format (`vcd`, `fst`, `fsdb`, `usdb`) or `OFF`.", \
      "波形格式（`vcd`、`fst`、`fsdb`、`usdb`）或 `OFF`。") \
    X(verdi_mode, "__VERDI_MODE__", build, enumeration, template_tag, \
      "Verdi integration mode: `legacy` or `modern`.", \
      "Verdi 集成模式：`legacy` 或 `modern`。") \
    X(verbose, "__VERBOSE__", build, onoff, template_tag, \
      "Keep intermediate build files and print build details.", \
      "保留中间构建文件并输出详细构建信息。") \
    X(example, "__EXAMPLE__", build, onoff, template_tag, \
      "Generate and run the language example after building.", \
      "构建后生成并运行目标语言示例。") \
    X(checkpoints, "__CHECKPOINTS__", build, onoff, template_tag, \
      "Enable simulator save/restore support.", \
      "启用模拟器存档/恢复（checkpoint）支持。") \
    X(has_vpi_views, "__HAS_VPI_VIEWS__", feature, boolean, template_tag, \
      "VPI is enabled; gates VPI build flags and generated view APIs.", \
      "是否启用 VPI；同时控制 VPI 构建开关和生成的视图接口。") \
    X(has_array_bindings, "__HAS_ARRAY_BINDINGS__", feature, boolean, template_tag, \
      "At least one unpacked array port is exported.", \
      "是否至少导出了一个非压缩数组端口。") \
    X(has_aggregate_views, "__HAS_AGGREGATE_VIEWS__", feature, boolean, template_tag, \
      "At least one struct or packed aggregate port is exported.", \
      "是否至少导出了一个结构体或多维压缩数组端口。") \
    X(has_unpacked_arrays, "__HAS_UNPACKED_ARRAYS__", feature, boolean, template_tag, \
      "Python only: include `xsp.XArray` in the internal view cache type check.", \
      "仅 Python 使用：内部视图缓存类型检查是否包含 `xsp.XArray`。") \
    X(coverage, "__COVERAGE__", feature, onoff, template_tag, \
      "Coverage collection is enabled.", \
      "是否启用覆盖率收集。") \
    X(coverage_metrics, "__COVERAGE_METRICS__", feature, integer, template_tag, \
      "Bitmask of enabled coverage metrics (line, cond, fsm, toggle, branch, assert).", \
      "启用的覆盖率指标位掩码（line、cond、fsm、toggle、branch、assert）。") \
    X(rw_type, "__RW_TYPE__", feature, enumeration, template_tag, \
      "Signal access mode: `DPI` or `MEM_DIRECT`.", \
      "信号访问模式：`DPI` 或 `MEM_DIRECT`。") \
    X(xspcomm_lib, "__XSPCOMM_LIB__", dependency, path, template_tag, \
      "Directory containing the xspcomm shared library.", \
      "xspcomm 动态库所在目录。") \
    X(xspcomm_include, "__XSPCOMM_INCLUDE__", dependency, path, template_tag, \
      "xspcomm C++ include directory.", \
      "xspcomm C++ 头文件目录。") \
    X(xspcomm_jar, "__XSPCOMM_JAR__", dependency, path, template_tag, \
      "xspcomm Java/Scala jar path.", \
      "xspcomm 的 Java/Scala jar 路径。") \
    X(xspcomm_python, "__XSPCOMM_PYTHON__", dependency, path, template_tag, \
      "xspcomm Python package path.", \
      "xspcomm 的 Python 包路径。") \
    X(xspcomm_lua, "__XSPCOMM_LUA__", dependency, path, template_tag, \
      "xspcomm Lua module path.", \
      "xspcomm 的 Lua 模块路径。") \
    X(xspcomm_golang, "__XSPCOMM_GOLANG__", dependency, path, template_tag, \
      "xspcomm Go package path.", \
      "xspcomm 的 Go 包路径。") \
    X(copy_xspcomm_lib, "__COPY_XSPCOMM_LIB__", dependency, boolean, template_tag, \
      "Copy the xspcomm library and headers into the generated project.", \
      "是否把 xspcomm 库和头文件复制到生成工程中。") \
    X(module_external_pins, "__MODULE_EXTERNAL_PINS__", data, list, template_tag, \
      "Exported external pins with name, direction, bounds and native name.", \
      "导出的外部端口列表，包含名字、方向、位宽范围和 native 名字。") \
    X(array_ports, "__ARRAY_PORTS__", data, list, template_tag, \
      "Unpacked array bindings with element width, dimensions and native strides.", \
      "非压缩数组绑定列表，包含元素位宽、维度和 native 步长。") \
    X(aggregate_layouts_json, "__AGGREGATE_LAYOUTS_JSON__", data, json, template_tag, \
      "Escaped JSON string of aggregate type layouts keyed by port name.", \
      "按端口名索引的聚合类型布局 JSON 字符串（已转义为源码字面量）。") \
    X(native_type_ids, "__NATIVE_TYPE_IDS__", data, list, template_tag, \
      "Native type identifiers copied into the mem_direct configuration database.", \
      "复制到 mem_direct 配置数据库中的 native 类型标识列表。") \
    X(comments, "__COMMENTS__", fragment, fragment, template_tag, \
      "Generated comment block listing exported ports.", \
      "生成的端口清单注释块。") \
    X(logic_pin_declaration, "__LOGIC_PIN_DECLARATION__", fragment, fragment, template_tag, \
      "Generated `logic` declarations for wrapper ports.", \
      "生成的 wrapper 端口 `logic` 声明。") \
    X(wire_pin_declaration, "__WIRE_PIN_DECLARATION__", fragment, fragment, template_tag, \
      "Generated `wire` declarations for wrapper ports.", \
      "生成的 wrapper 端口 `wire` 声明。") \
    X(inner_modules, "__INNER_MODULES__", fragment, fragment, template_tag, \
      "Generated submodule instances.", \
      "生成的子模块实例化代码。") \
    X(dpi_function_export, "__DPI_FUNCTION_EXPORT__", fragment, fragment, template_tag, \
      "Generated DPI export declarations.", \
      "生成的 DPI 导出声明。") \
    X(dpi_function_implement, "__DPI_FUNCTION_IMPLEMENT__", fragment, fragment, template_tag, \
      "Generated DPI function bodies.", \
      "生成的 DPI 函数实现。") \
    X(extend_sv, "__EXTEND_SV__", fragment, fragment, template_tag, \
      "Generated extra SystemVerilog, currently the finish hook.", \
      "生成的附加 SystemVerilog 代码，目前是 finish 钩子。") \
    X(sv_dump_wave, "__SV_DUMP_WAVE__", fragment, fragment, template_tag, \
      "Generated waveform and coverage control code.", \
      "生成的波形与覆盖率控制代码。") \
    X(xdata_declaration, "__XDATA_DECLARATION__", fragment, fragment, template_tag, \
      "Generated host-language signal member declarations.", \
      "生成的宿主语言信号成员声明。") \
    X(xdata_init, "__XDATA_INIT__", fragment, fragment, template_tag, \
      "Generated signal initialization (construction or ReInit).", \
      "生成的信号初始化代码（构造或 ReInit）。") \
    X(xdata_bind, "__XDATA_BIND__", fragment, fragment, template_tag, \
      "Generated DPI or native binding statements.", \
      "生成的 DPI 或 native 绑定语句。") \
    X(xport_add, "__XPORT_ADD__", fragment, fragment, template_tag, \
      "Generated XPort registration statements.", \
      "生成的 XPort 注册语句。") \
    X(xport_cascaded_dec, "__XPORT_CASCADED_DEC__", fragment, fragment, template_tag, \
      "Generated cascaded sub-port declarations.", \
      "生成的级联子端口声明。") \
    X(xport_cascaded_sgn, "__XPORT_CASCADED_SGN__", fragment, fragment, template_tag, \
      "Generated cascaded sub-port assignments.", \
      "生成的级联子端口赋值语句。") \
    X(render_variable_info_sub, "__RENDER_VARIABLE_INFO_SUB__", fragment, fragment, template_tag, \
      "Generated calls that register native variable offsets.", \
      "生成的 native 变量偏移注册调用。") \
    X(render_variable_info_sub_define, "__RENDER_VARIABLE_INFO_SUB_DEFINE__", fragment, fragment, template_tag, \
      "Generated declarations for native variable offset functions.", \
      "生成的 native 变量偏移函数声明。") \
    X(wave_file_name, "__WAVE_FILE_NAME__", build, string, embedded, \
      "Waveform file name used by the SystemVerilog templates embedded in C++.", \
      "C++ 内嵌 SystemVerilog 模板使用的波形文件名。") \
    X(dump_var_options, "__DUMP_VAR_OPTIONS__", build, string, embedded, \
      "Extra `$dumpvars` arguments from the `DUMPVARS_OPTION` environment variable.", \
      "来自 `DUMPVARS_OPTION` 环境变量的额外 `$dumpvars` 参数。") \
    X(signal_tree, "signal_tree", meta, json, internal, \
      "Serialized signal tree read back to populate the configuration database.", \
      "序列化信号树，供生成配置数据库时回读。") \
    X(single_config_database, "single_config_database", meta, boolean, internal, \
      "Marks renders that use the single `.xscfg` database and suppress legacy sidecars.", \
      "标记使用单一 `.xscfg` 数据库的渲染，并抑制旧的 sidecar 文件。")

    // Typed names used by producers: data[tags::top_module_name] = ...;
#define PICKER_DECLARE_TAG(id, wire, ...) inline constexpr auto id = wire;
    PICKER_RENDER_TAGS(PICKER_DECLARE_TAG)
#undef PICKER_DECLARE_TAG

    // Metadata consumed by scripts/check_render_tags.py.
#define PICKER_DECLARE_ROW(id, wire, group, type, where, summary, summary_zh) \
    definition{wire, category::group, value_type::type, scope::where, summary, summary_zh},
    inline constexpr std::array definitions{
    PICKER_RENDER_TAGS(PICKER_DECLARE_ROW)
    };
#undef PICKER_DECLARE_ROW
#undef PICKER_RENDER_TAGS

} // namespace picker::codegen::tags
