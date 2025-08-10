#!/usr/bin/env bash
set -euo pipefail

red()   { printf "\033[31m%s\033[0m\n" "$*"; }
green() { printf "\033[32m%s\033[0m\n" "$*"; }
blue()  { printf "\033[34m%s\033[0m\n" "$*"; }

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    red "Missing required command: $1"
    exit 1
  fi
}

# Resolve picker binary: prefer local build
resolve_picker() {
  local bin
  if [[ -x "$(git rev-parse --show-toplevel 2>/dev/null)/build/bin/picker" ]]; then
    bin="$(git rev-parse --show-toplevel)/build/bin/picker"
  elif command -v picker >/dev/null 2>&1; then
    bin="picker"
  else
    red "picker binary not found. Build with 'make' or install it."
    exit 1
  fi
  echo "$bin"
}

ROOT_DIR="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
