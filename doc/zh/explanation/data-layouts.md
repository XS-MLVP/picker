# XData 类型布局与显式视图

**适用版本：本分支的原生 ABI 6 源码及匹配 SDK。** 本页说明已实现的类型关联与显式视图接口；旧 ABI 5 SDK 不支持无参 `ArrayView()` / `StructView()`，需要重编配套 SDK 和 DUT。具体操作见[导出工程](../guides/export.md)和[内部信号](../guides/internal-signals.md)，语言和模拟器能力仍以实际回归范围为准。

## 要解决什么问题

原先生成器对 packed 聚合保留一个完整 `XData`，又自动增加一个 `*_view` 成员。例如 packed 数组 `matrix` 会同时得到 `dut.matrix` 和 `dut.matrix_view`，packed struct `packet` 会同时得到 `dut.packet` 和 `dut.packet_view`。它们共享信号存储，但增加了公开名称和初始化对象。

原先配置库已经有类型布局，`XData` 却没有关联这份布局，显式调用 `ArrayView(layout)`、`StructView(layout)` 仍要求外部提供布局。这不应成为用户需要重复描述 RTL 类型的理由。

本次目标是：**让实际的 `XData` 关联已有类型布局；用户显式请求结构化访问时再取得视图，不自动增加 packed `*_view` 成员。**

## Data、Array、Struct 与 packed

`XData` 表示一个固定宽度的完整值，普通位索引继续选择该值中的位。`XArray` 表示数组容器，索引选择当前维度的元素。`XStruct` 表示具名字段容器。

packed 与 unpacked 是 RTL 类型的组合方式，不是 Array 与 Data 的同义词。packed array 和 packed struct 都可以作为一个完整 `XData` 访问，也可以在明确请求后按其结构访问。unpacked 容器的字段或元素具有独立存储，不能当成一个连续 packed 值。

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

`matrix` 默认是完整的 8 位 `XData`。`mixed` 是 unpacked `XArray`，`mixed[3]` 默认是完整的 8 位 `XData`，不是自动展开的 packed 子数组。继续对这个 Data 做普通位索引仍是位选择；要按 `[1:0][7:4]` 解释，必须显式请求数组视图。

进入显式视图后，数组或字段选择由该视图的布局决定。不能在 packed/unpacked 交界处，仅因一个布尔属性就丢弃类型或改变普通索引的含义。嵌套 packed struct 也应遵循显式解释原则，不能只对 packed array 要求显式操作。

普通一维位向量同样保留可用的声明范围。例如 `[7:4]` 的 Data 默认仍按位偏移访问，但
显式 `ArrayView()` 可使用 RTL 下标 7 到 4。VPI 发现一维向量时保留其范围而不改变默认
`bits` 种类；没有完整范围信息时明确拒绝无参数组解释，不能仅凭宽度编造下标。

## 不混淆三种坐标

本次修改不重新定义已有的 Data 位索引，也不改变旧原生数组入口的存储坐标。

| 坐标 | 含义 | 本次要求 |
| --- | --- | --- |
| `XData` 位偏移 | 最低有效位为 0 | 保持现有语义 |
| 显式 RTL 数组视图下标 | 声明中的 left/right，含升降序和负下标 | 保留声明并检查边界 |
| 旧原生数组存储坐标 | `NewXArray()` 等旧入口的零基存储位置 | 不悄悄改成 RTL 下标 |

对于 `logic [7:4] word`，RTL 下标 7 与 Data 位偏移 3 指向同一位。保存类型布局不意味着把 `word[3]` 自动重新解释成 RTL 下标 3。视图的数组维度和完整值的位偏移是两个独立契约。

## 当前已经实现的部分

配置生成与运行时视图并非从零开始：

1. Slang 已采集固定聚合类型、字段、边界、signedness 和四态属性。
2. `.xscfg` 的 `ports` 元数据保存端口 `Layout`；原生信号通过 `layout_id` 引用类型表。
3. `XSignalCFG.NewXView(name)` 已能按原生布局解析类型、核对存储并创建视图。
4. `XData.ArrayView(layout)`、`StructView(layout)` 已能以外部布局解释现有 packed 值，并共享其存储。
5. packed 字段切片、unpacked 独立字段绑定及编译器核验的 native offset/stride 已存在。

本次补上的是 **类型布局与实际 `XData` 的关联**。`NewXData()` 将已核验的类型关联到返回的 Data；端口封装在创建、绑定并登记原有 Data 后调用 `BindDataLayouts()`。类型经精确端口名到 ID 的 `port_types` 元数据引用同一张去重类型表，不再送给自动创建的 packed `*_view`。unpacked 容器通过 `NewViewFromPort()` 使用同一类型图和原有叶子。

