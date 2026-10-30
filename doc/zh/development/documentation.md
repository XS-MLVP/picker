# 维护文档与 Wiki

中文正文放在 `doc/zh`，英文正文放在 `doc/en`，对应页面使用相同相对路径。修改正文时同步两种语言，导航在 `mkdocs.yml` 中维护。

## 按读者的需要组织内容

采用 [Diátaxis](https://diataxis.fr/) 的四种文档类型：

| 类型 | 读者需要什么 | 写作方式 |
| --- | --- | --- |
| 教程 | 在指导下完成一次练习 | 给出连续的小步骤，每一步都有能观察到的结果 |
| 操作指南 | 完成一个已知任务 | 说明操作、必要选项和结果检查，例如“保存波形” |
| 接口参考 | 查清一个参数或方法 | 按接口分组，准确写出签名、默认值、返回值和使用条件 |
| 原理说明 | 理解设计和行为 | 解释对象之间的关系、调用过程及设计原因 |

教程中先让读者操作并看到结果，再引入理解下一步所需的概念。详细参数链接到参考页，设计原因链接到原理页。

标题使用具体动作，正文直接说明做法。例如：“调用 `Finish()` 完成输出，然后打开波形文件。”说明限制时写清触发条件和影响，放在相关操作或接口旁。采用 [Google 技术写作指南](https://developers.google.com/style/highlights)建议的直接、清晰表达。

## 编写示例

标明命令执行目录、代码保存路径和预期输出。完整脚本可直接复制运行；片段说明已有的对象和环境。中文和英文使用相同的可执行代码，将解释放在正文。

## 预览网站

在仓库根目录执行：

```sh
python3 -m venv /tmp/docs-venv
/tmp/docs-venv/bin/python -m pip install -r requirements-docs.txt
/tmp/docs-venv/bin/python scripts/docs.py check
/tmp/docs-venv/bin/python -m mkdocs build --strict
/tmp/docs-venv/bin/python -m mkdocs serve -a 127.0.0.1:8000
```

浏览器打开 `http://127.0.0.1:8000`，检查语言切换、导航、全文搜索和代码显示。静态站点输出到 `build/docs-site`。

`check` 校验中英文页面对应关系、导航、链接、锚点和 Python 语法。完整可执行代码块前添加 `<!-- runnable:python -->` 或 `<!-- runnable:cpp -->`，可以通过以下命令运行：

```sh
/tmp/docs-venv/bin/python scripts/docs.py test --python /path/to/runtime/bin/python
```

Picker 的示例需要先导出教程中的 Adder、带波形和覆盖率的 Adder，以及内部信号指南中的 vpi。将带记录功能的 Adder 和 vpi 包放在同一个目录后，用 `--cwd` 指定该目录。

## 发布 GitHub Wiki

网站和 Wiki 由同一份 Markdown 生成。在空目录预览导出结果：

```sh
/tmp/docs-venv/bin/python scripts/docs.py wiki --output /tmp/picker-wiki-export
```

导出包含双语页面、首页、侧边栏、页脚和 SHA-256 文件清单。
页面链接转换为 GitHub Wiki 的绝对 URL，源码和图片链接指向主仓库。

**Documentation** 工作流检查链接、测试文档工具、构建网站，并上传网站和 Wiki 导出供评审。
文档修改进入 `master` 后，**Sync GitHub Wiki** 任务将已校验的导出同步到 `picker.wiki.git`。
PR 和 `docs/**` 分支只生成预览。需要重试时，在 Actions 页面手动运行 **Documentation**，
选择 `master`。若已有更新的文档提交，旧导出会跳过同步。

发布任务使用自带的 `GITHUB_TOKEN`，仅该任务授予 `contents: write`，无需额外 PAT 或 secret。
首次使用须启用仓库 Wiki，并在网页创建一次首页，之后 Actions 才能检出 Wiki 仓库。
参见 [GitHub Wiki 文档](https://docs.github.com/en/communities/documenting-your-project-with-wikis/adding-or-editing-wiki-pages)。

请在 `doc/zh` 和 `doc/en` 修改生成页面，下一次同步会按源码覆盖这些页面。
首次同步会替换初始化的首页、侧边栏和页脚；其他手工页面会保留，同名冲突需要人工处理。
删除或重命名页面时，只移除仍符合旧清单的生成文件；失效页面如有手工改动，任务会停止供检查。
内容无变化时不创建提交，推送不会强制覆盖 Wiki 历史。

本地可运行同一同步逻辑并查看差异，以下命令不会发布：

```sh
git clone https://github.com/XS-MLVP/picker.wiki.git /tmp/picker-wiki-checkout
python scripts/sync_wiki.py --export /tmp/picker-wiki-export --checkout /tmp/picker-wiki-checkout
git -C /tmp/picker-wiki-checkout diff
```
