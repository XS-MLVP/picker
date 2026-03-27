#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

SLANG_DIR="${SLANG_DIR:-${ROOT_DIR}/dependence/slang}"
SLANG_REPO="${SLANG_REPO:-https://github.com/MikePopoloski/slang.git}"
SLANG_TAG="${SLANG_TAG:-v10.0}"

FMT_DIR="${FMT_DIR:-${ROOT_DIR}/dependence/fmt}"
FMT_REPO="${FMT_REPO:-https://github.com/fmtlib/fmt.git}"
FMT_TAG="${FMT_TAG:-12.1.0}"

XCOMM_DIR="${XCOMM_DIR:-${ROOT_DIR}/dependence/xcomm}"
XCOMM_REPO="${XCOMM_REPO:-https://github.com/XS-MLVP/xcomm.git}"
XCOMM_REPO_FALLBACK="${XCOMM_REPO_FALLBACK:-https://gitlink.org.cn/XS-MLVP/xcomm.git}"

if [[ ! -d "${FMT_DIR}/.git" ]]; then
  mkdir -p "${FMT_DIR}"
  git -C "${FMT_DIR}" init -q
  git -C "${FMT_DIR}" remote add origin "${FMT_REPO}"
  git -C "${FMT_DIR}" fetch --depth=1 origin "refs/tags/${FMT_TAG}:refs/tags/${FMT_TAG}"
  git -c advice.detachedHead=false -C "${FMT_DIR}" checkout --detach "${FMT_TAG}"
  echo "[fmt] fetched release tag ${FMT_TAG} from ${FMT_REPO}"
elif [[ -n "${FMT_SKIP_ALIGN:-}" ]]; then
  echo "[fmt] FMT_SKIP_ALIGN is set, skipping automatic tag alignment."
elif [[ -n "$(git -C "${FMT_DIR}" status --porcelain)" ]]; then
  echo "[fmt] Local changes detected in ${FMT_DIR}, skipping automatic tag alignment."
elif ! git -C "${FMT_DIR}" fetch --depth=1 origin "refs/tags/${FMT_TAG}:refs/tags/${FMT_TAG}"; then
  echo "[fmt] Fetch failed, skipping automatic tag alignment."
else
  echo "[fmt] Checking out release tag '${FMT_TAG}'"
  git -c advice.detachedHead=false -C "${FMT_DIR}" checkout --detach "${FMT_TAG}"
fi

if [[ ! -d "${SLANG_DIR}/.git" ]]; then
  mkdir -p "${SLANG_DIR}"
  git -C "${SLANG_DIR}" init -q
  git -C "${SLANG_DIR}" remote add origin "${SLANG_REPO}"
  git -C "${SLANG_DIR}" fetch --depth=1 origin "refs/tags/${SLANG_TAG}:refs/tags/${SLANG_TAG}"
  git -c advice.detachedHead=false -C "${SLANG_DIR}" checkout --detach "${SLANG_TAG}"
  echo "[slang] fetched release tag ${SLANG_TAG} from ${SLANG_REPO}"
elif [[ -n "${SLANG_SKIP_ALIGN:-}" ]]; then
  echo "[slang] SLANG_SKIP_ALIGN is set, skipping automatic tag alignment."
elif [[ -n "$(git -C "${SLANG_DIR}" status --porcelain)" ]]; then
  echo "[slang] Local changes detected in ${SLANG_DIR}, skipping automatic tag alignment."
elif ! git -C "${SLANG_DIR}" fetch --depth=1 origin "refs/tags/${SLANG_TAG}:refs/tags/${SLANG_TAG}"; then
  echo "[slang] Fetch failed, skipping automatic tag alignment."
else
  echo "[slang] Checking out release tag '${SLANG_TAG}'"
  git -c advice.detachedHead=false -C "${SLANG_DIR}" checkout --detach "${SLANG_TAG}"
fi

if [[ ! -d "${XCOMM_DIR}/.git" ]]; then
  mkdir -p "$(dirname "${XCOMM_DIR}")"
  if git clone --depth=1 "${XCOMM_REPO}" "${XCOMM_DIR}"; then
    echo "[xcomm] cloned from github"
  else
    echo "[xcomm] github clone failed; falling back to ${XCOMM_REPO_FALLBACK}"
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
