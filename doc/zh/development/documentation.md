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

## 导出 GitHub Wiki

网站和 Wiki 都由同一份 Markdown 生成。在空目录导出：

```sh
/tmp/docs-venv/bin/python scripts/docs.py wiki --output /tmp/picker-wiki
```

输出包含双语页面、首页、侧栏、页脚和文件清单，页面链接已转换为 Wiki 名称。维护者审阅后，将这些文件同步到 `picker.wiki.git` 即可。[GitHub Wiki 文档](https://docs.github.com/en/communities/documenting-your-project-with-wikis/adding-or-editing-wiki-pages)介绍独立仓库的编辑方式。

文档 CI 在面向 master 的 PR、master 或 `docs/**` 分支上的文档改动，以及手动运行时执行。它检查双语页面、链接和示例语法，构建网站并导出 Wiki，结果作为 artifacts 提供评审。
