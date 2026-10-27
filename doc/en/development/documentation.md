# Maintain documentation and Wiki

Chinese pages live in `doc/zh` and English pages in `doc/en`, with matching relative paths. Update both languages when editing content. Maintain navigation in `mkdocs.yml`.

## Organize content around reader needs

Use the four documentation types from [Diátaxis](https://diataxis.fr/):

| Type | Reader need | Writing approach |
| --- | --- | --- |
| Tutorial | Complete an exercise with guidance | Give small, connected steps, each with an observable result |
| How-to guide | Complete a known task | Describe actions, necessary choices, and result checks, such as “Save waveforms” |
| Reference | Look up a parameter or method | Group by interface and state signatures, defaults, return values, and conditions precisely |
| Explanation | Understand design and behavior | Explain object relationships, call flow, and design decisions |

In tutorials, let readers act and see a result before introducing the concepts needed for the next step. Link detailed parameters to reference pages and design reasoning to explanations.

Use specific actions in titles and describe the procedure directly. For example: “Call `Finish()` to complete output handling, then open the waveform file.” Place limitations beside the relevant operation or interface, with their triggering conditions and effects. Follow the direct, clear style in the [Google technical writing guide](https://developers.google.com/style/highlights).

## Write examples

State the command's working directory, the script's file path, and expected output. Complete scripts should run when copied; fragments should identify their existing objects and environment. Use identical executable code in Chinese and English, with explanations in prose.

## Preview the site

From the repository root:

```sh
python3 -m venv /tmp/docs-venv
/tmp/docs-venv/bin/python -m pip install -r requirements-docs.txt
/tmp/docs-venv/bin/python scripts/docs.py check
/tmp/docs-venv/bin/python -m mkdocs build --strict
/tmp/docs-venv/bin/python -m mkdocs serve -a 127.0.0.1:8000
```

Open `http://127.0.0.1:8000` to check language switching, navigation, full-text search, and code rendering. The static site is written to `build/docs-site`.

`check` validates translated page sets, navigation, links, anchors, and Python syntax. Mark complete executable blocks with `<!-- runnable:python -->` or `<!-- runnable:cpp -->`, then run:

```sh
/tmp/docs-venv/bin/python scripts/docs.py test --python /path/to/runtime/bin/python
```

Picker examples need the tutorial's Adder, an Adder with waveforms and coverage, and the internal-signal guide's vpi model. Put the recording-enabled Adder and vpi packages in one directory and pass that directory with `--cwd`.

## Export a GitHub Wiki

The site and Wiki are generated from the same Markdown. Export into an empty directory:

```sh
/tmp/docs-venv/bin/python scripts/docs.py wiki --output /tmp/picker-wiki
```

The output includes bilingual pages, a home page, sidebar, footer, and file manifest. Links are converted to Wiki page names. After review, maintainers can sync these files to `picker.wiki.git`. See [GitHub's Wiki documentation](https://docs.github.com/en/communities/documenting-your-project-with-wikis/adding-or-editing-wiki-pages) for editing the separate repository.

Documentation CI runs for PRs targeting master, documentation changes pushed to master or `docs/**` branches, and manual dispatches. It checks translated pages, links, and example syntax, then builds the site and exports the Wiki as review artifacts.
