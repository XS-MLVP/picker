# 使用 VCS 时的静态 TLS 空间不足问题

在 Linux 上使用 Picker 和 VCS 时，如果加载生成的 DUT 动态库或其依赖库出现以下报错，可以尝试增大 glibc 预留的静态 TLS 空间：

```text
cannot allocate memory in static TLS block
```

报错前可能还会显示具体的动态库路径；在 Python 中，也可能以 `ImportError` 或 `OSError` 的形式出现。

这里的 TLS 指线程局部存储（Thread-Local Storage）。某些动态库需要使用静态 TLS，而进程启动时预留的空间有限；在运行过程中加载这些库时，如果剩余空间不足，就可能加载失败。这类报错不一定意味着系统物理内存不足。

## 解决方法

在终端中先执行：

```bash
export GLIBC_TUNABLES=glibc.rtld.optional_static_tls=262144
```

然后在同一个终端中重新启动原先失败的命令，例如运行使用 VCS 的 DUT 的 Python 测试程序：

```bash
# 在测试程序所在目录执行，将 test.py 替换为实际的程序文件名
python3 test.py
```

`glibc.rtld.optional_static_tls` 控制 glibc 在进程启动时额外预留的可选静态 TLS 空间，单位为字节。`262144` 即 256 KiB；这部分预留会增加每个线程的内存开销。

也可以只为某一次运行设置该环境变量：

```bash
GLIBC_TUNABLES=glibc.rtld.optional_static_tls=262144 python3 test.py
```

该设置必须在**加载 DUT 的宿主进程启动之前**生效。如果是在 Python 中加载 DUT，应在启动 Python 前设置；在已经运行的 Python 中修改 `os.environ`，无法重新调整当前进程的静态 TLS 预留空间。使用 Jupyter 时，需要让新启动的内核继承该环境变量。

通常不需要为此重新生成或编译 DUT，只需带着该环境变量重新启动相关进程。如果构建脚本会自动运行仿真或测试，也应在启动脚本前设置。

## 保留已有的 glibc 配置

上面的直接赋值会覆盖已有的 `GLIBC_TUNABLES`。如果已经设置了其他 glibc 参数，应使用冒号分隔并保留它们。例如，已有配置中**不包含** `glibc.rtld.optional_static_tls` 时，可以执行：

```bash
export GLIBC_TUNABLES="${GLIBC_TUNABLES:+${GLIBC_TUNABLES}:}glibc.rtld.optional_static_tls=262144"
```

如果已有配置中包含这个参数，直接将其值改为 `262144`，避免重复添加。

需要长期使用时，可以将适合自己环境的 `export` 命令放入仿真启动脚本，或 Bash 的 `~/.bashrc` 中。修改 `~/.bashrc` 后，打开新的终端，或执行 `source ~/.bashrc`，再启动测试程序。通过 IDE、CI 或容器启动时，应在对应的启动环境中配置该变量，确保实际运行仿真的进程能够继承它。

## 设置后仍然报错

- 确认报错确实涉及静态 TLS 空间不足；这个参数无法解决所有动态库加载错误。
- 在启动测试的终端中执行 `printenv GLIBC_TUNABLES`，确认配置值，并确保测试进程是在设置之后新启动的。
- 确认使用的是支持该参数的 glibc 环境。上游 glibc 从 2.32 起提供此参数，发行版可能有回移；不支持的环境可能忽略该参数。Linux 上可以使用 `getconf GNU_LIBC_VERSION` 查看 glibc 版本；此配置不适用于 musl 或 macOS。
- `262144` 是可尝试的配置值，具体需求取决于加载的库和运行环境。如果仍然失败，记录完整报错、glibc 版本、VCS 版本和启动命令，以便进一步排查。

参数定义可参考 [glibc 官方手册：Dynamic Linking Tunables](https://www.gnu.org/software/libc/manual/html_node/Dynamic-Linking-Tunables.html)。
