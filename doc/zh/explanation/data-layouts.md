# XData 类型布局与显式视图

**适用版本：本分支的原生 ABI 6 源码及匹配 SDK。** `XData` 关联配置中的类型布局，用户通过 `ArrayView()` / `StructView()` 显式取得结构化视图。具体操作见[导出工程](../guides/export.md)和[内部信号](../guides/internal-signals.md)。

## Data、Array、Struct 与 packed

`XData` 表示一个固定宽度的完整值，普通位索引继续选择该值中的位。`XArray` 表示数组容器，索引选择当前维度的元素。`XStruct` 表示具名字段容器。

packed 与 unpacked 描述 RTL 类型的组合方式。packed array 和 packed struct 都可以作为一个完整 `XData` 访问，也可以显式请求结构化视图。unpacked 容器的字段或元素具有独立存储。

| RTL 类型 | 默认信号表示 | 结构化访问 |
| --- | --- | --- |
| 普通 packed 位向量 | `XData` | 保持现有整值和位访问 |
| packed array | 完整 `XData` | 显式请求数组视图 |
| packed struct | 完整 `XData` | 显式请求 struct 视图 |
| unpacked array | `XArray` | 按声明边界选择元素 |
| unpacked struct | `XStruct` | 按字段名选择成员 |

例如：

```systemverilog
logic [1:0][7:4] matrix;
logic [1:0][7:4] mixed [3:2];
```

`matrix` 默认是完整的 8 位 `XData`。`mixed` 是 unpacked `XArray`，`mixed[3]` 默认是完整的 8 位 `XData`。普通位索引选择这个 Data 中的位；通过 `ArrayView()` 按 `[1:0][7:4]` 选择数组元素。显式视图中的数组或字段选择由其布局决定。

普通一维位向量同样保留可用的声明范围。例如 `[7:4]` 的 Data 默认仍按位偏移访问，但
显式 `ArrayView()` 可使用 RTL 下标 7 到 4。VPI 发现的一维向量保留其声明范围，默认种类为 `bits`。

## 三种坐标

| 坐标 | 含义 | 本次要求 |
| --- | --- | --- |
| `XData` 位偏移 | 最低有效位为 0 | 保持现有语义 |
| 显式 RTL 数组视图下标 | 声明中的 left/right，含升降序和负下标 | 保留声明并检查边界 |
| 旧原生数组存储坐标 | `NewXArray()` 等旧入口的零基存储位置 | 保持零基存储坐标 |

对于 `logic [7:4] word`，RTL 下标 7 与 Data 位偏移 3 指向同一位。`word[3]` 选择位偏移 3，`word.ArrayView()[7]` 使用 RTL 下标。

## 当前已经实现的部分

配置生成与运行时视图使用以下类型和存储信息：

1. Slang 已采集固定聚合类型、字段、边界、signedness 和四态属性。
2. `.xscfg` 的 `ports` 元数据保存端口 `Layout`；原生信号通过 `layout_id` 引用类型表。
3. `XSignalCFG.NewXView(name)` 已能按原生布局解析类型、核对存储并创建视图。
4. `XData.ArrayView(layout)`、`StructView(layout)` 已能以外部布局解释现有 packed 值，并共享其存储。
5. packed 字段切片、unpacked 独立字段绑定及编译器核验的 native offset/stride 已存在。

`NewXData()` 将已核验的类型关联到返回的 Data；端口封装在创建、绑定并登记 Data 后调用 `BindDataLayouts()`。`port_types` 将精确端口名映射到去重类型表的 ID，unpacked 容器通过 `NewViewFromPort()` 使用同一类型图和原有叶子。

`XPort` 是命名 Data 的 group。`NewViewFromPort(group, name)` 从配置取得指定 RTL 对象的结构，并从 group 按名字查找已有 Data。`BindDataLayouts(group)` 为这些 Data 关联类型。

## 调用方式

下面的片段假定已用匹配的 ABI 6 SDK 导出并初始化具有相应 packed 端口的 DUT：

```python
dut.matrix.value = 0xA5
matrix = dut.matrix.ArrayView()
packet = dut.packet.StructView()
```

无参形式使用 Data 已关联的类型。手工 Data 可用 `SetLayout(layout)` 显式关联类型，或使用带 `layout` 的视图重载；`HasLayout()` 检查关联是否存在。视图请求时核验来源、类型和位宽。

