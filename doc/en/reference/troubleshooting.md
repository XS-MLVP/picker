# Troubleshooting

Keep the complete export command and the first error. The parser, compiler, or simulator usually reports the specific cause before a later `make` failure.

## Export and compilation

| Check | Action |
| --- | --- |
| Template path | Run `picker --check` and `picker --show_default_template_path`; use the full installation tree or set `--sdir` |
| Top module | Check the `--sname` spelling, source list, and conditional compilation macros |
| Include paths | Add the directory with `+incdir+` in the file list and check relative paths |
| Port widths | Check top-level parameters, macros, and the elaborated interface |
| SWIG and compiler | Check PATH and prepare the tools in [installation](../guides/installation.md) |
| Language resources | Inspect the corresponding `picker --check` entry and build the required bindings |

## Python imports and signals

| Check or error text | Action |
| --- | --- |
| `No module named 'Adder'` | Run from the generated package's parent directory or add that directory to PYTHONPATH |
| Native extension loading | Check the Python minor version, platform, and architecture; rebuild in the same environment |
| `libxspcomm` and library symbols | Restore the complete generated package and runtime resources; regenerate and compile after replacing dependencies |
| `XData attributes of DUT are read-only` | Assign values through `dut.pin.value = value` |
| Writes and output refresh | Call `Step()` after writing inputs; see [drive a simulation](../guides/simulation.md) for combinational evaluation |
| Wide-signal reads | Use Python `.value` or the full C++ byte interface, and check the widths of temporary variables in the test |
| Internal signal lookup | Enable VPI or native layout at export time, enumerate actual names, then query the object |

## Waveforms and UVM

For an empty waveform, check `-w` in the export command, the runtime output path, and pause state. Call `Finish()` to complete output handling. The export configuration determines the file format. See [save waveforms](../guides/simulation.md#recording) for complete commands.

For UVM builds, check VCS, UVM, UVMC, Verdi, and SDK paths. For transaction
transfer, check channel names and driver/monitor execution; see [UVM transaction bridges](../guides/uvm-tlm.md).

## Report a problem

When opening a [GitHub issue](https://github.com/XS-MLVP/picker/issues), include minimal RTL, file lists, the complete export command, and error logs, plus `picker --version`, operating system, Python, and simulator versions. For private designs, provide a public example that retains the relevant syntax or interface.
