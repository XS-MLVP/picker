"""Keep the source-build CI package version inside the consumer dependency range."""

from pathlib import Path
import re
import tomllib
import unittest


ROOT = Path(__file__).resolve().parents[2]


class DependencyVersionContractTests(unittest.TestCase):
    def test_ci_xcomm_version_satisfies_the_package_requirement(self):
        project = tomllib.loads((ROOT / "pyproject.toml").read_text())
        requirement = next(item for item in project["project"]["dependencies"]
                           if item.startswith("uc-xcomm"))
        bounds = re.fullmatch(r"uc-xcomm>=(\d+)\.(\d+),<(\d+)", requirement)
        self.assertIsNotNone(bounds)
        lower = tuple(map(int, bounds.group(1, 2))) + (0,)
        upper = (int(bounds.group(3)), 0, 0)
        workflow = (ROOT / ".github/workflows/ci.yml").read_text()
        versions = re.findall(
            r"SETUPTOOLS_SCM_PRETEND_VERSION_FOR_UC_XCOMM:\s*['\"]?(\d+\.\d+\.\d+)",
            workflow,
        )
        self.assertTrue(versions)
        for version in versions:
            with self.subTest(version=version):
                actual = tuple(map(int, version.split(".")))
                self.assertGreaterEqual(actual, lower)
                self.assertLess(actual, upper)


if __name__ == "__main__":
    unittest.main()
