# Maintaining Picker releases

Picker and xspcomm have independent version tags. Release xspcomm first;
Picker's wheel and native packages use a compatible xspcomm release. This page
records the maintainer procedure.

## Versions and releases

Version tags are independent in the two Git repositories. Use `vX.Y.Z` tags,
for example `v2.0.0` for Picker and `v0.1.0` for xcomm. The wheel version is
derived from the tag by setuptools-scm. Commits after a tag yield development
versions such as `2.0.1.dev3`; ordinary CI uploads these wheels as artifacts
and does not create a stable release. Older date/SHA AppImage tags do not
participate in Python package version calculation; CI no longer creates them.

Every PR targeting `master` must have exactly one of `release:patch`,
`release:minor`, or `release:major`. Other labels are allowed. The **Version
policy / release-label** check verifies this. In GitHub Settings → Rules →
Rulesets (or the `master` branch protection rule), add this check to the
required status checks; the workflow alone does not prevent merging. Create
the three labels in each repository. Also require the relevant CI build checks
and require an up-to-date PR before merging; otherwise tagging after merge does
not imply the merged source has passed CI. Do not use merge queue until the
required checks also support `merge_group`.

After a labeled PR merges, the `master` push starts **Version policy**. It
finds the merged PR, computes the next version, pushes its tag and dispatches
**Tagged CI** with the tag and commit SHA. A direct push without an associated
merged PR does not create a tag. Every merged PR, including a
documentation-only PR, creates a version tag. The tag push itself does not
start CI when it was made using `GITHUB_TOKEN`; the explicit dispatch does.
Both version calculation and Tagged CI apply the same master-tag rule: the tag
must point to a commit on master's first-parent line that GitHub records as the
result of a merged PR targeting master. A higher-numbered tag on a feature
branch is ignored when calculating the next version, and manually requesting a
build for a tag outside this rule fails before any assets are built. Tag names
remain repository-wide, so a conflicting name elsewhere still blocks creation
of the next tag rather than being overwritten. Git itself does not record the
branch on which a tag was created. To prevent users from creating, moving or
deleting `v*` tags outside this workflow, protect that tag pattern with a
GitHub tag ruleset; the workflow checks the target commit, not the tagger's
historical branch.
The version-tag job serializes its runs, but GitHub does not guarantee that
rapid consecutive merges enter that queue in commit order. Until version
allocation is made order-aware, avoid merging another PR while a previous
Version policy run is still tagging; a later commit tagged first can cause the
older run to fail its ancestry check.

**Tagged CI** checks out that exact tag, tests the native builds and builds a
CPython 3.12 Linux x86-64 wheel, Linux x86-64 and aarch64 AppImages, and Linux
x86-64 and aarch64 native archives. It uploads one checked release bundle.
After the bundle upload succeeds, Tagged CI explicitly dispatches **Release
Picker** with its run ID. A `workflow_run` listener is not used because Tagged
CI is itself dispatched using `GITHUB_TOKEN`. Release Picker waits up to 60
minutes for the source run to complete successfully; a failed, cancelled or
unfinished run cannot publish. Tagged CI does not wait for Release Picker,
which would prevent the source run from completing. Release Picker downloads
that run's bundle, checks its hashes and tag SHA, and publishes those exact
files without rebuilding. All five packages, `SHA256SUMS` and
`RELEASE-MANIFEST.json` go into one GitHub Release. The manifest records the
Picker tag, commit, CI run ID and
xcomm version used. The archives contain the installed
`usr/bin/picker`, `usr/share/picker` templates and native libraries; they are
not bare executables. AppImage is the self-contained desktop distribution;
the archive is an installed tree for a compatible Linux system. The wheel is
for pip and depends on a separately published xspcomm wheel. Tagged CI selects
one compatible xspcomm version for all three package types.

**Release Picker** never changes the tag. If the tagged build fails, rerun
**Tagged CI** manually with the existing tag and a compatible xcomm version.
If publication fails, rerun **Release Picker** with the successful Tagged CI
run ID; it publishes the same artifact. **Version policy** has a manual bump
input for recovering from a missed merge event. GitHub release wheels are not
automatically uploaded to PyPI; publishing there needs separate credentials
and platform-wheel policy. Development CI still builds AppImages and wheels,
but it no longer creates a date/SHA AppImage Release. The old `master`-push
Docker image publish is not part of this pipeline; Docker PR changes still
receive a build check.

Development CI and Tagged CI use the same scripts under `scripts/build/` for
native toolchain setup, CMake build and install, AppImage packaging, and wheel
installation checks. A master PR runs full Development CI and the version-label
check. A non-master branch push also runs Development CI unless that branch
already has an open master PR; in that case the PR run owns the build. A merge
to master runs Version policy rather than a second development build. Tagged
CI uses the final tag and a published xcomm version; its checked artifacts are
the only inputs to Release Picker. PR and branch builds pin xcomm to the commit
in `.build-config.yml` (an explicit `XCOMM_SOURCE_REF` commit can override it)
and never become release assets. Tagged builds use the selected published xcomm
tag instead. The build refuses to switch an xcomm checkout with local changes.

