Picker test suite

Overview
- Provides lightweight scripts to validate picker CLI and core flows.
- Focuses on Verilator + Python by default to minimize toolchain needs.

Prerequisites
- Built picker binary at `./build/bin/picker` (or installed `picker` in PATH).
- Verilator (>=4.218), CMake (>=3.11), a C++ compiler, Python3.
- Verible `verible-verilog-syntax` in PATH for export examples.

Usage
- Run all: `make -C test all`
- Smoke only (no builds): `make -C test smoke`
- Individual:
  - `make -C test export_python`
  - `make -C test export_vpi_python`
  - `make -C test pack_python`

Notes
- Tests write to subfolders in the repo (e.g. `picker_out_*`, `pack_example`).
- Clean with `make -C test clean`.
