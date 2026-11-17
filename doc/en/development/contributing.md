# Development and testing

After following [installation](../guides/installation.md) in a source checkout, you can edit, build, and run tests. Read [how generation and simulation work](../explanation/architecture.md) to locate the part you need to change.

## Source directories

| Path | Contents |
| --- | --- |
| `src/picker.cpp` | CLI arguments and export/pack flows |
| `src/parser/` | RTL, transactions, file lists, and signal layout parsing |
| `src/codegen/` | Convert parsed data into template inputs |
| `include/codegen/render_tags.hpp` | The only place template variables are declared |
| `template/lib/` | Simulator wrappers and build configuration |
| `template/python/` and other language directories | Generated language interfaces and build templates |
| `test/` | Unit, export integration, and release script tests |
| `example/` | Runnable designs and tests |
| `doc/zh/`, `doc/en/` | Chinese and English documentation |

To fix generated code, edit its template and export an example again to verify it. Libraries under `dependence/` are maintained separately; commit dependency changes in their own repositories.

## Template variables

Template variables are declared once, as rows of `PICKER_RENDER_TAGS` in `include/codegen/render_tags.hpp`. Each row carries the `tags::<id>` constant, the `__UPPER_SNAKE__` wire name used by `template/`, the value type, the scope and both summaries. Producers always write through the constant:

```cpp
data[tags::top_module_name] = dst_module_name;
```

To add, rename or remove a variable:

1. Edit its row in `PICKER_RENDER_TAGS`.
2. Update the assignment under `src/codegen/` and the use under `template/`.
3. Run `python scripts/check_render_tags.py --docs` to regenerate `doc/en/reference/render-tags.md` and `doc/zh/reference/render-tags.md`.
4. Run `python scripts/check_render_tags.py` to verify declaration, producers, templates and docs agree; CI runs the same check.

Internal flow keys such as `signal_tree` are declared with `internal` scope and keep their plain `snake_case` name.

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
| RTL parameters and macros | `make -C test export_rtl_params` |
| Unpacked array ports and native internal arrays | `make -C test export_unpacked_array` |
| Recursive struct and packed-array port views | `make -C test export_aggregate_views` |
| Internal signals and VPI | `make -C test export_vpi_python` |
| Transaction packing | `make -C test pack_transaction` |
| RTL to transactions | `make -C test pack_from_rtl` |
| Release scripts | `python3 -m unittest discover -s test/release -v` |
| Documentation | [Checks and preview](documentation.md) |

Run the complete integration suite with `make -C test all`, using the corresponding language and simulator environments. Preflight checks and builds bindings as needed; use a separate checkout when preserving other build outputs. Record tool versions and test results.
Set `AGGREGATE_EXTRA_LANGUAGES=1` when running `export_aggregate_views` with the
Go, Lua and Scala toolchains and xcomm bindings installed to include those RTL tests.
Set `ARRAY_EXTRA_LANGUAGES=1` for additional Go/Lua `export_unpacked_array` tests.

VPI regressions report basic aggregate access separately from `test_vpi_unpacked_struct_*`.
Cases reuse each language's build artifacts and run in separate processes.

## Submit a change

Work on a topic branch and open a PR against `master`. Describe how to reproduce the problem, the resulting behavior, and tests run. Update Chinese and English references and examples when changing an interface.

Choose one `release:patch`, `release:minor`, or `release:major` label per PR. See [release maintenance](releases.md) for the post-merge process and [troubleshooting](../reference/troubleshooting.md) for issue report details.
