# Picker

[![PyPI](https://img.shields.io/pypi/v/uc-picker?style=flat-square&logo=pypi&logoColor=white)](https://pypi.org/project/uc-picker/)
[![Python](https://img.shields.io/badge/python-3.11%20%7C%203.12-blue?style=flat-square&logo=python&logoColor=white)](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/guides/installation.md)
[![PR CI](https://img.shields.io/github/actions/workflow/status/XS-MLVP/picker/ci.yml?event=pull_request&style=flat-square&label=PR%20CI&logo=github)](https://github.com/XS-MLVP/picker/actions/workflows/ci.yml?query=event%3Apull_request)
[![License](https://img.shields.io/badge/license-MulanPSL--2.0-blue?style=flat-square)](https://github.com/XS-MLVP/picker/blob/master/LICENSE)
[![Docs](https://img.shields.io/badge/docs-Guide-0A7EA4?style=flat-square)](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/index.md)
[![GitHub Stars](https://img.shields.io/github/stars/XS-MLVP/picker?style=social)](https://github.com/XS-MLVP/picker)

English | [中文](https://github.com/XS-MLVP/picker/blob/master/README.zh.md)

Picker exports Verilog/SystemVerilog designs as Python, C++, Java, Scala, Go, or Lua modules, so you can write hardware tests in a familiar language. Tests can read and write signals, advance clocks, check results, and save waveforms. Picker also generates transaction bridges between Python and UVM.

The Python distribution is `uc-picker`; the command remains `picker`. It depends on `uc-xcomm` and SWIG.

## Get started

[Install Picker](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/guides/installation.md), then follow the [adder tutorial](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/tutorials/first-dut.md) to run your first test. From the repository root, export a Python module:

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --sim verilator --tdir build/docs-adder/Adder --autobuild false
make -C build/docs-adder/Adder -j4
```

Create a test in the generated package's parent directory to assign inputs by port name and check outputs. The tutorial includes a complete script and expected results.

## VCS integration

Generated Python DUT classes expose `SIMULATOR` (for example, `DUTAdder.SIMULATOR == "vcs"`)
and `GetSimulator()`. This identifies the export backend, not a runtime backend switch.

VCS waveform paths must use `.fsdb`; use `--verdi-mode modern` with current Verdi releases.
`Finish()` finalizes simulation and coverage in a forked child and waits for it, allowing
Python to continue pytest teardown and report generation.

Coverage-enabled VCS exports include `coverage/coverage`, a standalone UCAPI query
executable using schema version 1, without xcomm coverage-client bindings.
`GetCoveragePath()` returns the actual VDB; `SetCoverage()` selects a testdata name.
Finalization uses a database lock before the snapshot can be queried. Regenerate and
rebuild existing DUT packages after upgrading; installing Picker or updating only the
Python reporter does not update their compiled libraries.

## Documentation

[English documentation](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/index.md) · [完整中文文档](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/zh/index.md)

- Use Picker: [export RTL](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/guides/export.md), [clocks and waveforms](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/guides/simulation.md), [internal signals](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/guides/internal-signals.md), [languages](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/guides/languages.md), [UVM](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/guides/uvm-tlm.md).
- Look up an interface: [CLI](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/reference/cli.md), [DUT API](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/reference/dut-api.md), [troubleshooting](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/reference/troubleshooting.md).
- Develop Picker: [how it works](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/explanation/architecture.md), [contributing and tests](https://github.com/XS-MLVP/picker/blob/master/CONTRIBUTING.md), [documentation and Wiki](https://github.com/XS-MLVP/picker/blob/85fb45bc7432e9e94b59f8bb8805c40f0d544a0a/doc/en/development/documentation.md).

## Community

Report issues and suggestions through [GitHub Issues](https://github.com/XS-MLVP/picker/issues). The community QQ group is 600480230. More verification resources are available in the [MLVP documentation](https://open-verify.cc/mlvp/docs/).

[Code of conduct](https://github.com/XS-MLVP/picker/blob/master/CODE_OF_CONDUCT.md) · [Security reports](https://github.com/XS-MLVP/picker/blob/master/SECURITY.md) · [License](https://github.com/XS-MLVP/picker/blob/master/LICENSE)
