# Picker

English | [中文](README.zh.md)

Picker exports Verilog/SystemVerilog designs as Python, C++, Java, Scala, Go, or Lua modules, so you can write hardware tests in a familiar language. Tests can read and write signals, advance clocks, check results, and save waveforms. Picker also generates transaction bridges between Python and UVM.

## Get started

[Install Picker](doc/en/guides/installation.md), then follow the [adder tutorial](doc/en/tutorials/first-dut.md) to run your first test. From the repository root, export a Python module:

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --sim verilator --tdir build/docs-adder/Adder --autobuild false
make -C build/docs-adder/Adder -j4
```

Create a test in the generated package's parent directory to assign inputs by port name and check outputs. The tutorial includes a complete script and expected results.

## Documentation

[English documentation](doc/en/index.md) · [完整中文文档](doc/zh/index.md)

- Use Picker: [export RTL](doc/en/guides/export.md), [clocks and waveforms](doc/en/guides/simulation.md), [internal signals](doc/en/guides/internal-signals.md), [languages](doc/en/guides/languages.md), [UVM](doc/en/guides/uvm-tlm.md).
- Look up an interface: [CLI](doc/en/reference/cli.md), [DUT API](doc/en/reference/dut-api.md), [troubleshooting](doc/en/reference/troubleshooting.md).
- Develop Picker: [how it works](doc/en/explanation/architecture.md), [contributing and tests](CONTRIBUTING.md), [documentation and Wiki](doc/en/development/documentation.md).

## Community

Report issues and suggestions through [GitHub Issues](https://github.com/XS-MLVP/picker/issues). The community QQ group is 600480230. More verification resources are available in the [MLVP documentation](https://open-verify.cc/mlvp/docs/).

[Code of conduct](CODE_OF_CONDUCT.md) · [Security reports](SECURITY.md) · [License](LICENSE)
