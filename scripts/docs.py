#!/usr/bin/env python3
"""Check bilingual documentation, run marked examples, and export a local Wiki."""
import argparse
import ast
import hashlib
from html.parser import HTMLParser
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
from urllib.parse import quote, unquote, urlsplit

import markdown
import yaml

ROOT = Path(__file__).resolve().parents[1]
CONFIG = yaml.load((ROOT / 'mkdocs.yml').read_text(), Loader=yaml.BaseLoader)
DOCS = ROOT / CONFIG['docs_dir']
EXAMPLES = re.compile(r'<!-- runnable:(python|cpp) -->\s*```(?:python|cpp)\n(.*?)\n```', re.S)
LINK = re.compile(r'(!?\[[^\]\n]*\]\()([^\s)]+)(\))')


class Links(HTMLParser):
    def __init__(self, text):
        super().__init__()
        self.targets, self.ids = [], set()
        self.feed(markdown.markdown(text, extensions=['toc', 'tables', 'fenced_code']))

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if 'id' in attrs:
            self.ids.add(attrs['id'])
        key = 'src' if tag == 'img' else 'href'
        if key in attrs:
            self.targets.append(attrs[key])


def nav_items(nav):
    for entry in nav:
        for title, value in entry.items():
            if isinstance(value, list):
                yield from nav_items(value)
            else:
                yield title, value


def pages():
    return sorted(p for lang in ('zh', 'en') for p in (DOCS / lang).rglob('*.md'))


def check():
    localized = {lang: {p.relative_to(DOCS / lang) for p in (DOCS / lang).rglob('*.md')}
                 for lang in ('zh', 'en')}
    errors = []
    if localized['zh'] != localized['en']:
        errors.append('Missing translations: ' + str(localized['zh'] ^ localized['en']))
    navigation = {Path(path) for _, path in nav_items(CONFIG['nav'])}
    for lang, files in localized.items():
        if files != navigation:
            errors.append(f'{lang}: navigation/page mismatch: {files ^ navigation}')
    all_pages = pages()
    entrypoints = list(ROOT.glob('README*.md')) + list(ROOT.glob('CONTRIBUTING.md'))
    if (ROOT / 'test/README.md').exists():
        entrypoints.append(ROOT / 'test/README.md')
    parsed = {p.resolve(): Links(p.read_text()) for p in [*DOCS.rglob('*.md'), *entrypoints]}
    link_count = 0
    for path, data in parsed.items():
        for target in data.targets:
            url = urlsplit(target)
            if url.scheme or url.netloc:
                continue
            destination = (path.parent / unquote(url.path)).resolve() if url.path else path
            if not destination.exists():
                errors.append(f'{path.relative_to(ROOT)}: missing {target}')
            elif url.fragment and destination in parsed and unquote(url.fragment) not in parsed[destination].ids:
                errors.append(f'{path.relative_to(ROOT)}: missing anchor {target}')
            link_count += 1
    for path in all_pages:
        content = path.read_text()
        if not content.startswith('# ') or len(content.splitlines()) < 8:
            errors.append(f'{path.relative_to(ROOT)}: missing title or substantive content')
        for code in re.findall(r'^```python\n(.*?)^```', content, re.M | re.S):
            try:
                ast.parse(code)
            except SyntaxError as error:
                errors.append(f'{path.relative_to(ROOT)}: Python syntax: {error}')
    for relative in sorted(localized['zh'] & localized['en']):
        zh = EXAMPLES.findall((DOCS / 'zh' / relative).read_text())
        en = EXAMPLES.findall((DOCS / 'en' / relative).read_text())
        if zh != en:
            errors.append(f'{relative}: runnable examples differ between translations')
    if errors:
        raise SystemExit('\n'.join(errors))
    print(f'Checked {len(all_pages)} localized pages and {link_count} local links; translations and examples match.')
    return all_pages


def run_examples(args):
    checked = check()
    count, skipped = 0, 0
    for page in checked:
        if page.relative_to(DOCS).parts[0] != 'zh':
            continue  # check() established identical executable examples.
        for language, code in EXAMPLES.findall(page.read_text()):
            with tempfile.TemporaryDirectory(prefix='docs-example-') as directory:
                directory = Path(directory)
                if language == 'python':
                    command = [args.python, '-c', code]
                elif args.include and args.lib:
                    source, executable = directory / 'example.cpp', directory / 'example'
                    source.write_text(code)
                    subprocess.run([args.cxx, '-std=c++20', '-O2', '-I', str(Path(args.include).resolve()),
                                    str(source), '-L', str(Path(args.lib).resolve()),
                                    '-Wl,-rpath,' + str(Path(args.lib).resolve()), '-lxspcomm',
                                    '-o', str(executable)], check=True, timeout=120)
                    command = [str(executable)]
                else:
                    print(f'SKIP C++ {page}: supply --include and --lib')
                    skipped += 1
                    continue
                subprocess.run(command, check=True, cwd=args.cwd or directory, timeout=60)
                count += 1
                print(f'PASS {language}: {page.relative_to(DOCS)}')
    print(f'{count} executable examples passed, {skipped} skipped (shared by both languages).')


