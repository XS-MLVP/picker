#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

VERIBLE_VERSION=${VERIBLE_VERSION:-v0.0-4051-g9fdb4057}
VERIBLE_ARCH=$(uname -m)
if [[ "${VERIBLE_ARCH}" != "x86_64" ]]; then
  VERIBLE_ARCH=${VERIBLE_ARCH/aarch64/arm64}
fi
VERIBLE_TGZ="verible-${VERIBLE_VERSION}-linux-static-${VERIBLE_ARCH}.tar.gz"
VERIBLE_URL="https://github.com/chipsalliance/verible/releases/download/${VERIBLE_VERSION}/${VERIBLE_TGZ}"

XCOMM_DIR="${ROOT_DIR}/dependence/xcomm"
XCOMM_REPO="https://github.com/XS-MLVP/xcomm.git"
XCOMM_REPO_FALLBACK="https://gitlink.org.cn/XS-MLVP/xcomm.git"

if ! command -v verible-verilog-syntax >/dev/null 2>&1; then
  echo "verible-verilog-syntax could not be found, please install verible first"
  echo "you can install verible by following command:"
  echo "\t$ wget \"${VERIBLE_URL}\""
  echo "\t$ tar -xzf ${VERIBLE_TGZ}"
  echo "\t$ mv verible-${VERIBLE_VERSION}/bin/verible-verilog-syntax /usr/local/bin/"
  echo "or you can install in user local directory, remember to add ~/.local/bin to your PATH"
  echo "\t$ mv verible-${VERIBLE_VERSION}/bin/verible-verilog-syntax ~/.local/bin/"
  exit 1
fi

if [[ ! -d "${XCOMM_DIR}/.git" ]]; then
  mkdir -p "$(dirname "${XCOMM_DIR}")"
  if git clone --depth=1 "${XCOMM_REPO}" "${XCOMM_DIR}"; then
    echo "[xcomm] cloned from github"
  else
    echo "[xcomm] github clone failed; falling back to gitlink.org.cn"
    rm -rf "${XCOMM_DIR}"
    git clone --depth=1 "${XCOMM_REPO_FALLBACK}" "${XCOMM_DIR}"
  fi
else
  if [[ -n "${XCOMM_SKIP_ALIGN:-}" ]]; then
    echo "[xcomm] XCOMM_SKIP_ALIGN is set, skipping automatic branch alignment."
  elif ! git -C "${XCOMM_DIR}" remote get-url origin >/dev/null 2>&1; then
    echo "[xcomm] No 'origin' remote found, skipping automatic branch alignment."
  elif ! git -C "${XCOMM_DIR}" rev-parse --abbrev-ref --symbolic-full-name @{u} >/dev/null 2>&1; then
    echo "[xcomm] No upstream configured for current branch, skipping automatic branch alignment."
  elif [[ -n "$(git -C "${XCOMM_DIR}" status --porcelain)" ]] || [[ -n "$(git -C "${XCOMM_DIR}" rev-list @{u}..HEAD 2>/dev/null || true)" ]]; then
    echo "[xcomm] Local changes or unpushed commits detected in dependence/xcomm, skipping automatic branch alignment."
  else
    if ! git -C "${XCOMM_DIR}" fetch --all --tags --prune; then
      echo "[xcomm] Fetch failed, skipping automatic branch alignment."
      exit 0
    fi
    PARENT_BRANCH=$(git -C "${ROOT_DIR}" branch --show-current)
    if [[ -z "${PARENT_BRANCH}" ]]; then
      echo "[xcomm] In detached HEAD, falling back to master branch for xcomm"
      PARENT_BRANCH="master"
    fi
    echo "[xcomm] Trying to align with parent branch '${PARENT_BRANCH}'"
    BRANCH="${PARENT_BRANCH}"
    if git -C "${XCOMM_DIR}" ls-remote --exit-code --heads origin "${BRANCH}" >/dev/null 2>&1; then
      echo "[xcomm] Branch '${BRANCH}' found in xcomm. Checking it out."
    else
      echo "[xcomm] Branch '${BRANCH}' not found in xcomm. Falling back to master."
      BRANCH="master"
    fi
    CURRENT_BRANCH=$(git -C "${XCOMM_DIR}" branch --show-current)
    if [[ "${CURRENT_BRANCH}" == "${BRANCH}" ]] && \
       [[ "$(git -C "${XCOMM_DIR}" rev-parse HEAD)" == "$(git -C "${XCOMM_DIR}" rev-parse "origin/${BRANCH}")" ]]; then
      echo "[xcomm] Already up to date on '${BRANCH}', skipping alignment."
      exit 0
    fi
    git -C "${XCOMM_DIR}" fetch origin "${BRANCH}:refs/remotes/origin/${BRANCH}"
    git -C "${XCOMM_DIR}" checkout -B "${BRANCH}" "origin/${BRANCH}"
  fi
fi
