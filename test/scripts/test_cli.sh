#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

PICKER_BIN="$(resolve_picker)"

blue "[CLI] Checking picker --help and subcommands"
"${PICKER_BIN}" -h >/dev/null
"${PICKER_BIN}" --version >/dev/null || true
"${PICKER_BIN}" export -h >/dev/null
"${PICKER_BIN}" pack -h >/dev/null

blue "[CLI] Checking --check summary"
"${PICKER_BIN}" --check || true

green "[CLI] OK"
