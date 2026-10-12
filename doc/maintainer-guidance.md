# Maintaining Picker

## Versions and dependencies

Picker and xcomm have independent `vX.Y.Z` package tags. Picker derives its
package, CLI and CMake release versions from its own tag using setuptools-scm.
Release a compatible xcomm version before releasing Picker.

| Setting | Source | Current requirement |
| --- | --- | --- |
| Development xcomm source | `.build-config.yml` | Exact tested commit; `XCOMM_SOURCE_REF` can override it |
| Python dependency | `pyproject.toml` | `xspcomm>=1.0.0.dev0,<2` |
| Development xcomm wheel | Development CI | `1.0.0.dev0` preview until xcomm 1.0 is released |
| Native ABI | `xspcomm.abi_version()` | 3; packages contain `libxspcomm.so.3` |
| Coverage descriptors | `XTriggerEngine.CoverageVersion()` | 5 |

Package version, native ABI and descriptor protocol are separate identifiers.
When updating xcomm, align the source pin, Python range and preview version,
then rebuild native clients and generated DUT extensions against matching
headers and runtime. ABI checks cannot be replaced by a package version range.

## Local build and validation

Use a compiler, CMake, SWIG >= 4.2, Python development headers and a JDK
for Java tests. Run `make init` when source dependencies are missing. Build
from a clean checkout with a fresh wheel output directory; `make wheel` does
not align an existing xcomm checkout to the dependency pin or install packages
into the interpreter.

```sh
python3 -m venv /tmp/picker-build
/tmp/picker-build/bin/python -m pip install build
make unit_tests BUILD_XSPCOMM_SWIG=python,java
SETUPTOOLS_SCM_PRETEND_VERSION_FOR_XSPCOMM=1.0.0.dev0 \
    make wheel PYTHON=/tmp/picker-build/bin/python
bash scripts/build/verify-wheel.sh
make -C test all
```

The preview override affects only xcomm. Picker keeps its Git-derived version.
CI's native build script refuses to switch an xcomm checkout with local changes.
Keep `-O3`, the native `-fno-strict-aliasing` flags and signal-value regressions
enabled. Wheel verification installs both packages into a fresh environment and
checks dependency consistency, native APIs, clock ownership and CLI exit codes.

## Release procedure

1. Publish the compatible xcomm release. Tagged CI selects a published stable
   release satisfying `pyproject.toml`; it never uses a development preview.
2. Give the Picker master PR exactly one label: `release:patch`, `release:minor`
   or `release:major`. Require the version-label and build checks in branch
   protection, and test the PR against current master. Merge queues need
   `merge_group` checks before they can be used.
3. After merge, **Version policy** creates the next tag and explicitly dispatches
   **Tagged CI**. Direct master pushes without an associated merged PR do not
   create a tag. Wait for tagging to finish before merging the next release PR.
4. Tagged CI builds and validates the exact tag: CPython 3.12 Linux x86-64 wheel,
   plus x86-64/aarch64 AppImages and native archives, using one xcomm release.
5. After bundle upload, Tagged CI dispatches **Release Picker** with its run ID.
   The publisher waits up to 60 minutes for successful completion, verifies the
   bundle hashes and tag SHA, and publishes those exact artifacts without
   rebuilding. GitHub Releases are not automatically uploaded to PyPI.

Only tags on master's first-parent line that identify merged PR results are
accepted by the tagging/build policy. Configure a `v*` tag ruleset to restrict
manual creation or replacement. Do not publish the current Picker sdist as a
pip fallback: it omits vendored build dependencies; use a full source checkout
when a matching wheel is unavailable.

## Recovery

- Build failed: rerun **Tagged CI** with the existing tag and a compatible
  published xcomm version. Do not move the tag.
- Publication failed: rerun **Release Picker** with the successful Tagged CI
  run ID; it downloads the same checked bundle.
- Merge did not create a tag: inspect **Version policy**, then use its manual
  bump input if recovery is needed.

Generated DUTs may copy xcomm libraries. The package checks do not establish
that independently packaged DUT extensions load the same runtime instance;
verify ABI compatibility and relocation separately before publishing DUT wheels.
