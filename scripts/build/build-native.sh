#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
cd "${repo_root}"

install_appdir=false
if [[ "${1:-}" == --install-appdir ]]; then
  install_appdir=true
elif [[ $# -ne 0 ]]; then
  echo "Usage: $0 [--install-appdir]" >&2
  exit 2
fi

if [[ -n "${XCOMM_RELEASE_VERSION:-}" ]]; then
  [[ "${XCOMM_RELEASE_VERSION}" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]
  xcomm_ref="refs/tags/v${XCOMM_RELEASE_VERSION}"
else
  xcomm_ref=${XCOMM_SOURCE_REF:-$(sed -n 's/^  xcomm:[[:space:]]*//p' .build-config.yml)}
  [[ "$xcomm_ref" =~ ^[0-9a-f]{40}$ ]]
fi
XCOMM_SKIP_ALIGN=1 make init
if [[ -n "$(git -C dependence/xcomm status --porcelain)" ]]; then
  echo 'Refusing to switch an xcomm checkout with local changes' >&2
  exit 1
fi
git -C dependence/xcomm fetch origin "$xcomm_ref"
git -C dependence/xcomm checkout --detach FETCH_HEAD

cmake_args=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr)
if [[ -n "${PICKER_RELEASE_VERSION:-}" ]]; then
  [[ "${PICKER_RELEASE_VERSION}" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]
  cmake_args+=(
    "-DSKBUILD_PROJECT_VERSION=${PICKER_RELEASE_VERSION}"
    "-DSKBUILD_PROJECT_VERSION_FULL=${PICKER_RELEASE_VERSION}"
  )
fi
cmake_args+=('-DCMAKE_INSTALL_RPATH=$ORIGIN/../lib')

cmake -S . -B build "${cmake_args[@]}"
cmake --build build --parallel "$(nproc)"
if [[ "${install_appdir}" == true ]]; then
  DESTDIR="${repo_root}/AppDir" cmake --install build
fi
