#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
verilator_version=${VERILATOR_VERSION:-$(sed -n 's/^  verilator:[[:space:]]*//p' "${repo_root}/.build-config.yml")}
[[ -n "${verilator_version}" ]]
verilator_prefix=${VERILATOR_PREFIX:-${HOME}/.cache/verilator/${verilator_version}}

if [[ ! -x "${verilator_prefix}/bin/verilator" ]]; then
  verilator_source=$(mktemp -d /tmp/picker-verilator.XXXXXX)
  trap 'rm -rf -- "${verilator_source}"' EXIT
  git clone https://github.com/verilator/verilator -b "${verilator_version}" --depth=1 "${verilator_source}"
  cd "${verilator_source}"
  autoconf
  ./configure --prefix="${verilator_prefix}"
  make -j"$(nproc)"
  make install
fi

"${verilator_prefix}/bin/verilator" --version
if [[ -n "${GITHUB_PATH:-}" ]]; then
  echo "${verilator_prefix}/bin" >> "${GITHUB_PATH}"
fi
