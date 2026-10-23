# Generate a UVM transaction bridge

Use `picker pack` with an existing UVM environment to generate Python transaction classes and UVM agents. They exchange data between Python tests and a SystemVerilog testbench.

## Generate from a transaction class

`example/Pack/adder_trans.sv` defines `adder_trans`, derived from `uvm_sequence_item`, with `a`, `b`, `cin`, `sum`, and `cout` fields. Run from the repository root:

```sh
picker pack example/Pack/adder_trans.sv --example
```

The output is written to `uvmpy/`:

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

`xagent.py` provides the Python transaction interface, and `xagent.sv` provides the UVM agent. `example.py` and `example.sv` show how the two sides connect and exchange transactions.

Put one transaction class in each input file. For multiple transactions, collect the files with `--filelist`; see `example/Pack/03_MultiTrans` for an example.

## Generate from RTL ports

You can also generate a transaction and DUT wrapper from module ports:

```sh
picker pack --from-rtl example/Adder/Adder.v --name Adder \
    --generate-dut --example
```

This generates `uvmpy/Adder/Adder_trans.sv` and its agent. `--name Adder` selects the module and package name; `--generate-dut` adds a wrapper for pin access. If the output directory already exists, save any manual changes, then use a new directory or regenerate with `--force`.

## Compile and run

The generated example Makefile uses VCS, UVM, UVMC, and SystemC, with Verdi PLI for waveforms. Set `VCS_HOME`, `UVM_HOME`, `UVMC_HOME`, and `VERDI_HOME` for your installation. Make sure `vcs`, `vlogan`, `syscan`, SWIG, and Python development tools are available, then run:

```sh
make -C uvmpy
```

The Makefile compiles the bridge library and UVM environment, then runs `example.py`. It locates the TLM SDK under `share/xspcomm/tlm` in the installation tree. Set `XSP_COMM_TLM` when using a separately installed SDK.

## Connect an existing testbench

Start from the generated `example.sv`. Connect the driver's transaction receive method to your design interface and send monitor samples back to Python. Use matching channel names and the same transaction field definitions on both sides.

`example/Adder/example-uvm.sv` shows a complete connection: the driver writes `a`, `b`, and `cin` after receiving a transaction, and the monitor samples and returns `sum` and `cout`. Adapt these driving and sampling steps for your design.

For build or communication failures, first check generation logs, tool paths, and channel names. See the [pack reference](../reference/cli.md#pack) for command options.