## 类型信息如何到达 Data

### 配置层提供类型

xcfg 负责按需查找、校验和复用类型描述。生成 `.xscfg` 时保存元数据，运行时在实际信号创建或绑定时建立关联。同类型共享只读描述，native 基址、padding、offset、stride 由已核验的存储绑定提供。

### 两条创建路径都要接通

- **端口**：封装创建并绑定 DPI/native Data，随后通过配置关联已有类型。
- **内部信号**：通过 `xcfg.NewXData()` 或递归视图按需创建 Data 时，关联选中节点的类型。

关联使用 RTL 逻辑名或明确的类型引用，DPI 和 mem_direct 工程均可查询端口类型配置。

### 字段、元素、重初始化和生命周期

通过布局选择字段或数组元素得到的 Data 保留对应子类型。复制 Data 时分别处理类型与后端。`ReInit()` 清除关联类型及视图缓存，来源 Data 的生命周期检查应用于视图访问。

## 实现位置

| 层次 | 主要位置 | 修改内容 |
| --- | --- | --- |
| xcomm Data | `include/xspcomm/xdata.h`、`src/data/data.cpp` | 可选的共享类型关联；初始化、复制、重初始化和失效规则 |
| xcomm 视图 | `src/data/view.cpp` | 复用类型描述，增加使用自身布局的显式入口，传播子类型 |
| xcomm 配置 | `src/core/signal_cfg.cpp` | 复用已有类型解析，把类型关联到创建或现有的 Data；保持按需查询 |
| 语言绑定 | `swig/xcomm.i`、`swig/view_raw.i`、`swig/python/xcomm.py` | 六语言暴露一致入口；检查嵌套 packed 节点转换与句柄生命周期 |
| Picker 生成器 | `src/codegen/array.cpp`、`names.cpp`、各语言生成器和模板 | 关联端口类型，移除 packed `*_view` 成员及其专用命名和登记 |
| Picker 类型 | `src/parser/sv.cpp` | 统一端口与原生显式视图的布局解释 |
| 测试与文档 | 两仓库的单元、绑定、真实 RTL 测试和双语手册 | 验证新入口与迁移用法 |

`XData` 的共享类型与视图缓存成员改变了原生对象布局。本分支使用 **ABI 6**，动态库 SONAME 为 `libxspcomm.so.6`，Python 依赖为 `uc-xcomm>=1.6,<2`。使用匹配的 SDK 重新导出并编译 DUT。

## 验证与性能测量

ABI 6 的真实聚合 fixture 在 Verilator 5.052 上验证六语言的 DPI、mem_direct 与 VPI 路径，
Scala 使用 2.13.16。普通向量的非零/升序边界、嵌套 packed/unpacked 元素与端口/native 的
显式视图一致性另有 Python 回归。

性能微测使用同一份含 **711,754 条合成、共享 32 位 struct 类型记录**的 `.xscfg`，对比
ABI 5 基线与 ABI 6，五次独立进程取中位数。被查询的值均实际绑定并读取，后续阶段保留 4,096
个 Data。冷缓存测量使用 `POSIX_FADV_DONTNEED` 建议。

| 指标 | ABI 5 基线 | ABI 6 类型关联 |
| --- | --- | --- |
| 本机 `sizeof(XData)` | 584 字节 | 616 字节 |
| 热缓存初始化至首次成功读值 | 0.375 ms | 0.522 ms |
| 建议冷缓存初始化至首次成功读值 | 5.26 ms | 8.01 ms |
| 重复创建同名 Data 并读值 | 1.13 µs/次 | 1.21 µs/次 |
| 同一 Data 重复取得 struct 字段并读值 | 14.67 µs/次 | 0.144 µs/次 |
| 读取并保留 4,096 个 Data 后的进程 RSS | 33.8 MiB | 34.0 MiB |
| 再取得并保留相应视图、字段后的进程 RSS | 44.8 MiB | 42.8 MiB |

基线重复视图使用带 JSON 布局的旧入口，新入口使用先前关联的类型及缓存。

[导出指南](../guides/export.md)、[内部信号指南](../guides/internal-signals.md)和[DUT API](../reference/dut-api.md)提供实际用法。外部消费者从自动 `*_view` 成员迁移到显式入口；配置库的 `Metadata("ports")` 入口保持可用。