这里的 `XPort` 只是一个命名 Data 的 group，不是 RTL struct/array，也没有自己的聚合
layout。`NewViewFromPort(group, name)` 中，结构来自配置里明确指定的 RTL 对象，group
仅提供名字到已有 Data 的查找。`BindDataLayouts(group)` 关联的是这些 Data 的类型，
不是给 group 设置类型；两者都不新增信号或时钟登记。

## 调用方式

下面的片段假定已用匹配的 ABI 6 SDK 导出并初始化具有相应 packed 端口的 DUT：

```python
dut.matrix.value = 0xA5
matrix = dut.matrix.ArrayView()
packet = dut.packet.StructView()
```

无参形式使用该 Data 已关联的类型。调用者不手写布局，不再依赖 `dut.matrix_view`、`dut.packet_view` 或自动附加的其他公开名称。手工 Data 可用 `SetLayout(layout)` 显式关联类型；`HasLayout()` 检查关联是否存在，视图请求时仍核验来源和类型。

现有带 `layout` 的重载仍可服务于手工构造的 Data；不能将其删除与去除生成成员混为一谈。手工 `XData(width)` 没有可恢复的 RTL 字段和数组声明，不能仅按位宽推测布局。没有布局或布局种类不匹配时，无参请求必须明确失败。

## 类型信息如何到达 Data

### 配置层提供类型，不提前创建全部视图

xcfg 负责查找、校验和复用类型描述。生成 `.xscfg` 时保存元数据，运行时在实际信号创建或绑定时建立关联；构造 xcfg 不等于展开所有信号、解析所有类型或建立所有视图。

同类型应共享只读描述，不在每个 Data 或数组元素上复制完整 JSON，不保留 Slang AST，不增加逐元素地址登记或额外配置文件。类型描述只表达语义；native 基址、padding、offset、stride 仍由已核验的存储绑定负责，不能从类型位宽猜测。

### 两条创建路径都要接通

- **端口**：封装已经创建并绑定 DPI/native Data，随后通过配置关联已有类型。不能另造一个原生 Data 替代端口对象，否则会改变后端、叶子身份或边沿提交。
- **内部信号**：通过 `xcfg.NewXData()` 或递归视图按需创建 Data 时，关联选中节点的类型；不要在 DUT 初始化时遍历全部内部信号。

关联使用 RTL 逻辑名或明确的类型引用，而不是被重命名后的语言属性名。类型查询不应依赖 mem_direct 基址非零：DPI 工程同样有端口类型配置。

### 字段、元素、重初始化和生命周期

类型不能只关联到顶层：通过布局选择字段或数组元素得到的 Data，也要获得其子类型。任意位切片则不能直接继承整个父 struct/array 布局；只有范围和类型确定匹配时才能保留相应类型。

复制 Data 时要分别处理类型与后端：共享不可变类型不等于共享模型存储。`ReInit()` 或宽度变化后必须清除或重新验证失效的类型与视图缓存，不能将旧布局用于新位宽。

Data 的类型引用不能指向已销毁 xcfg 中的临时对象。反过来，持有类型或视图也不应延长 DUT 内存的有效期；原有模型和父 Data 的生命周期检查仍须生效。获取视图不改变写模式，不增加 XPort 登记、时钟回调或逐字段 SV bridge。

## 实现位置

| 层次 | 主要位置 | 修改内容 |
| --- | --- | --- |
| xcomm Data | `include/xspcomm/xdata.h`、`src/data/data.cpp` | 可选的共享类型关联；初始化、复制、重初始化和失效规则 |
| xcomm 视图 | `src/data/view.cpp` | 复用类型描述，增加使用自身布局的显式入口，传播子类型 |
| xcomm 配置 | `src/core/signal_cfg.cpp` | 复用已有类型解析，把类型关联到创建或现有的 Data；保持按需查询 |
| 语言绑定 | `swig/xcomm.i`、`swig/view_raw.i`、`swig/python/xcomm.py` | 六语言暴露一致入口；检查嵌套 packed 节点转换与句柄生命周期 |
| Picker 生成器 | `src/codegen/array.cpp`、`names.cpp`、各语言生成器和模板 | 关联端口类型，移除 packed `*_view` 成员及其专用命名和登记 |
| Picker 类型 | `src/parser/sv.cpp` | 统一端口与原生显式视图的布局解释，不改变默认 packed Data 语义 |
| 测试与文档 | 两仓库的单元、绑定、真实 RTL 测试和双语手册 | 验证新入口、迁移旧生成成员用法，区分实际支持与未验收范围 |

