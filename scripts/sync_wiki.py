#!/usr/bin/env python3
"""Apply a verified Wiki export to a checkout, preserving unmanaged files."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_manifest(directory):
    path = directory / 'manifest.json'
    if path.is_symlink():
        raise ValueError('Manifest must not be a symlink')
    data = json.loads(path.read_text(encoding='utf-8'))
    if not isinstance(data, dict):
        raise ValueError('Manifest must be an object')
    folded = set()
    for name, checksum in data.items():
        if not re.fullmatch(r'[\w-]+\.md', name) or name.casefold() in folded:
            raise ValueError(f'Unsafe or colliding Wiki filename: {name}')
        folded.add(name.casefold())
        if not isinstance(checksum, str) or not re.fullmatch(r'[0-9a-f]{64}', checksum):
            raise ValueError(f'Invalid SHA-256 for {name}')
    return data


def sync(export, checkout):
    export, checkout = Path(export).resolve(), Path(checkout).resolve()
    if not export.is_dir() or not checkout.is_dir():
        raise ValueError('Export and Wiki checkout must already exist')
    if export == checkout or export in checkout.parents or checkout in export.parents:
        raise ValueError('Export and checkout must be separate directories')
    current = read_manifest(export)
    if 'Home.md' not in current:
        raise ValueError('Refusing an export without Home.md')
    previous = read_manifest(checkout) if (checkout / 'manifest.json').exists() or (checkout / 'manifest.json').is_symlink() else {}

    # Validate everything before changing the checkout, including deleted pages.
    for name in current.keys() | previous.keys():
        target = checkout / name
        if target.is_symlink() or (target.exists() and not target.is_file()):
            raise ValueError(f'Unsafe Wiki destination: {name}')
    for name, checksum in current.items():
        source = export / name
        if source.is_symlink() or not source.is_file() or digest(source) != checksum:
            raise ValueError(f'Export checksum mismatch: {name}')
        target = checkout / name
        # The first export can replace GitHub's initial Home/navigation pages.
        if name not in previous and name not in {'Home.md', '_Sidebar.md', '_Footer.md'} and target.exists() and digest(target) != checksum:
            raise ValueError(f'Unmanaged Wiki page conflicts with export: {name}')
    removed = previous.keys() - current.keys()
    for name in removed:
        target = checkout / name
        if target.exists() and digest(target) != previous[name]:
            raise ValueError(f'Obsolete generated page has manual edits: {name}')

    for name in removed:
        (checkout / name).unlink(missing_ok=True)
    for name in current:
        shutil.copyfile(export / name, checkout / name)
    shutil.copyfile(export / 'manifest.json', checkout / 'manifest.json')
    print(f'Synced {len(current)} generated pages; removed {len(removed)} obsolete pages.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--export', required=True, type=Path)
    parser.add_argument('--checkout', required=True, type=Path)
    args = parser.parse_args()
    try:
        sync(args.export, args.checkout)
    except (ValueError, OSError) as error:
        raise SystemExit(str(error)) from error


if __name__ == '__main__':
    main()
