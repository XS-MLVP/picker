#!/usr/bin/env python3
"""Validate picker's template variables against their only declaration.

``include/codegen/render_tags.hpp`` declares every variable a template may use,
the typed ``tags::<id>`` constants that producers write through, and the
reference metadata. This script checks that declaration against the templates,
the C++ producers and the generated reference pages.

    python scripts/check_render_tags.py            # verify, fail on drift
    python scripts/check_render_tags.py --docs     # regenerate the reference pages
"""
from __future__ import annotations

import argparse
import collections
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "include" / "codegen" / "render_tags.hpp"
TEMPLATE_DIR = ROOT / "template"
SRC_DIR = ROOT / "src"
DOC_PATHS = {
    "en": ROOT / "doc" / "en" / "reference" / "render-tags.md",
    "zh": ROOT / "doc" / "zh" / "reference" / "render-tags.md",
}

TAG_RE = re.compile(r"__[A-Za-z0-9_]+__")
PUBLIC_RE = re.compile(r"^__[A-Z][A-Z0-9_]*__$")
INTERNAL_RE = re.compile(r"^[a-z][a-z0-9_]*$")
PLACEHOLDER_RE = re.compile(r"\{\{(.*?)\}\}|\{%(.*?)%\}", re.S)

CATEGORY_ORDER = ("meta", "build", "feature", "dependency", "data", "fragment")
SCOPE_TEMPLATE = "template_tag"
SCOPE_EMBEDDED = "embedded"
SCOPE_INTERNAL = "internal"
TYPES = ("string", "boolean", "onoff", "integer", "enumeration", "path", "json", "list", "fragment")

TYPE_LABEL = {"boolean": "bool", "enumeration": "enum", "integer": "int"}

# Tokens that look like tags but belong to other languages, Verilator or the
# compiler. They are never picker template variables.
IGNORED = {
    "__init__": "Python protocol name in generated source.",
    "__name__": "Python protocol name in generated source.",
    "__main__": "Python protocol name in generated source.",
    "__file__": "Python protocol name in generated source.",
    "__package__": "Python protocol name in generated source.",
    "__all__": "Python protocol name in generated source.",
    "__version__": "Python module attribute in generated source.",
    "__pycache__": "Python bytecode directory name used by build rules.",
    "__setattr__": "Python protocol name in generated source.",
    "__getitem__": "Python protocol name in generated source.",
    "__del__": "Python protocol name in generated source.",
    "__repr__": "Python protocol name in generated source.",
    "__class__": "Python protocol name in generated source.",
    "__linux__": "Compiler predefined macro.",
    "__CYGWIN__": "Compiler predefined macro.",
    "__VA_ARGS__": "C/C++ variadic macro keyword.",
    "__DOT__": "Verilator flattened-scope separator in generated member names.",
    "__PVT__": "Verilator member prefix in generated aggregate names.",
}

CATEGORY_TITLES = {
    "en": {
        "meta": "meta — project identity",
        "build": "build — build and runtime configuration",
        "feature": "feature — feature switches",
        "dependency": "dependency — xspcomm and toolchain locations",
        "data": "data — structured values consumed by loops",
        "fragment": "fragment — pre-rendered source text",
    },
    "zh": {
        "meta": "meta — 项目身份",
        "build": "build — 构建与运行配置",
        "feature": "feature — 功能开关",
        "dependency": "dependency — xspcomm 与工具链位置",
        "data": "data — 模板循环使用的结构化数据",
        "fragment": "fragment — 预渲染源码片段",
    },
}


def rel(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="ignore")


def split_arguments(text: str) -> list[str]:
    """Split a macro argument list, respecting quotes and nested parentheses."""
    parts: list[str] = []
    current: list[str] = []
    depth = 0
    quoted = False
    index = 0
    while index < len(text):
        char = text[index]
        if quoted:
            current.append(char)
            if char == "\\" and index + 1 < len(text):
                current.append(text[index + 1])
                index += 2
                continue
            if char == '"':
                quoted = False
        elif char == '"':
            quoted = True
            current.append(char)
        elif char == "(":
            depth += 1
            current.append(char)
        elif char == ")":
            depth -= 1
            current.append(char)
        elif char == "," and depth == 0:
            parts.append("".join(current).strip())
            current = []
        else:
            current.append(char)
        index += 1
    parts.append("".join(current).strip())
    return parts


