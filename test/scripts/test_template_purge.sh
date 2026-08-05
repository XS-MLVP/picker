#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/picker-template-purge.XXXXXX")"
trap 'rm -rf "${TMP_DIR}"' EXIT

run_flat_purge() {
  local language="$1"
  local case_dir="${TMP_DIR}/${language}"
  local work_dir="${case_dir}/work"

  mkdir -p "${work_dir}"
  cp "${ROOT_DIR}/template/${language}/Makefile" "${work_dir}/Makefile"
  printf 'artifact\n' > "${work_dir}/final artifact.so"
  printf 'hidden\n' > "${work_dir}/.hidden-artifact"
  printf 'coverage\n' > "${work_dir}/coverage.vdb"

  if [[ "${language}" == "python" ]]; then
    printf 'stale\n' > "${case_dir}/__init__.py"
    printf 'package\n' > "${work_dir}/__init__.py"
  fi

  make -s -C "${work_dir}" purge

  test ! -e "${work_dir}"
  test -f "${case_dir}/final artifact.so"
  test -f "${case_dir}/.hidden-artifact"
  test ! -e "${case_dir}/coverage.vdb"
  if [[ "${language}" == "python" ]]; then
    grep -Fxq 'package' "${case_dir}/__init__.py"
  fi
}

run_selected_purge() {
  local language="$1"
  local output_name="$2"
  local case_dir="${TMP_DIR}/${language}"
  local work_dir="${case_dir}/work"

  mkdir -p "${work_dir}"
  cp "${ROOT_DIR}/template/${language}/Makefile" "${work_dir}/Makefile"
  printf 'discard\n' > "${work_dir}/discard.tmp"
  printf 'coverage\n' > "${work_dir}/coverage.vdb"

  if [[ "${language}" == "golang" ]]; then
    mkdir -p "${work_dir}/${output_name}"
    printf 'artifact\n' > "${work_dir}/${output_name}/final artifact"
  else
    printf 'artifact\n' > "${work_dir}/${output_name}"
  fi

  make -s -C "${work_dir}" purge

  test ! -e "${work_dir}"
  test -e "${case_dir}/${output_name}"
  test ! -e "${case_dir}/discard.tmp"
  test ! -e "${case_dir}/coverage.vdb"
}

run_failed_move() {
  local case_dir="${TMP_DIR}/failed move"
  local work_dir="${case_dir}/work"
  local log_file="${case_dir}/purge.log"

  mkdir -p "${work_dir}/blocked" "${case_dir}/blocked"
  cp "${ROOT_DIR}/template/cpp/Makefile" "${work_dir}/Makefile"
  printf 'source\n' > "${work_dir}/blocked/source-artifact"
  printf 'existing\n' > "${case_dir}/blocked/existing-artifact"

  if make -s -C "${work_dir}" purge > "${log_file}" 2>&1; then
    red "[template-purge] Purge unexpectedly succeeded after an artifact move failed"
    exit 1
  fi

  test -d "${work_dir}"
  test -f "${work_dir}/blocked/source-artifact"
  test -f "${case_dir}/blocked/existing-artifact"
  if grep -Fq 'purge complete' "${log_file}"; then
    red "[template-purge] Purge reported completion after an artifact move failed"
    exit 1
  fi
}

blue "[template-purge] Checking portable artifact moves"
run_flat_purge cpp
run_flat_purge python
run_flat_purge lua
run_selected_purge golang golang
run_selected_purge java "final artifact.jar"
run_selected_purge scala "final artifact.jar"
run_failed_move
green "[template-purge] OK"
