# 发行维护

Picker 的版本来自 `vX.Y.Z` 标签。PR 上的 `release:patch`、`release:minor` 或 `release:major` 标签决定合并后的版本增量。

## 自动发布流程

1. PR 通过版本检查和相关构建后，合并到 `master`。
2. Version policy 创建版本标签，并触发 Tagged CI。
3. Tagged CI 构建、安装并验证该标签对应的 wheel 和原生产物。
4. Release Picker 等待源构建完成，最长 60 分钟；随后检查提交 SHA 和产物哈希，将产物发布到 GitHub Release。

正式标签对应 `master` 第一父链上的合并结果。等待本次标签创建完成后，再合并下一个发行 PR。仓库分支保护应将版本检查和相关构建设为必需状态。

## 维护依赖配置

| 配置 | 作用 |
| --- | --- |
| `pyproject.toml` | Python 包依赖范围 |
| `.build-config.yml` | CI 构建依赖的源码提交和工具配置 |
| `.github/workflows/ci.yml` | 开发构建和预览 wheel 配置 |
| Tagged CI | 为正式构建选择满足约束的已发布依赖 |

更新依赖后，重新构建并验证 wheel 和生成的 DUT。需要新依赖版本时，先完成该依赖的发布，再发布 Picker。

## 失败恢复

| 失败位置 | 操作 |
| --- | --- |
| Tagged CI 构建失败 | 修复构建环境后，对已有标签重跑 Tagged CI |
| 发布任务失败 | 使用成功的源 run ID 重跑 Release Picker |
| 合并事件未创建标签 | 检查 Version policy 日志，使用其 manual bump 恢复 |

恢复过程中沿用已创建的标签，保留提交和产物的对应关系。

## 本地检查

```sh
python3 -m unittest discover -s test/release -v
bash scripts/build/verify-wheel.sh
```

wheel 检查需要 `dist/` 中有构建产物，也可通过 `PICKER_WHEEL_DIR` 和 `XCOMM_WHEEL_DIR` 指定目录。脚本在独立环境安装并检查包和接口。

当前发行产物发布到 GitHub Releases。文档 CI 的站点和 Wiki 预览可从构建 artifacts 下载，见[文档维护](documentation.md)。

## Python 分发包与 PyPI 配置

分发包名为 `uc-picker`，命令仍是 `picker`，依赖 `uc-xcomm>=1.1,<2` 和 SWIG 4.5.1。
先发布 `uc-xcomm`；Tagged CI 选择同时存在于 GitHub 与 PyPI 的兼容稳定版本。
开发 CI 固定对应 xcomm 提交，仅在测试构建中使用满足目标范围的版本号。

发行 wheel 在固定的 manylinux 2.28 x86-64 镜像中构建，使用通用 CPU 参数并静态链接
C++ 运行库，通过审计后上传。标签为 `py3-none-manylinux_2_28_x86_64`：Picker 本身
不绑定 Python 小版本，但依赖 `uc-xcomm` 仍绑定。wheel 不再内置 xcomm 绑定，原始
CMake 构建仍可编译它。AppImage 与原生压缩包保留现有架构和文件名；开发 wheel
保留构建主机的平台标签。

在 [PyPI Publishing](https://pypi.org/manage/account/publishing/) 登记 pending GitHub publisher：

| 字段 | 值 |
| --- | --- |
| Project name | `uc-picker` |
| Owner | `XS-MLVP` |
| Repository | `picker` |
| Workflow filename | `release.yml` |
| Environment | `pypi` |

创建同名 GitHub 环境，允许从 `master` 发行。无需 PyPI API token。若首次 PyPI 任务
因配置未完成而失败，补齐发布者后重跑该任务，复用同一份已验证 CI 产物。