def parse_header() -> list[dict]:
    text = read(HEADER).replace("\\\n", " ")
    entries: list[dict] = []
    index = 0
    while True:
        start = text.find("X(", index)
        if start < 0:
            break
        cursor = start + 2
        depth, quoted = 1, False
        while cursor < len(text) and depth:
            char = text[cursor]
            if quoted:
                if char == "\\":
                    cursor += 2
                    continue
                if char == '"':
                    quoted = False
            elif char == '"':
                quoted = True
            elif char == "(":
                depth += 1
            elif char == ")":
                depth -= 1
            cursor += 1
        arguments = split_arguments(text[start + 2 : cursor - 1])
        index = cursor
        if len(arguments) != 7:
            continue  # the PICKER_RENDER_TAGS(X) parameter itself
        ident, wire, category, value_type, scope, summary, summary_zh = arguments
        if not summary.startswith('"') or not summary_zh.startswith('"'):
            continue
        entries.append(
            {
                "id": ident.strip(),
                "name": json.loads(wire),
                "category": category.strip(),
                "type": value_type.strip(),
                "scope": scope.strip(),
                "description": json.loads(summary),
                "description_zh": json.loads(summary_zh),
            }
        )
    return entries


# Producers live in src/codegen/, and shared inja fragments also live under
# include/codegen/ (for example include/codegen/sv/templates.hpp). Other parser
# code has tag-shaped sentinels of its own and is not part of the contract.
SOURCE_DIRS = (ROOT / "src" / "codegen", ROOT / "include" / "codegen")


def source_files() -> list[Path]:
    return sorted(p for directory in SOURCE_DIRS for p in directory.rglob("*")
                  if p.suffix in (".cpp", ".hpp"))


def template_files() -> list[Path]:
    return sorted(p for p in TEMPLATE_DIR.rglob("*") if p.is_file())


def scan_templates() -> dict[str, set[str]]:
    found: dict[str, set[str]] = collections.defaultdict(set)
    for path in template_files():
        for tag in TAG_RE.findall(read(path)):
            found[tag].add(rel(path))
    return found


def scan_producers(entries: list[dict]) -> dict[str, set[str]]:
    """Files that write a tag through its typed constant: data[tags::<id>]."""
    found: dict[str, set[str]] = collections.defaultdict(set)
    usages = {entry["id"]: re.compile(rf"\btags::{re.escape(entry['id'])}\b") for entry in entries}
    for path in source_files():
        if path == HEADER:
            continue
        text = read(path)
        for ident, pattern in usages.items():
            if pattern.search(text):
                found[ident].add(rel(path))
    return found


def scan_embedded(entries: list[dict]) -> dict[str, set[str]]:
    """Files whose inja strings embedded in C++ reference the wire name."""
    found: dict[str, set[str]] = collections.defaultdict(set)
    wires = {entry["name"]: entry["id"] for entry in entries if entry["scope"] == SCOPE_EMBEDDED}
    for path in source_files():
        if path == HEADER:
            continue
        for placeholder in PLACEHOLDER_RE.finditer(read(path)):
            body = placeholder.group(1) or placeholder.group(2) or ""
            for tag in TAG_RE.findall(body):
                if tag in wires:
                    found[wires[tag]].add(rel(path))
    return found


def scan_usages(entries: list[dict]) -> dict[str, set[str]]:
    """Every place a tag is referenced, in template/ or in an inja placeholder.

    Split from scan_templates() on purpose: scope decides where a tag *may* be
    used, while the reference page should list where it *is* used, including
    first-party headers such as include/codegen/sv/templates.hpp.
    """
    found: dict[str, set[str]] = collections.defaultdict(set)
    wires = {entry["name"]: entry["id"] for entry in entries}
    for path in template_files():
        for tag in TAG_RE.findall(read(path)):
            if tag in wires:
                found[wires[tag]].add(rel(path))
    for path in source_files():
        if path == HEADER:
            continue
        for placeholder in PLACEHOLDER_RE.finditer(read(path)):
            body = placeholder.group(1) or placeholder.group(2) or ""
            for tag in TAG_RE.findall(body):
                if tag in wires:
                    found[wires[tag]].add(rel(path))
    return found


