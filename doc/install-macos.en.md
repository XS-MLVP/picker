# Installing Picker on macOS

## Known Limitations

Due to differences between macOS and Linux in system kernels and development toolchains, Picker has some functional limitations on macOS:

1. The emulator only supports Verilator
2. Dynamic multi-instance is not supported; please use static multi-instance if multiple instances are needed

Therefore, we strongly recommend using Picker in a Linux environment for more comprehensive feature support.

## Dependency Installation

> [!WARNING]
> We recommend installing Picker dependencies on macOS via [Homebrew](https://brew.sh/). If you do not use Homebrew for dependency setup, you will need to troubleshoot any problems yourself.
>
> The instructions below assume you have Homebrew properly configured.

### Configuring Verible

Make sure that the environment variables for [Verible](https://github.com/chipsalliance/verible) are correctly set.

### Installing Dependencies

```bash
brew install cmake lcov lld llvm pkgconf python3 swig systemc verilator zlib
```

These dependencies are required when Picker exports C++ and Python DUTs. Among them:

- `llvm` is optional; you can also use the system’s default `clang` (AppleClang), but AppleClang typically compiles larger DUTs more slowly.
- `lld` is optional but speeds up linking. If not installed, the system will use the default `ld` (Apple Linker).
- If multiple Python versions exist on your system, ensure tools like `python` and `python-config` are not mixed across versions to avoid compilation errors caused by ABI mismatch.

If you want support for other languages like Java, Go, Lua, or Scala2, please configure the relevant toolchains yourself; this guide does not cover that.

## Building

> [!TIP]
> Please refer to the [README.md](README.md) in the project root for build steps.

Here we demonstrate installing picker to the `~/Applications/Picker` folder.

First, run this command to compile and install:

```shell
make init
make install ARGS="-DCMAKE_INSTALL_PREFIX=$HOME/Applications/Picker"
```

Then configure your environment variables:

```shell
export PATH="$HOME/Applications/Picker:$PATH"
```

Finally, run `picker --check` to verify the installation is complete.
