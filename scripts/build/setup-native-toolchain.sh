#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
swig_version=${SWIG_VERSION:-$(sed -n 's/^  swig:[[:space:]]*//p' "${repo_root}/.build-config.yml")}
[[ -n "${swig_version}" ]]

sudo apt-get update
sudo apt-get install -y --no-install-recommends \
  ca-certificates tzdata gnupg build-essential git wget curl \
  python3 python3-pip python3-venv python3-dev libpython3-dev \
  pkg-config libpcre3-dev libpcre2-dev libfl-dev \
  bison flex gperf clang g++ ninja-build zlib1g-dev liblz4-dev \
  autoconf automake libtool help2man openjdk-17-jdk

wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
  gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | \
  sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
sudo apt-get update
sudo apt-get install -y --no-install-recommends cmake

swig_source=$(mktemp -d /tmp/picker-swig.XXXXXX)
trap 'rm -rf -- "${swig_source}"' EXIT
git clone https://github.com/swig/swig.git -b "${swig_version}" --depth=1 "${swig_source}"
cd "${swig_source}"
./autogen.sh
./configure --prefix=/usr/local
make -j"$(nproc)"
sudo make install
