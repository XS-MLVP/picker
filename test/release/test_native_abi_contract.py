"""Keep install/release checks aligned with the SDK ABI required by Picker."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[2]


class NativeABIContractTests(unittest.TestCase):
    def required_abi(self):
        source = (ROOT / "CMakeLists.txt").read_text()
        match = re.search(r"set\(PICKER_NATIVE_ABI\s+(\d+)\)", source)
        self.assertIsNotNone(match)
        return int(match.group(1))

    def test_wheel_verifier_checks_the_required_abi(self):
        source = (ROOT / "scripts/build/verify-wheel.sh").read_text()
        match = re.search(r"assert xspcomm\.abi_version\(\) == (\d+)", source)
        self.assertIsNotNone(match)
        self.assertEqual(int(match.group(1)), self.required_abi())

    def test_native_release_checks_the_required_soname(self):
        source = (ROOT / ".github/workflows/release-build.yml").read_text()
        match = re.search(r"test -e AppDir/usr/lib/libxspcomm\.so\.(\d+)", source)
        self.assertIsNotNone(match)
        self.assertEqual(int(match.group(1)), self.required_abi())


if __name__ == "__main__":
    unittest.main()
