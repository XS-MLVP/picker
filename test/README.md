Picker test suite

Overview
- Provides lightweight scripts to validate picker CLI and core flows.
- Focuses on Verilator + Python by default to minimize toolchain needs.

Prerequisites
- Built picker binary at `./build/bin/picker` (or installed `picker` in PATH).
- Verilator (>=4.218), CMake (>=3.11), a C++ compiler, Python3.

Usage
- Run all: `make -C test all`
- Smoke only (no builds): `make -C test smoke`
- Individual:
  - `make -C test export_python`
  - `make -C test export_vpi_python`
  - `make -C test export_filelist`
  - `make -C test export_complex_port`
  - `make -C test pack_transaction`
  - `make -C test pack_from_rtl`
  - `make -C test pack_unsupported_type`
  - `make -C test pack_multiple_classes`
  - `make -C test pack_python`

Notes
- Tests write to subfolders in the repo (e.g. `picker_out_*`, `pack_example`).
- Clean with `make -C test clean`.
