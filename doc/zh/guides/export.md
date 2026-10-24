# 导出 RTL 工程

导出时需要确定源文件、顶层模块、目标语言和模拟器。下面的命令在 Picker 仓库根目录运行；接入自己的工程时，替换文件路径和模块名即可。

## 导出单个文件

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --sim verilator --tdir build/Adder --autobuild false
make -C build/Adder -j4
```

`--sname` 选择 RTL 中的顶层模块。`--tdir` 指定生成目录，`--autobuild false` 让生成和编译分两步执行。保留默认的自动构建时，Picker 会在生成完成后直接编译。

如果希望 Python 包使用另一个名字，例如 `Arithmetic`，添加 `--tname Arithmetic`，并将输出目录改成 `build/Arithmetic`。对应的类名为 `DUTArithmetic`。

## 使用文件列表

工程包含多个文件时，通过 `--fs` 提供文件列表。仓库的 `example/MultiFileRTL/design.f` 包含头文件目录和多个源码文件：

```text
+incdir+inc
inc/defs.vh
inc/outer.svh
leaf.v
top.sv
```

这些路径相对于文件列表所在目录。在仓库根目录执行：

```sh
picker export --fs example/MultiFileRTL/design.f \
    --sname MultiFileRTLTop --lang python --sim verilator \
    --tdir build/MultiFileRTLTop --autobuild false
make -C build/MultiFileRTLTop -j4
```

文件列表支持嵌套 `-f` / `-F`、`+incdir+` 和 `+define+`。将工程使用的包含目录和宏定义写入列表，让解析和模拟器编译使用同一组输入。

## 传入编译选项

| 选项 | 作用阶段 | 常见用途 |
| --- | --- | --- |
| `-S` | slang 解析 | 解析器选项、顶层参数配置 |
| `-V` | 模拟器编译 | 模拟器的诊断或优化选项 |
| `-C` | C++ 编译 | 编译器选项 |

含空格的参数需要加引号。例如，向 Verilator 传入警告选项：

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --tdir build/AdderDebug -V '-Wno-fatal' --autobuild false
```

## 将多个模块组合导出

`--sname` 可以指定模块及实例数。假设 `rtl/a.v` 定义 `A`，`rtl/b.v` 定义 `B`，以下命令生成含两个 A 和三个 B 的组合模块 C：

```sh
picker export rtl/a.v,rtl/b.v --sname A,2,B,3 \
    --tname C --lang python --tdir build/C
```

生成后，查看接口文件中的实例和端口名称，再编写对应测试。

## 添加调试功能

- 使用 `-w trace.fst` 生成波形支持，具体操作见[记录仿真结果](simulation.md#recording)。
- 使用 `--coverage` 收集模拟器代码覆盖率。
- 使用 `--vpi` 或 `--internal` 访问内部信号，见[内部信号指南](internal-signals.md)。
- 使用 `--lang` 切换语言，见[多语言示例](languages.md)。

完整选项、默认值和输出目录规则见[命令行参考](../reference/cli.md)。
