# 生成 UVM 事务桥

已有 UVM 环境时，可以用 `picker pack` 生成事务对应的 Python 类和 UVM agent，在 Python 测试与 SystemVerilog testbench 之间传递数据。

## 从事务类生成

仓库的 `example/Pack/adder_trans.sv` 定义了继承 `uvm_sequence_item` 的 `adder_trans`，包含 `a`、`b`、`cin`、`sum` 和 `cout` 字段。在仓库根目录执行：

```sh
picker pack example/Pack/adder_trans.sv --example
```

生成结果位于 `uvmpy/`：

```text
uvmpy/
├── Makefile
├── example.py
├── example.sv
└── adder_trans/
    ├── __init__.py
    ├── xagent.py
    ├── xagent.sv
    ├── utils_pkg.sv
    └── ...
```

`xagent.py` 提供 Python 事务接口，`xagent.sv` 提供 UVM agent。`example.py` 和 `example.sv` 展示两端如何连接和收发事务。

每个输入文件放一个事务类。多个事务分别放入文件后，可用 `--filelist` 汇总；仓库 `example/Pack/03_MultiTrans` 提供示例。

## 从 RTL 端口生成

也可以从模块端口生成事务和 DUT 包装：

```sh
picker pack --from-rtl example/Adder/Adder.v --name Adder \
    --generate-dut --example
```

这会生成 `uvmpy/Adder/Adder_trans.sv` 和对应 agent。`--name Adder` 选择模块及包名，`--generate-dut` 增加逐引脚操作包装。已有同名输出目录时，先保存手工修改，再选择新目录或用 `--force` 重新生成。

## 编译和运行

生成的示例 Makefile 使用 VCS、UVM、UVMC 和 SystemC，并通过 Verdi PLI 记录波形。准备好工具环境后，设置 `VCS_HOME`、`UVM_HOME`、`UVMC_HOME`、`VERDI_HOME`，确认 `vcs`、`vlogan`、`syscan`、SWIG 和 Python 开发工具可执行，然后运行：

```sh
make -C uvmpy
```

Makefile 会编译桥接库、编译 UVM 环境，并执行 `example.py`。TLM SDK 默认从安装目录中的 `share/xspcomm/tlm` 查找；单独安装 SDK 时，可通过 `XSP_COMM_TLM` 指定路径。

## 接入已有 testbench

以生成的 `example.sv` 为起点，将 driver 的事务接收方法连接到设计接口，将 monitor 的采样结果发送回 Python。发送端和接收端使用相同通道名，双方使用同一份事务字段定义。

`example/Adder/example-uvm.sv` 展示了完整连接：driver 收到事务后写入 `a`、`b`、`cin`，monitor 采样 `sum`、`cout` 并回传。替换设计时，重点修改这些驱动和采样步骤。

编译或收发失败时，先检查生成日志、工具路径和通道名称；命令参数见 [pack 参考](../reference/cli.md#pack)。