`XData` 增加共享类型与按需视图缓存成员，改变了原生对象布局，因此本分支使用 **ABI 6**，动态库 SONAME 为 `libxspcomm.so.6`；ABI 5 的 `v1.5.0` SDK 不能与之混用。Picker 的 Python 依赖下界为 `uc-xcomm>=1.6,<2`。配套更新固定源码依赖并重编 SDK/DUT；没有全局对象地址表或旧布局兼容层。视图缓存只观察来源 Data，不拥有 DUT 内存。

## 回归要求

- 无参视图使用关联类型；手工 Data 缺少布局、类型不匹配或位宽不匹配时明确报错。
- packed 默认仍是 Data；不再自动导出或创建 `*_view` 成员；unpacked 容器继续可用。
- packed array 与 packed struct 的显式原则一致，字段和数组元素保留正确的子类型。
- 同一 RTL 声明在端口和原生显式视图中具有一致的维度与边界；保留负下标、升降序、字段顺序和类型属性。
- 整值和视图共享同一信号；字段写入不破坏相邻位，DPI/native 写入模式、输出方向、四态处理及生命周期行为不退化。
- 对语义有足够类型信息的 VPI 路径验证同一契约；缺少信息时明确报告能力限制，不猜布局。
- 初始化不全量展开内部信号或数组元素；按需请求不重复解析同类型布局。测量初始化至首次成功读取、重复查询及内存，而非只测构造函数。
- C++、Python、Java、Scala、Go、Lua 的接口与句柄所有权同步检查；未实际运行的语言或模拟器必须标明未验收。

### 本次验收与性能边界

ABI 6 的真实聚合 fixture 在 Verilator 5.052 上验证六语言的 DPI、mem_direct 与 VPI 路径，
Scala 使用 2.13.16。普通向量的非零/升序边界、嵌套 packed/unpacked 元素与端口/native 的
显式视图一致性另有 Python 回归。VCS、UVS、GSIM 聚合运行时，以及完整大核模型导出，
不属于已验收范围。

性能微测使用同一份含 **711,754 条合成、共享 32 位 struct 类型记录**的 `.xscfg`，对比
ABI 5 基线与 ABI 6，五次独立进程取中位数。被查询的值均实际绑定并读取，后续阶段保留 4,096
个 Data；这不是完整模型性能或全量物化测试。冷缓存使用 `POSIX_FADV_DONTNEED` 建议，
不等同于强制清空系统缓存。

| 指标 | ABI 5 基线 | ABI 6 类型关联 |
| --- | --- | --- |
| 本机 `sizeof(XData)` | 584 字节 | 616 字节 |
| 热缓存初始化至首次成功读值 | 0.375 ms | 0.522 ms |
| 建议冷缓存初始化至首次成功读值 | 5.26 ms | 8.01 ms |
| 重复创建同名 Data 并读值 | 1.13 µs/次 | 1.21 µs/次 |
| 同一 Data 重复取得 struct 字段并读值 | 14.67 µs/次 | 0.144 µs/次 |
| 读取并保留 4,096 个 Data 后的进程 RSS | 33.8 MiB | 34.0 MiB |
| 再取得并保留相应视图、字段后的进程 RSS | 44.8 MiB | 42.8 MiB |

基线重复视图使用带 JSON 布局的旧入口，新入口使用先前关联的类型及缓存，因此这项比较
只说明消除重复解析的收益，不代表通用读值加速。类型解析成本计入新入口的首读，首读及
每个 Data 的固定开销确实有所增加；初始化没有因此展开全部 711,754 条信号或创建所有视图。

[导出指南](../guides/export.md)、[内部信号指南](../guides/internal-signals.md)和[DUT API](../reference/dut-api.md)提供实际用法。外部消费者需要从自动 `*_view` 成员迁移到显式入口；配置库的 `Metadata("ports")` 入口保持可用。RTL 本来声明的 `*_view` 信号仍正常生成，此次删除的只是生成器额外附加的成员。

## 不属于本次修改的内容

不重写配置数据库的压缩、索引和模型标识机制，不新增 JSON/YAML 旁路文件，不改变已核验的 native 地址导出，不把旧存储坐标改成 RTL 坐标，也不顺带重定义普通位索引、加入 tuple 索引或优化逐位 PinBind 分配。这些问题需要独立的范围与验证，不能混入类型关联的实现。
