#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

require_cmd rg

ROOT_DIR="/home/sfangyy/work/XSV/picker"
cd "${ROOT_DIR}"
PICKER_BIN="$(resolve_picker)"
OUT_DIR="${ROOT_DIR}/picker_out_vcs_coverage_dir"
DEFAULT_DIR="${ROOT_DIR}/picker_out_vcs_coverage_default"
REPORT_DIR="${OUT_DIR}/uc_test_report"
VDB_PATH="${REPORT_DIR}/Adder.vdb"

rm -rf "${OUT_DIR}" "${DEFAULT_DIR}"

"${PICKER_BIN}" export \
  "${ROOT_DIR}/example/Adder/Adder.v" \
  --autobuild false \
  --sdir "${ROOT_DIR}/template" \
  --sname Adder \
  --tdir "${OUT_DIR}" \
  --lang python \
  --sim vcs \
  --coverage \
  --coverage-dir "${REPORT_DIR}"

rg -Fq "export VCS_COVERAGE_DB := ${VDB_PATH}" "${OUT_DIR}/Makefile"
if rg -Fq 'cp -r build/${PROJECT}.vdb' "${OUT_DIR}/Makefile"; then
  red "[vcs-coverage-dir] custom VDB export still copies build VDB"
  exit 1
fi
rg -Fq "${VDB_PATH}" "${OUT_DIR}/dut_base.cpp"
rg -Fq 'vcs_coverage_db_path()' "${OUT_DIR}/dut_base.cpp"
rg -Fq 'if (kind == "toggle") return isTglMetric(metric);' "${OUT_DIR}/coverage/coverage.cpp"
rg -Fq 'if (kind == "branch") return isBranchMetric(metric);' "${OUT_DIR}/coverage/coverage.cpp"
rg -Fq 'if (opt.kinds.empty()) opt.kinds = {"line"};' "${OUT_DIR}/coverage/coverage.cpp"
rg -Fq 'covdb_loadmerge(covdbTest, test, name.c_str())' "${OUT_DIR}/coverage/coverage.cpp"
rg -Fq '\"schema_version\": 1' "${OUT_DIR}/coverage/coverage.cpp"
rg -Fq '\"simulator\": \"vcs\"' "${OUT_DIR}/coverage/coverage.cpp"
rg -Fq '\"metrics\"' "${OUT_DIR}/coverage/coverage.cpp"
rg -Fq '\"file\"' "${OUT_DIR}/coverage/coverage.cpp"
if rg -Fq 'source-map' "${OUT_DIR}/coverage/coverage.cpp"; then
  red "[vcs-coverage-dir] query executable still depends on a source map"
  exit 1
fi

"${PICKER_BIN}" export \
  "${ROOT_DIR}/example/Adder/Adder.v" \
  --autobuild false \
  --sdir "${ROOT_DIR}/template" \
  --sname Adder \
  --tdir "${DEFAULT_DIR}" \
  --lang python \
  --sim vcs \
  --coverage

rg -Fq 'cp -r build/${PROJECT}.vdb' "${DEFAULT_DIR}/Makefile"
rg -Fq 'std::filesystem::path(vcs_so_dir()) / "Adder.vdb"' "${DEFAULT_DIR}/dut_base.cpp"
rg -Fq 'item.kind = opt.kind;' "${DEFAULT_DIR}/coverage/coverage.cpp"
rg -Fq 'kinds = ["line"] if kind is None' "${DEFAULT_DIR}/python/dut.py"
rg -Fq 'def GetCoverage(self, kind=None, module=None, instance=None, test=None):' "${DEFAULT_DIR}/python/dut.py"
rg -Fq 'provider.database = path' "${DEFAULT_DIR}/python/dut.py"
rg -Fq 'provider.executable = executable' "${DEFAULT_DIR}/python/dut.py"
if rg -Fq 'def GetCoverageHelperPath' "${DEFAULT_DIR}/python/dut.py" ||
   rg -Fq 'def __getattr__' "${DEFAULT_DIR}/python/dut.py" ||
   rg -Fq 'def DumpCoverage' "${DEFAULT_DIR}/python/dut.py"; then
  red "[vcs-coverage-dir] generated Python API still exposes a redundant coverage alias"
  exit 1
fi
test ! -e "${DEFAULT_DIR}/coverage/source-map.tsv"

if "${PICKER_BIN}" export --sim verilator --coverage --coverage-dir "${REPORT_DIR}" >/dev/null 2>&1; then
  red "[vcs-coverage-dir] accepted --coverage-dir for Verilator"
  exit 1
fi

if "${PICKER_BIN}" export --sim vcs --coverage-dir "${REPORT_DIR}" >/dev/null 2>&1; then
  red "[vcs-coverage-dir] accepted --coverage-dir without --coverage"
  exit 1
fi

green "[vcs-coverage-dir] OK"
