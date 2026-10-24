#!/usr/bin/env python3
"""Reject release wheels whose Picker executable is not fully static x86-64."""

import argparse
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile
import zipfile


EXECUTABLE = "picker/bin/picker"


def check_wheel(path: Path) -> None:
    with zipfile.ZipFile(path) as wheel:
        elf_files = []
        for name in wheel.namelist():
            with wheel.open(name) as member:
                header = member.read(20)
            if header.startswith(b"\x7fELF"):
                elf_files.append(name)
                if len(header) < 20 or header[4:6] != b"\x02\x01" or struct.unpack_from("<H", header, 18)[0] != 62:
                    raise ValueError(f"{name}: expected a 64-bit little-endian x86-64 ELF")
        if elf_files != [EXECUTABLE]:
            raise ValueError(f"expected only {EXECUTABLE} as native payload, found {elf_files}")
        if not (wheel.getinfo(EXECUTABLE).external_attr >> 16) & 0o111:
            raise ValueError(f"{EXECUTABLE} is not executable")
        with tempfile.TemporaryDirectory(prefix="picker-static-check-") as directory:
            executable = Path(directory) / "picker"
            executable.write_bytes(wheel.read(EXECUTABLE))
            env = dict(os.environ, LC_ALL="C")
            program_headers = subprocess.check_output(["readelf", "-lW", str(executable)], text=True, env=env)
            dynamic = subprocess.check_output(["readelf", "-dW", str(executable)], text=True, env=env)
            versions = subprocess.check_output(["readelf", "-VW", str(executable)], text=True, env=env)
            if re.search(r"^\s*INTERP\s", program_headers, re.MULTILINE) or "(NEEDED)" in dynamic:
                raise ValueError("Picker is dynamically linked; release wheels require a fully static executable")
            if "Version needs section" in versions:
                raise ValueError("Picker still requires versioned symbols from shared libraries")
    print(f"{path.name}: fully static x86-64 Picker; no bundled native bindings")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("wheels", type=Path, nargs="+")
    args = parser.parse_args()
    for path in args.wheels:
        try:
            check_wheel(path)
        except ValueError as error:
            parser.exit(1, f"{path.name}: {error}\n")


if __name__ == "__main__":
    main()
