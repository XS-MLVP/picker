"""Exercise the release guard with actual static and dynamic ELF executables."""

import importlib.util
from pathlib import Path
import platform
import subprocess
import tempfile
import unittest
import zipfile


ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("static_wheel", ROOT / "scripts/release/check_static_wheel.py")
static_wheel = importlib.util.module_from_spec(spec)
spec.loader.exec_module(static_wheel)


@unittest.skipUnless(platform.system() == "Linux" and platform.machine() == "x86_64", "Linux x86-64 release guard")
class StaticWheelTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory(prefix="picker-static-test-")
        cls.addClassCleanup(cls.directory.cleanup)
        cls.root = Path(cls.directory.name)
        source = cls.root / "main.c"
        source.write_text("int main(void) { return 0; }\n")
        for name, flags in [("static", ["-static"]), ("dynamic", [])]:
            subprocess.run(["cc", str(source), *flags, "-o", str(cls.root / name)], check=True, capture_output=True)

    def wheel(self, executable, extra=False):
        path = self.root / "test.whl"
        with zipfile.ZipFile(path, "w") as archive:
            archive.write(self.root / executable, static_wheel.EXECUTABLE)
            if extra:
                archive.write(self.root / "dynamic", "xspcomm/_xspcomm.so")
        return path

    def test_fully_static_executable_passes(self):
        static_wheel.check_wheel(self.wheel("static"))

    def test_dynamic_executable_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "dynamically linked"):
            static_wheel.check_wheel(self.wheel("dynamic"))

    def test_bundled_native_bindings_are_rejected(self):
        with self.assertRaisesRegex(ValueError, "native payload"):
            static_wheel.check_wheel(self.wheel("static", extra=True))


if __name__ == "__main__":
    unittest.main()