The `xspcomm>=0.3.0.dev0,<0.4` requirement in `pyproject.toml` selects the xcomm
0.3 series with native ABI 3 and coverage descriptor protocol 4. Development CI
pins the tested coverage-v2 commit in `.build-config.yml` and builds an explicit
`0.3.0.dev0` xcomm preview until the ABI 3 release is published. This avoids
labeling an ABI 3 wheel as a development patch of the ABI 2 release. Wheel
verification checks both versions, and native release archives must contain
`libxspcomm.so.3`. Tagged CI requires a compatible published xcomm 0.3 release;
it cannot use the development preview.
Change the range only after testing the corresponding ABI and baseline APIs.
Release metadata, the installed Picker command and CMake receive their version
from the same tag.

## Building and installing locally

Install a compiler, CMake, SWIG >= 4.2, Python development headers and the
Picker source dependencies. Run `make init` explicitly before building Picker
if those dependencies are missing. The wheel target itself does not fetch or
switch source repositories and does not remove existing packages.

```sh
python3 -m venv /tmp/picker-package-build
/tmp/picker-package-build/bin/python -m pip install build
SETUPTOOLS_SCM_PRETEND_VERSION_FOR_XSPCOMM=0.3.0.dev0 \
    make wheel PYTHON=/tmp/picker-package-build/bin/python

python3 -m venv /tmp/picker-package-test
/tmp/picker-package-test/bin/python -m pip install --no-index --find-links=dist picker
/tmp/picker-package-test/bin/python -m pip check
/tmp/picker-package-test/bin/picker --version
```

Before xcomm 0.3 is tagged, set
`SETUPTOOLS_SCM_PRETEND_VERSION_FOR_XSPCOMM=0.3.0.dev0` when running `make wheel`
from this development branch. This affects only the xcomm preview; Picker
continues to derive its version from its own Git history. Formal releases use
the published xcomm wheel and its release version.

Use a fresh checkout or empty `dist` directory for release builds; pip selects
the highest matching version when several old wheels are present. `make
wheel_install` intentionally only prints an installation command and never
uninstalls packages from the current interpreter.

Native xcomm builds and generated simulator libraries use
`-fno-strict-aliasing` to keep optimization settings consistent across signal
bindings and their consumers. Picker applies it to the pinned xcomm target,
and `make wheel` passes it when building xcomm separately. Generated simulator
libraries use the same option, including the C++ flags passed to
Verilator/VCS/UVS. Native memory bindings also use typed, fixed-size `memcpy`
accesses to respect object boundaries and alignment.
Keep `-O3` and the complete signal-value checks enabled. This option changes
type-based alias analysis; it does not fix out-of-bounds or unaligned accesses,
invalid shifts, object lifetime errors, or thread synchronization. Use
ASan/UBSan and explicit boundary tests when investigating those issues.

Picker contains a compiled executable and xspcomm contains a SWIG/CPython
extension plus a C++ shared library. A matching wheel avoids compilation on
the user's machine. Picker's current sdist omits its vendored slang/fmt/yaml-cpp
sources, so **do not publish it as a pip fallback**. If no Picker wheel matches,
build from a full source checkout after `make init`; this does require local
compilation and the build toolchain. Use `--only-binary=:all:` when an
installation must fail instead of attempting a source build.
The wheel release currently targets CPython 3.12 on Linux x86-64. Native
archives and AppImages target Linux x86-64 and aarch64; expand these matrices
before claiming other platforms are supported.

## Native compatibility

xcomm's package version and C++ ABI version are different. The latter is
`XSPCOMM_ABI_VERSION` in xcomm's `CMakeLists.txt`; it controls the
`libxspcomm.so.N` SONAME and is exposed as `xspcomm.abi_version()`. Bump it
when a native ABI change is incompatible, then rebuild xcomm, Picker and any
generated DUT extensions. A package patch release alone must not change the
SONAME. A Python wheel is also tied to its CPython ABI and platform tag;
the SWIG module is not declared `abi3`.

The xcomm 0.2 trigger engine changed the native layouts of `XClock`, `XData`,
and `ExprNode` and required ABI 2. The coverage-v2 descriptors expand
`XCoverageBin`, requiring ABI 3 for the xcomm 0.3 series. Rebuild native clients
and generated DUT extensions against the matching headers and runtime;
changing a library symlink cannot make incompatible binaries compatible. Keep
each DUT extension and its xcomm runtime on the same native ABI. The coverage
descriptor protocol version is separate from this ABI number.

The TLM template uses the source SDK installed under `share/xspcomm/tlm` for
its SWIG interface and bridge source. Set `XSP_COMM_TLM` to override that SDK
location; the public C++ headers remain under `include/xspcomm/tlm`.

Generated Python and UVM DUT signals are direct `XData` objects. Use
`dut.signal.value` for signal values and pass `dut.signal` to backend APIs;
the former `XPin` wrapper and `dut.signal.xdata` access are no longer used.

The wheel CI installs xcomm and Picker from built wheels in a clean virtual
environment, checks metadata against native versions, exercises signal and clock APIs,
and runs the installed Picker command. These checks do **not** yet prove that
an independently packaged DUT extension uses the same `libxspcomm` instance:
generated Python DUTs can still copy xcomm. Keep DUT wheel/linker relocation
tests as a separate release gate before publishing a general-purpose DUT wheel.