def validate(entries: list[dict]) -> list[str]:
    errors: list[str] = []
    if not entries:
        return [f"{rel(HEADER)}: no tags parsed; the PICKER_RENDER_TAGS list is empty or malformed"]

    ids = collections.Counter(entry["id"] for entry in entries)
    wires = collections.Counter(entry["name"] for entry in entries)
    for ident, count in ids.items():
        if count > 1:
            errors.append(f"{rel(HEADER)}: duplicate id {ident}")
    for wire, count in wires.items():
        if count > 1:
            errors.append(f"{rel(HEADER)}: duplicate wire name {wire}")

    for entry in entries:
        ident, wire, scope = entry["id"], entry["name"], entry["scope"]
        if not INTERNAL_RE.match(ident):
            errors.append(f"{rel(HEADER)}: {ident} is not a snake_case id")
        if entry["category"] not in CATEGORY_ORDER:
            errors.append(f"{rel(HEADER)}: {ident} has unknown category {entry['category']}")
        if entry["type"] not in TYPES:
            errors.append(f"{rel(HEADER)}: {ident} has unknown type {entry['type']}")
        if scope not in (SCOPE_TEMPLATE, SCOPE_EMBEDDED, SCOPE_INTERNAL):
            errors.append(f"{rel(HEADER)}: {ident} has unknown scope {scope}")
        if scope == SCOPE_INTERNAL:
            if wire != ident:
                errors.append(f"{rel(HEADER)}: internal key {ident} must use its plain name, not {wire}")
        elif not PUBLIC_RE.match(wire):
            errors.append(f"{rel(HEADER)}: {ident} wire name {wire} must match __UPPER_SNAKE__")
        if not entry["description"] or not entry["description_zh"]:
            errors.append(f"{rel(HEADER)}: {ident} is missing a summary")
        elif entry["description"] == entry["description_zh"]:
            errors.append(f"{rel(HEADER)}: {ident} has identical English and Chinese summaries")

    templates = scan_templates()
    producers = scan_producers(entries)
    embedded = scan_embedded(entries)
    by_id = {entry["id"]: entry for entry in entries}

    for tag, files in sorted(templates.items()):
        if tag in IGNORED:
            continue
        match = next((entry for entry in entries if entry["name"] == tag), None)
        if match is None:
            errors.append(f"template uses undeclared tag {tag}: {', '.join(sorted(files))}")
        elif match["scope"] != SCOPE_TEMPLATE:
            errors.append(f"{match['id']}: declared as {match['scope']} but referenced from template/")

    for entry in entries:
        ident, wire, scope = entry["id"], entry["name"], entry["scope"]
        if scope == SCOPE_TEMPLATE:
            if not templates.get(wire):
                errors.append(f"{ident}: declared but no template references {wire}")
            if not producers.get(ident):
                errors.append(f"{ident}: declared but no producer writes tags::{ident}")
        elif scope == SCOPE_EMBEDDED:
            if templates.get(wire):
                errors.append(f"{ident}: embedded tag must not appear in template/")
            if not embedded.get(ident):
                errors.append(f"{ident}: embedded tag has no inja placeholder in src/")
            if not producers.get(ident):
                errors.append(f"{ident}: declared but no producer writes tags::{ident}")
        elif scope == SCOPE_INTERNAL:
            if templates.get(wire) or any(re.search(rf"__{re.escape(wire)}__", read(p)) for p in
                                          sorted(TEMPLATE_DIR.rglob("*")) if p.is_file()):
                errors.append(f"{ident}: internal key must not appear in template/")
            if not producers.get(ident):
                errors.append(f"{ident}: declared but no producer writes tags::{ident}")

    for path in source_files():
        if path == HEADER:
            continue
        text = read(path)
        spans = [m.span(1) if m.group(1) is not None else m.span(2)
                 for m in PLACEHOLDER_RE.finditer(text)]
        for match in TAG_RE.finditer(text):
            tag = match.group(0)
            if tag in IGNORED:
                continue
            entry = by_id.get(next((e["id"] for e in entries if e["name"] == tag), ""))
            if entry is None:
                # A public-looking name in a source placeholder is a typo in a
                # producer or a fragment that was never declared. The gsim
                # backend prefixes unsupported values with a sentinel that is
                # itself tag-shaped; skip that composite.
                if PUBLIC_RE.match(tag) and "__GISM_NOT_SUPPORT__" not in tag:
                    errors.append(f"{rel(path)}: source references undeclared tag {tag}")
                continue
            if entry["scope"] == SCOPE_INTERNAL:
                continue
            # Every occurrence must sit inside a placeholder. A single correct
            # occurrence must not mask a typo or a literal elsewhere.
            in_placeholder = any(start <= match.start() and match.end() <= end
                                 for start, end in spans)
            if entry and not in_placeholder:
                errors.append(f"{rel(path)}: literal {tag} must use tags::{entry['id']} instead")
    return errors


