# Release maintenance

Picker versions come from `vX.Y.Z` tags. A PR's `release:patch`, `release:minor`, or `release:major` label selects the version increment after merge.

## Automated release flow

1. Merge the PR into `master` after version checks and relevant builds pass.
2. Version policy creates a version tag and starts Tagged CI.
3. Tagged CI builds, installs, and validates wheels and native artifacts for the tag.
4. Release Picker waits up to 60 minutes for the source build, checks the commit SHA and artifact hashes, then publishes artifacts to GitHub Releases.

Release tags identify merged results on the first-parent history of `master`. Wait for tagging to finish before merging the next release PR. Configure branch protection to require version checks and relevant builds.

## Maintain dependency configuration

| Configuration | Purpose |
| --- | --- |
| `pyproject.toml` | Python dependency ranges |
| `.build-config.yml` | Source commits and tools used by CI builds |
| `.github/workflows/ci.yml` | Development builds and preview wheel configuration |
| Tagged CI | Select published dependencies satisfying release constraints |

After changing dependencies, rebuild and verify wheels and generated DUTs. If Picker needs a new dependency version, release that dependency first.

## Recover failures

| Failure | Action |
| --- | --- |
| Tagged CI build fails | Fix the build environment and rerun Tagged CI for the existing tag |
| Publication fails | Rerun Release Picker using the successful source run ID |
| A merge event fails to create a tag | Inspect Version policy logs and use its manual bump action |

Reuse existing tags during recovery to preserve the mapping between commits and artifacts.

## Local checks

```sh
python3 -m unittest discover -s test/release -v
bash scripts/build/verify-wheel.sh
```

Wheel checks need build outputs in `dist/`. Use `PICKER_WHEEL_DIR` and `XCOMM_WHEEL_DIR` to select other directories. The script installs packages and checks interfaces in a separate environment.

Release artifacts are published to GitHub Releases. The verified Python wheel is then published to PyPI as `uc-picker`. Documentation CI provides site and Wiki previews as build artifacts; see [documentation maintenance](documentation.md).

## Python distribution and PyPI setup

The distribution is `uc-picker`; the executable remains `picker`. It requires
`uc-xcomm>=1.1,<2` and SWIG 4.5.1. Publish `uc-xcomm` first: Tagged CI selects a
compatible stable release available on both GitHub and PyPI. Development CI pins
the corresponding xcomm source and uses a test-only version matching this range.

The release wheel is built in the pinned manylinux 2.28 x86-64 image, with generic
CPU flags and static C++ runtime linking, then audited before upload. It uses
`py3-none-manylinux_2_28_x86_64`: Picker does not bind to a Python minor version,
but its `uc-xcomm` dependency does. A wheel always omits bundled xcomm bindings;
raw CMake builds can still build them. Tagged native releases also disable
`XSPCOMM_NATIVE` for the bundled runtime. AppImages and native archives retain their
existing architectures and filenames. Development wheels retain their host tags.

Create a pending GitHub publisher in [PyPI Publishing](https://pypi.org/manage/account/publishing/):

| Field | Value |
| --- | --- |
| Project name | `uc-picker` |
| Owner | `XS-MLVP` |
| Repository | `picker` |
| Workflow filename | `release.yml` |
| Environment | `pypi` |

Create the matching GitHub environment, allowing releases from `master`. No PyPI
API token is needed. If the initial PyPI job fails before setup is complete,
configure the publisher and rerun that job with the same verified CI bundle.
