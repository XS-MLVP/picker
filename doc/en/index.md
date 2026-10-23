# Picker documentation

Picker turns Verilog/SystemVerilog designs into Python, C++, Java, Scala, Go, or Lua modules. Your tests can assign inputs, advance the simulation, read outputs, and check the results with assertions.

Start with [installation](guides/installation.md), then follow the [adder tutorial](tutorials/first-dut.md) to run your first test.

## Use Picker

| Task | Guide |
| --- | --- |
| Export your RTL | [Export an RTL project](guides/export.md) |
| Set up clocks, reset, and sampling | [Drive a simulation](guides/simulation.md) |
| Save waveforms and collect code coverage | [Record simulation results](guides/simulation.md#recording) |
| Inspect internal registers or memories | [Access internal signals](guides/internal-signals.md) |
| Use C++, Java, or another language | [Language examples](guides/languages.md) |
| Exchange transactions between Python and UVM | [Generate a UVM transaction bridge](guides/uvm-tlm.md) |

## Look up an interface

The [CLI reference](reference/cli.md) lists export options. The [DUT API](reference/dut-api.md) describes generated objects and signal access. Use [simulator and language support](reference/compatibility.md) to choose a toolchain, and [troubleshooting](reference/troubleshooting.md) to diagnose errors.

## Understand and change Picker

[How generation and simulation work](explanation/architecture.md) follows RTL into a callable simulation module. The [development guide](development/contributing.md) covers source directories and tests. [Documentation maintenance](development/documentation.md) covers bilingual editing, site previews, and Wiki export.
