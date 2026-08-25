#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

require_cmd python3

CXX_BIN="${CXX:-c++}"
require_cmd "${CXX_BIN}"

ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"
HELPER_SOURCE="${ROOT_DIR}/template/coverage/verilator/verilator_coverage.cpp"
DATABASE="${ROOT_DIR}/test/data/verilator_line_coverage.dat"

temp_dir="$(mktemp -d)"
report="${temp_dir}/report.json"
helper="${temp_dir}/coverage"
trap 'rm -rf "${temp_dir}"' EXIT

"${CXX_BIN}" -std=c++20 -O2 "${HELPER_SOURCE}" -o "${helper}"
"${helper}" --database "${DATABASE}" --kind line > "${report}"

python3 - "${report}" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as source:
    report = json.load(source)

metric = report["metrics"]["line"]
assert report["schema_version"] == 1
assert report["simulator"] == "verilator"
assert report["query"] == {
    "kinds": ["line"], "module": None, "instance": None, "tests": []
}
assert metric["covered"] == 2
assert metric["total"] == 6
assert metric["uncovered"] == 4
assert abs(metric["rate"] - 100.0 / 3.0) < 1e-5
assert [(item["file"], item["line"], item["count"]) for item in report["items"]] == [
    ("build/{{__TOP_MODULE_NAME__}}.v", 99, 0),
    ("rtl/alpha.v", 20, 0),
    ("rtl/alpha.v", 30, 0),
    ("rtl/alpha.v", 31, 0),
]
PY

"${helper}" --database "${DATABASE}" --kind line --module Alpha > "${report}"

python3 - "${report}" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as source:
    metric = json.load(source)["metrics"]["line"]

assert metric["covered"] == 1
assert metric["total"] == 4
assert metric["uncovered"] == 3
PY

if "${helper}" --database "${DATABASE}" --kind branch >/dev/null 2>&1; then
    red "[verilator-coverage] helper accepted an unsupported coverage kind"
    exit 1
fi

green "[verilator-coverage] OK"
