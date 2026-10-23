# Install Picker

The Python + Verilator examples require Picker, Verilator, SWIG, a C++ compiler, and Python development files. The steps below build from source. If you already have the compilation tools, you can also use a release package described at the end of this page.

## Prepare the tools

| Tool | Requirement or purpose |
| --- | --- |
| C++ compiler | C++20 support, such as GCC 11 or newer |
| CMake, make, Git | Configure, compile, and fetch dependencies |
| Python and development headers | Build Python extensions; use the same Python environment for compilation and execution |
| SWIG | 4.2 or newer, for language bindings |
| Verilator | 5.020 or newer, to compile RTL |
| bison, flex, gperf | Build supporting tools |
| zlib and lz4 development packages | Compression dependencies |

Follow the [Verilator installation guide](https://verilator.org/guide/latest/install.html) and the [SWIG download page](https://www.swig.org/download.html) to install these tools. Then check:

```sh
c++ --version
cmake --version
python3 --version
swig -version
verilator --version
```

On macOS, install the tools with Homebrew:

```sh
brew install cmake pkgconf python3 swig verilator zlib lz4
```

## Build and install

Fetch the source and dependencies, then install into your user directory:

```sh
git clone https://github.com/XS-MLVP/picker.git
cd picker
make init
make install NPROC=4 BUILD_XSPCOMM_SWIG=python \
    ARGS="-DCMAKE_INSTALL_PREFIX=$HOME/.local/picker"
export PATH="$HOME/.local/picker/bin:$PATH"
```

`make init` downloads dependencies into `dependence/`. `NPROC=4` runs four compilation jobs; reduce it on machines with less memory. Add the `export PATH=...` line to your shell configuration to make Picker available in new terminals.

For use within the source directory, replace `make install` with `make build` and run commands through `./build/bin/picker`.

## Check the installation

```sh
picker --version
picker --check
```

`--check` reports the status of templates and language resources. The tutorial needs successful checks for templates and Python. You can add other languages when you need them.

Next, export and run the [adder tutorial](../tutorials/first-dut.md) to check the full compilation and simulation process.

## Use a release package

[GitHub Releases](https://github.com/XS-MLVP/picker/releases) provides Linux AppImages, installation archives, and Python wheels. Choose a file for your machine's architecture. Wheels must also match the Python minor version.

For an AppImage, make the downloaded file executable and run it. For an installation archive, extract the full directory and add its `bin` directory to `PATH`. Templates and runtime resources are located through the installation tree, so keep them together when distributing it.

To install wheels, put Picker and the dependency wheels listed in the release notes into `wheels/`, then run:

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install --no-index --find-links=wheels picker
python -m pip check
picker --check
```

Exporting new models also requires the Verilator, SWIG, and compilation tools listed above.

## Build wheels locally

From a source directory where you have run `make init`:

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install build
make wheel PYTHON="$VIRTUAL_ENV/bin/python"
python -m pip install --no-index --find-links=dist picker
python -m pip check
```

The wheels are written to `dist/`. You can copy them to another environment with the same Python minor version and platform.