def wiki_slug(page):
    language, *parts = page.relative_to(DOCS).parts
    relative = Path(*parts)
    return language + '-' + ('Home' if relative == Path('index.md') else relative.with_suffix('').as_posix().replace('/', '-'))


def wiki_url(slug):
    return CONFIG['repo_url'].rstrip('/') + '/wiki/' + quote(slug)


def export_wiki(args):
    checked = check()
    output = Path(args.output).resolve()
    if output.exists() and any(output.iterdir()):
        raise SystemExit('Choose an empty output directory; Wiki export does not overwrite existing work.')
    output.mkdir(parents=True, exist_ok=True)
    slugs = {p.resolve(): wiki_slug(p) for p in checked}
    if len(set(slugs.values())) != len(slugs):
        raise SystemExit('Wiki page names collide')
    for page in checked:
        def replace_link(match):
            url = urlsplit(match[2])
            if url.scheme or url.netloc or not url.path:
                return match[0]
            target = (page.parent / unquote(url.path)).resolve()
            if target in slugs:
                destination = wiki_url(slugs[target])
            else:
                route = '/raw/refs/heads/master/' if match[1].startswith('!') else '/blob/master/'
                destination = CONFIG['repo_url'].rstrip('/') + route + quote(target.relative_to(ROOT).as_posix())
            if url.fragment:
                destination += '#' + url.fragment
            return match[1] + destination + match[3]
        chunks = re.split(r'(^```.*?^```[^\n]*$)', page.read_text(), flags=re.M | re.S)
        for index in range(0, len(chunks), 2):
            chunks[index] = LINK.sub(replace_link, chunks[index])
        relative = page.relative_to(DOCS)
        counterpart = DOCS / ('en' if relative.parts[0] == 'zh' else 'zh') / Path(*relative.parts[1:])
        label = 'English' if relative.parts[0] == 'zh' else '中文'
        source = CONFIG['repo_url'].rstrip('/') + '/blob/master/' + page.relative_to(ROOT).as_posix()
        content = f'[{label}]({wiki_url(slugs[counterpart.resolve()])}) · [源文档 / Source]({source})\n\n' + ''.join(chunks)
        (output / (slugs[page.resolve()] + '.md')).write_text(content)
    (output / 'Home.md').write_text('# ' + CONFIG['site_name'] + f'\n\n[中文]({wiki_url("zh-Home")}) | [English]({wiki_url("en-Home")})\n')
    sidebar = [f'[Home]({wiki_url("Home")})', '']
    translations = CONFIG['plugins'][0]['i18n']['languages'][1].get('nav_translations', {})
    for language, name in [('zh', '中文'), ('en', 'English')]:
        sidebar += ['## ' + name, '']
        def add_navigation(entries, indent=''):
            for entry in entries:
                for title, value in entry.items():
                    label = translations.get(title, title) if language == 'en' else title
                    if isinstance(value, list):
                        sidebar.append(f'{indent}- **{label}**')
                        add_navigation(value, indent + '  ')
                    else:
                        sidebar.append(f'{indent}- [{label}]({wiki_url(slugs[(DOCS / language / value).resolve()])})')
        add_navigation(CONFIG['nav'])
        sidebar.append('')
    (output / '_Sidebar.md').write_text('\n'.join(sidebar))
    (output / '_Footer.md').write_text('来源 / Source: [' + CONFIG['repo_url'] + '](' + CONFIG['repo_url'] + '). '
                                      '内容在主仓库维护 / Content is maintained in the source repository.\n')
    for page in output.glob('*.md'):
        for target in Links(page.read_text()).targets:
            url = urlsplit(target)
            if not (url.scheme or url.netloc) and url.path:
                raise SystemExit(f'Relative exported Wiki link: {page.name}: {target}')
            if target.startswith(wiki_url('')) and not (output / (unquote(url.path.rsplit('/', 1)[-1]) + '.md')).exists():
                raise SystemExit(f'Broken exported Wiki link: {page.name}: {target}')
    manifest = {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(output.glob('*.md'))}
    (output / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print(f'Exported {len(manifest)} Wiki pages to {output}; no remote repository was modified.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='action', required=True)
    sub.add_parser('check')
    test = sub.add_parser('test')
    test.add_argument('--python', default=sys.executable)
    test.add_argument('--cwd', help='Working directory for examples importing a generated DUT')
    test.add_argument('--cxx', default='c++')
    test.add_argument('--include', help='Installed xcomm SDK include directory')
    test.add_argument('--lib', help='Installed xcomm native library directory')
    wiki = sub.add_parser('wiki')
    wiki.add_argument('--output', required=True)
    args = parser.parse_args()
    if args.action == 'check':
        check()
    elif args.action == 'test':
        run_examples(args)
    else:
        export_wiki(args)


if __name__ == '__main__':
    main()
