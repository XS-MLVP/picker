#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

require_cmd cmake
require_cmd c++
require_cmd grep
require_cmd ln
require_cmd make
require_cmd mktemp
require_cmd python3
require_cmd verilator

PICKER_BIN="$(resolve_picker)"
ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"
TEST_TMP="$(mktemp -d "${TMPDIR:-/tmp}/picker-autobuild-failure.XXXXXX")"
trap 'rm -rf -- "${TEST_TMP}"' EXIT
LOG_DIR="${TEST_TMP}/logs"
mkdir -p "${LOG_DIR}"

cp -R "${ROOT_DIR}/template" "${TEST_TMP}/template"
cp "${ROOT_DIR}/test/data/failing_example.py" "${TEST_TMP}/template/python/example.py"
cp "${ROOT_DIR}/test/data/failing_example.cpp" "${TEST_TMP}/template/cpp/example.cpp"

failures=0

SHIM_BIN="${TEST_TMP}/shim-bin"
mkdir -p "${SHIM_BIN}"
for command in python3 lua go java scala sleep; do
  ln -s "${ROOT_DIR}/test/data/example_status_shim.sh" "${SHIM_BIN}/${command}"
done

run_template_case() {
  local language="$1"
  local expected_status="$2"
  local case_dir="${TEST_TMP}/template-${language}-${expected_status}/binding"
  local log_file="${LOG_DIR}/template-${language}-${expected_status}.log"
  local make_status

  mkdir -p "${case_dir}"
  cp "${ROOT_DIR}/template/${language}/Makefile" "${case_dir}/Makefile"
  if [[ ${language} == cpp ]]; then
    mkdir -p "${case_dir}/build"
    cp "${ROOT_DIR}/test/data/example_status_shim.sh" \
      "${case_dir}/build/UT{{__TOP_MODULE_NAME__}}_example"
    chmod +x "${case_dir}/build/UT{{__TOP_MODULE_NAME__}}_example"
  fi

  set +e
  PICKER_EXAMPLE_STATUS="${expected_status}" PATH="${SHIM_BIN}:${PATH}" \
    make -s -C "${case_dir}" -f Makefile -o compile MAKE="make -o compile" all >"${log_file}" 2>&1
  make_status=$?
  set -e

  if ! grep -q "PICKER_TEMPLATE_EXAMPLE_SHIM" "${log_file}"; then
    red "[template-status] ${language} example command was not exercised"
    return 1
  fi
  if [[ ${expected_status} -eq 0 && ${make_status} -ne 0 ]]; then
    red "[template-status] ${language} rejected a successful example"
    return 1
  fi
  if [[ ${expected_status} -ne 0 && ${make_status} -eq 0 ]]; then
    red "[template-status] ${language} swallowed example status ${expected_status}"
    return 1
  fi

  green "[template-status] ${language} example status ${expected_status} handled correctly"
}

for language in python lua cpp golang java scala; do
  run_template_case "${language}" 0 || failures=$((failures + 1))
  run_template_case "${language}" 23 || failures=$((failures + 1))
done

run_failure_case() {
  local language="$1"
  local marker="$2"
  local output_dir="${TEST_TMP}/output-${language}"
  local log_file="${LOG_DIR}/${language}.log"
  local picker_status

  blue "[autobuild-failure] Verifying ${language} example failures reach Picker"
  set +e
  "${PICKER_BIN}" export \
    "${ROOT_DIR}/example/Adder/Adder.v" \
    --autobuild true \
    --build-threads 2 \
    --example \
    --sdir "${TEST_TMP}/template" \
    --sname Adder \
    --tdir "${output_dir}" \
    --lang "${language}" \
    --sim verilator >"${log_file}" 2>&1
  picker_status=$?
  set -e

  if [[ ${picker_status} -eq 0 ]]; then
    red "[autobuild-failure] Picker returned success for the failing ${language} example"
    return 1
  fi
  if ! grep -q "${marker}" "${log_file}"; then
    red "[autobuild-failure] The intentional ${language} failure was not exercised"
    return 1
  fi
  if ! grep -q "Build failed" "${log_file}"; then
    red "[autobuild-failure] Picker did not report the generated build failure"
    return 1
  fi

  green "[autobuild-failure] ${language} failed closed (Picker status ${picker_status})"
}

run_failure_case python PICKER_INTENTIONAL_PYTHON_EXAMPLE_FAILURE || failures=$((failures + 1))
run_failure_case cpp PICKER_INTENTIONAL_CPP_EXAMPLE_FAILURE || failures=$((failures + 1))

if [[ ${failures} -ne 0 ]]; then
  exit 1
fi

green "[autobuild-failure] OK"
