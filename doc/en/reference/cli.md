# Command-line reference

Use `picker --help`, `picker export --help`, and `picker pack --help` for command help. The tables below cover global options, RTL export, and UVM packing.

## Global options

| Option | Purpose |
| --- | --- |
| `-h, --help` | Show help |
| `-v, --version` | Show build version |
| `--check` | Inspect templates and language resources; read every result |
| `--show_default_template_path` | Print template location |
| `--show_xcom_lib_location_cpp` | Print C++ runtime include/lib locations |
| `--show_xcom_lib_location_python/java/scala/golang/lua` | Locate one language's resources; use one complete option name |

## export

| Option | Argument/default | Purpose |
| --- | --- | --- |
| `file` | One or more RTL files | Source inputs; required without `--sname` |
| `--fs, --filelist` | Files or lists, multiple allowed | Additional sources, includes and definitions |
| `--sname, --source_module_name` | Module names, multiple allowed | Select tops |
| `--tname, --target_module_name` | Name | Generated module name; defaults to source name |
| `--tdir, --target_dir` | Path | Destination; a trailing `/` adds a module-named directory |
| `--sdir, --source_dir` | Template directory | Override installed templates |
| `--lang, --language` | `python` | cpp/python/java/scala/golang/lua |
| `--sim` | `verilator` | verilator/vcs/gsim/uvs; requires the toolchain |
| `--rw, --access-mode` | `dpi` | dpi or mem_direct; see [array access modes](../guides/export.md#export-unpacked-array-ports) and [version support](compatibility.md#unpacked-arrays-and-verilator-versions) |
| `--internal` | Configuration file | Select internal signals |
| `--vpi` | Flag, off | Compile VPI support |
| `--checkpoints` | Flag, off | Compile supported save/restore functionality |
| `-w, --wave_file_name` | Filename, empty | Configure waveform format/output |
| `-c, --coverage` | Flag, off | Compile simulator coverage |
| `-F, --frequency` | `100MHz` | VCS timing; accepts Hz/KHz/MHz/GHz |
| `--verdi-mode` | `legacy` | VCS Verdi integration; modern selects the newer debug setup |
| `--cp_lib, --copy_xspcomm_lib` | `true` | Copy runtime libraries |
| `--rtl-param` | `NAME=VALUE`, repeatable | Override an RTL parameter port; evaluated by the RTL frontend and pinned in the generated wrapper |
| `--rtl-define` | `NAME[=VALUE]`, repeatable | Define an RTL preprocessor macro for both the RTL frontend and the simulator |
| `-V, --vflag` | String | Simulator compilation options |
| `-S, --sflag` | String list | slang parsing options |
| `-C, --cflag` | String | C++ compiler options |
| `-e, --example` | Flag, off | Generate/build an example |
| `--autobuild` | `true` | Build after generation |
| `-j, --build-threads` | 0 | Parallelism; 0 uses all cores |
| `--verbose` | Flag, off | Additional diagnostic output |

Quote arguments containing spaces, for example `-V '--trace -Wno-fatal'`. See [simulator support](compatibility.md) for tool environments.

`--rtl-param` and `--rtl-define` describe the design rather than the tools, so prefer them over the raw passthroughs. A simulator parameter override in `-V` cannot reach the instantiated module, and a macro in `-S` is invisible to the simulator (a macro in `-V` is invisible to the RTL frontend). Picker evaluates parameters with the RTL frontend once and writes the constants into the generated `<module>_top.sv`, so the simulator needs no parameter arguments of its own; macros are forwarded to both sides. Both structured options take precedence over a raw passthrough (`-S`/`-V`), and `--rtl-define` also overrides the same macro in a filelist.

## pack

| Option | Purpose |
| --- | --- |
| `file` | SV file containing transactions |
| `-f, --filelist` | Transaction file list |
| `--from-rtl` | Derive a transaction from RTL ports |
| `-d, --generate-dut` | Generate a DUT wrapper for pin access |
| `-r, --rename` | Rename a transaction |
| `-n, --name` | Generated package name |
| `-e, --example` | Generate an example project |
| `-c, --force` | Delete and replace an existing generated directory |

See [UVM transaction bridges](../guides/uvm-tlm.md) for generated directories and examples.

## config

`picker config <module>.xscfg` reports the format, model identifier, byte size and
record count. `--section metadata|variables|signals|types|candidates` inspects a
section; `--name NAME` selects one record without decoding the entire database.
Output is JSON on stdout; no sidecars are created. `candidates` is a build-time
section removed when native storage is finalized. Every export ships one database,
including DPI/VPI-only projects; JVM bindings embed it in their JAR.
See [internal signals](../guides/internal-signals.md).

## Environment variables

`DUMPVARS_OPTION` passes options to VCS `$fsdbDumpvars`, for example:

```sh
DUMPVARS_OPTION="+mda" picker export rtl/top.sv --sname Top --sim vcs -w trace.fsdb
```

This fragment requires real RTL and configured VCS/Verdi tools. `+mda` enables array waveform support in that environment.
