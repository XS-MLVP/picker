# Development and testing

After following [installation](../guides/installation.md) in a source checkout, you can edit, build, and run tests. Read [how generation and simulation work](../explanation/architecture.md) to locate the part you need to change.

## Source directories

| Path | Contents |
| --- | --- |
| `src/picker.cpp` | CLI arguments and export/pack flows |
| `src/parser/` | RTL, transactions, file lists, and signal layout parsing |
| `src/codegen/` | Convert parsed data into template inputs |
| `template/lib/` | Simulator wrappers and build configuration |
| `template/python/` and other language directories | Generated language interfaces and build templates |
| `test/` | Unit, export integration, and release script tests |
| `example/` | Runnable designs and tests |
| `doc/zh/`, `doc/en/` | Chinese and English documentation |

To fix generated code, edit its template and export an example again to verify it. Libraries under `dependence/` are maintained separately; commit dependency changes in their own repositories.

## Run tests

Build and run C++ unit tests:

```sh
make unit_tests NPROC=4 BUILD_XSPCOMM_SWIG=python,java
```

Choose integration tests for the change:

| Change | Command |
| --- | --- |
| Python export and templates | `make -C test export_python` |
| File lists | `make -C test export_filelist` |
| Complex ports | `make -C test export_complex_port` |
| Internal signals and VPI | `make -C test export_vpi_python` |
| Transaction packing | `make -C test pack_transaction` |
| RTL to transactions | `make -C test pack_from_rtl` |
| Release scripts | `python3 -m unittest discover -s test/release -v` |
| Documentation | [Checks and preview](documentation.md) |

Run the complete integration suite with `make -C test all`, using the corresponding language and simulator environments. Test preflight checks bindings and may rebuild missing ones; use a separate checkout when preserving other build outputs. Record the tests executed and any skips caused by missing tools.

## Submit a change

Work on a topic branch and open a PR against `master`. Describe how to reproduce the problem, the resulting behavior, and tests run. Update Chinese and English references and examples when changing an interface.

Choose one `release:patch`, `release:minor`, or `release:major` label per PR. See [release maintenance](releases.md) for the post-merge process and [troubleshooting](../reference/troubleshooting.md) for issue report details.