def short_source(path: str) -> str:
    parts = Path(path).parts
    if "codegen" in parts:
        tail = parts[parts.index("codegen") + 1 :]
        return tail[0] if len(tail) > 1 else Path(tail[0]).stem
    return Path(path).stem


def short_template(path: str) -> str:
    return path[len("template/") :] if path.startswith("template/") else path


def short_usage(path: str) -> str:
    """Template paths keep their layout; C++ is shortened to its module name."""
    return short_template(path) if path.startswith("template/") else short_source(path)


def summarize(items: list[str], formatter, limit: int = 3) -> str:
    # Two files can shorten to the same label (sv/sv.cpp and sv/templates.hpp
    # both become "sv"), so deduplicate after formatting.
    names: list[str] = []
    for item in sorted(items):
        name = formatter(item)
        if name not in names:
            names.append(name)
    if not names:
        return "—"
    shown = ", ".join(f"`{name}`" for name in names[:limit])
    if len(names) > limit:
        shown += f" +{len(names) - limit}"
    return shown


def render_docs(entries: list[dict], language: str) -> str:
    zh = language == "zh"
    producers = scan_producers(entries)
    embedded = scan_embedded(entries)
    usages = scan_usages(entries)

    if zh:
        lines = [
            "# 模板渲染变量",
            "",
            "<!-- 由 scripts/check_render_tags.py 从 include/codegen/render_tags.hpp 生成，请勿手工编辑。 -->",
            "",
            "模板由 [inja](https://github.com/pantor/inja) 渲染。本页列出代码生成模板可以使用的全部变量",
            "（`template/` 与 `src/codegen/` 内嵌模板）；`include/codegen/render_tags.hpp` 中的",
            "`PICKER_RENDER_TAGS` 列表是唯一声明处，`scripts/check_render_tags.py` 负责校验声明、",
            "生产者与本页是否一致。UVM 事务模板由 `src/parser/uvm.cpp` 用自有的 `TemplateVars` 渲染，",
            "不在此契约内。",
            "",
            "## 约定",
            "",
            "| 类别 | 约定 |",
            "|---|---|",
            "| 公共变量 | `__UPPER_SNAKE__`，模板只能引用这一类名字 |",
            "| `bool` | 渲染为 `true` / `false` |",
            "| `onoff` | 渲染为字符串 `ON` / `OFF` |",
            "| `fragment` | 预渲染源码文本，模板只能注入，不能重排 |",
            "| 内部键 | 普通 `snake_case`，不会出现在模板中 |",
            "",
            "## 维护流程",
            "",
            "1. 在 `include/codegen/render_tags.hpp` 的 `PICKER_RENDER_TAGS` 中增加一行。",
            "2. 在 `src/codegen/` 中用 `data[tags::<id>]` 赋值，在 `template/` 中用 `{{__NAME__}}` 引用。",
            "3. 运行 `python scripts/check_render_tags.py --docs` 重新生成本页。",
            "4. 提交前运行 `python scripts/check_render_tags.py`，CI 会执行同一检查。",
            "",
            f"当前共有 {len(entries)} 个登记项。",
            "",
        ]
    else:
        lines = [
            "# Render tags",
            "",
            "<!-- Generated by scripts/check_render_tags.py from include/codegen/render_tags.hpp. Do not edit by hand. -->",
            "",
            "Templates are rendered by [inja](https://github.com/pantor/inja). This page lists every variable",
            "a codegen template may use (`template/` and the inja strings embedded in `src/codegen/`). The",
            "`PICKER_RENDER_TAGS` list in `include/codegen/render_tags.hpp` is the only place they are",
            "declared; `scripts/check_render_tags.py` verifies that the declaration, the producers and this",
            "page agree. UVM transaction templates are rendered by `src/parser/uvm.cpp` from its own",
            "`TemplateVars` and are outside this contract.",
            "",
            "## Conventions",
            "",
            "| Kind | Convention |",
            "|---|---|",
            "| Public tag | `__UPPER_SNAKE__`; the only names templates may reference |",
            "| `bool` | Rendered as `true` / `false` |",
            "| `onoff` | Rendered as the string `ON` / `OFF` |",
            "| `fragment` | Pre-rendered source text; inject it, do not restructure it |",
            "| Internal key | Plain `snake_case`; never visible to templates |",
            "",
            "## Workflow",
            "",
            "1. Add a row to `PICKER_RENDER_TAGS` in `include/codegen/render_tags.hpp`.",
            "2. Assign it with `data[tags::<id>]` under `src/codegen/` and reference `{{__NAME__}}` from `template/`.",
            "3. Run `python scripts/check_render_tags.py --docs` to regenerate this page.",
            "4. Run `python scripts/check_render_tags.py` before committing; CI runs the same check.",
            "",
            f"{len(entries)} entries are declared.",
            "",
        ]

    header = "| 变量 | 类型 | 说明 | 生产者 | 使用方 |" if zh else "| Tag | Type | Description | Producers | Used by |"
    divider = "|---|---|---|---|---|"

    def row(entry: dict, produced: set[str], consumed: set[str], consumer_formatter) -> str:
        description = entry["description_zh"] if zh else entry["description"]
        return "| `{}` | `{}` | {} | {} | {} |".format(
            entry["name"],
            TYPE_LABEL.get(entry["type"], entry["type"]),
            description,
            summarize(produced, short_source),
            summarize(consumed, consumer_formatter),
        )

    for category in CATEGORY_ORDER:
        group = [e for e in entries if e["category"] == category and e["scope"] == SCOPE_TEMPLATE]
        if not group:
            continue
        lines += [f"## {CATEGORY_TITLES[language][category]} ({len(group)})", "", header, divider]
        for entry in group:
            lines.append(row(entry, producers.get(entry["id"], set()), usages.get(entry["id"], set()),
                             short_usage))
        lines.append("")

    embedded_group = [e for e in entries if e["scope"] == SCOPE_EMBEDDED]
    if embedded_group:
        lines += ["## 内嵌模板变量", ""] if zh else ["## Embedded template tags", ""]
        lines += [
            "仅用于 `src/codegen/` 内嵌 inja 模板的变量，模板目录不引用。"
            if zh
            else "Used only by inja templates embedded in `src/codegen/`; the template directory does not reference them.",
            "",
            header,
            divider,
        ]
        for entry in embedded_group:
            lines.append(row(entry, producers.get(entry["id"], set()), embedded.get(entry["id"], set()),
                             short_source))
        lines.append("")

    internal_group = [e for e in entries if e["scope"] == SCOPE_INTERNAL]
    if internal_group:
        lines += ["## 内部键", ""] if zh else ["## Internal keys", ""]
        lines += [
            "只在 C++ 内部传递的 JSON 键，不属于模板契约。"
            if zh
            else "JSON keys passed between C++ stages. They are not part of the template contract.",
            "",
            "| 键 | 类型 | 说明 | 文件 |" if zh else "| Key | Type | Description | Files |",
            "|---|---|---|---|",
        ]
        for entry in internal_group:
            description = entry["description_zh"] if zh else entry["description"]
            lines.append("| `{}` | `{}` | {} | {} |".format(
                entry["id"], TYPE_LABEL.get(entry["type"], entry["type"]), description,
                summarize(producers.get(entry["id"], set()), short_source, limit=6)))
        lines.append("")

    return "\n".join(lines).rstrip() + "\n"


def check_docs(entries: list[dict]) -> list[str]:
    errors = []
    for language, path in DOC_PATHS.items():
        expected = render_docs(entries, language)
        if not path.exists():
            errors.append(f"docs: missing {rel(path)}; run --docs")
        elif read(path) != expected:
            errors.append(f"docs: {rel(path)} is stale; run --docs")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--docs", action="store_true", help="regenerate the reference pages")
    args = parser.parse_args()

    entries = parse_header()
    errors = validate(entries)
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1

    if args.docs:
        for language, path in DOC_PATHS.items():
            path.parent.mkdir(parents=True, exist_ok=True)
            # Pin the line ending: the generated pages are committed and must
            # not differ between Windows and Linux checkouts.
            path.write_text(render_docs(entries, language), encoding="utf-8", newline="\n")
        print(f"Regenerated {len(DOC_PATHS)} reference pages from {rel(HEADER)}")
        return 0

    errors = check_docs(entries)
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1

    public = sum(1 for entry in entries if entry["scope"] == SCOPE_TEMPLATE)
    print(
        f"Checked {public} template tags, {len(entries) - public} embedded/internal keys, "
        f"{len(IGNORED)} ignored tokens and 2 generated pages; declaration, producers, "
        "templates and docs match."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
