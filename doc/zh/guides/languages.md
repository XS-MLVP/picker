# 在不同语言中使用 DUT

`--lang` 支持 `python`、`cpp`、`java`、`scala`、`golang` 和 `lua`。选择语言后，Picker 会生成对应的包装、构建文件和端口接口。

## 构建并运行现有示例

在 Picker 源码目录中，先构建需要的语言支持。例如 Python 和 Java：

```sh
make build NPROC=4 BUILD_XSPCOMM_SWIG=python,java
make test_Adder EXAMPLE_LANG=java
```

第二条命令导出 Adder、编译并运行 `example/Adder/example.java`。把 `EXAMPLE_LANG` 改为 `python`、`cpp`、`scala`、`golang` 或 `lua`，即可选择对应示例。示例脚本会重新生成 `output/Adder`。

Java/Scala 需要 JDK；Scala 还需要 Scala 工具链；Go 需要 Go 编译器；Lua 需要解释器和开发头文件。可通过 `picker --check` 查看所选语言的资源是否齐全。

## 导出自己的设计

以 C++ 为例：

```sh
picker export example/Adder/Adder.v --sname Adder --lang cpp \
    --tdir build/AdderCpp --autobuild false
make -C build/AdderCpp -j4
```

先用仓库中对应语言的 `example/Adder/example.*` 熟悉生成接口，再替换为自己的设计和测试。

## 读写信号

| 语言 | 写入示例 | 读取示例 |
| --- | --- | --- |
| Python | `dut.a.value = 42` | `dut.sum.value` |
| C++ | `dut.a = 42` | `dut.sum.U()` |
| Java | `dut.a.Set(42)` | `dut.sum.U()` |
| Scala | `dut.a.Set(42)` | `dut.sum.U()` |
| Go | `dut.A.Set(42)` | `dut.Sum.U()` |
| Lua | `dut.a:Set(42)` | `dut.sum:U()` |

Python 的整数读写支持完整位宽。C++ 的 `U()` / `S()` 返回低 64 位，宽信号使用 `GetBytes()` / `SetBytes()`。Java/Scala 提供大整数包装；Go 和 Lua 的宽值可以通过字节或字符串接口处理，具体签名见生成代码和对应示例。

端口名与方法名冲突时，生成器会按语言规则调整名称，例如添加 `pin_` 前缀。Go 的导出名称首字母大写。

## 分发生成结果

编译后的 DUT 包包含语言接口、模型库和运行资源。默认的 `--cp_lib true` 会复制运行库。将整个包一起交付，并在目标环境运行一份短测试，检查导入、端口读写和仿真推进。

Python 扩展需要匹配目标环境的 Python 小版本、操作系统和架构。
