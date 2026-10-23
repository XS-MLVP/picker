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
| `--rw, --access-mode` | `dpi` | dpi or mem_direct; see the access guide for support |
| `--internal` | Configuration file | Select internal signals |
| `--vpi` | Flag, off | Compile VPI support |
| `--checkpoints` | Flag, off | Compile supported save/restore functionality |
| `-w, --wave_file_name` | Filename, empty | Configure waveform format/output |
| `-c, --coverage` | Flag, off | Compile simulator coverage |
| `-F, --frequency` | `100MHz` | VCS timing; accepts Hz/KHz/MHz/GHz |
| `--verdi-mode` | `legacy` | VCS Verdi integration; modern selects the newer debug setup |
| `--cp_lib, --copy_xspcomm_lib` | `true` | Copy runtime libraries |
| `-V, --vflag` | String | Simulator compilation options |
| `-S, --sflag` | String list | slang parsing options |
| `-C, --cflag` | String | C++ compiler options |
| `-e, --example` | Flag, off | Generate/build an example |
| `--autobuild` | `true` | Build after generation |
| `-j, --build-threads` | 0 | Parallelism; 0 uses all cores |
| `--verbose` | Flag, off | Additional diagnostic output |

Quote arguments containing spaces, for example `-V '--trace -Wno-fatal'`. See [simulator support](compatibility.md) for tool environments.

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

## Environment variables

`DUMPVARS_OPTION` passes options to VCS `$fsdbDumpvars`, for example:

```sh
DUMPVARS_OPTION="+mda" picker export rtl/top.sv --sname Top --sim vcs -w trace.fsdb
```

This fragment requires real RTL and configured VCS/Verdi tools. `+mda` enables array waveform support in that environment.
