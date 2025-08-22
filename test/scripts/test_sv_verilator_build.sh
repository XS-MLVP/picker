#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

require_cmd verible-verilog-syntax
require_cmd cmake

ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"

blue "[sv-build] Verilate and build .sv example (XDummyCache)"
pushd "${ROOT_DIR}" >/dev/null
bash "${ROOT_DIR}/example/XDummyCache/release-verilator.sh"
popd >/dev/null

green "[sv-build] OK"

