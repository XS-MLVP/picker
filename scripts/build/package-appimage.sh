#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
cd "${repo_root}"

arch=$(uname -m)
if [[ "${arch}" == arm64 ]]; then
  arch=aarch64
fi
[[ "${arch}" == x86_64 || "${arch}" == aarch64 ]]
test -x AppDir/usr/bin/picker

tool_dir=$(mktemp -d /tmp/picker-appimage-tools.XXXXXX)
trap 'rm -rf -- "${tool_dir}"' EXIT
wget -q -O "${tool_dir}/appimagetool" \
  "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-${arch}.AppImage"
wget -q -O "${tool_dir}/linuxdeploy" \
  "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${arch}.AppImage"
chmod +x "${tool_dir}/appimagetool" "${tool_dir}/linuxdeploy"
PATH="${tool_dir}:${PATH}" "${tool_dir}/linuxdeploy" --appdir AppDir/ --output appimage \
  --desktop-file src/appimage/picker.desktop \
  --icon-file src/appimage/logo256.png

if [[ -n "${APPIMAGE_OUTPUT_FILE:-}" ]]; then
  shopt -s nullglob
  images=( ./*.AppImage )
  [[ ${#images[@]} -eq 1 ]]
  mkdir -p "$(dirname "${APPIMAGE_OUTPUT_FILE}")"
  mv "${images[0]}" "${APPIMAGE_OUTPUT_FILE}"
fi
