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

# init.sh checks out the pinned xcomm revision (or XCOMM_RELEASE_VERSION);
# XCOMM_SKIP_ALIGN only disables aligning to a same-named branch.
XCOMM_SKIP_ALIGN=1 make init

cmake_args=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr)
if [[ -n "${PICKER_RELEASE_VERSION:-}" ]]; then
  [[ "${PICKER_RELEASE_VERSION}" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]
  cmake_args+=(
    "-DSKBUILD_PROJECT_VERSION=${PICKER_RELEASE_VERSION}"
    "-DSKBUILD_PROJECT_VERSION_FULL=${PICKER_RELEASE_VERSION}"
    "-DXSPCOMM_NATIVE=OFF"
  )
fi
cmake_args+=('-DCMAKE_INSTALL_RPATH=$ORIGIN/../lib')

cmake -S . -B build "${cmake_args[@]}"
cmake --build build --parallel "$(nproc)"
if [[ "${install_appdir}" == true ]]; then
  DESTDIR="${repo_root}/AppDir" cmake --install build
fi
