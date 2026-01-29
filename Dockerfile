FROM ubuntu:22.04

# Non-interactive installs and locale
ENV DEBIAN_FRONTEND=noninteractive \
    LANG=C.UTF-8 \
    LANGUAGE=C.UTF-8 \
    LC_ALL=C.UTF-8 \
    TZ=Europe/Paris

# Use bash for RUN so pipefail works reliably.
SHELL ["/bin/bash", "-c"]

# Base deps in one layer; set timezone cleanly
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    tzdata \
    build-essential \
    git \
    sudo \
    wget \
    curl \
    vim \
    software-properties-common \
    python3 \
    python3-pip \
    python3-dev \
    libpcre3-dev \
    libpcre2-dev \
    pkg-config \
    libfl-dev \
    bison \
    flex \
    gperf \
    clang \
    g++ \
    zlib1g-dev \
    gnupg \
    autoconf \
    automake \
    libtool \
    openjdk-17-jdk \
    help2man && \
    ln -fs /usr/share/zoneinfo/$TZ /etc/localtime && \
    dpkg-reconfigure -f noninteractive tzdata && \
    rm -rf /var/lib/apt/lists/*

# SWIG
ARG SWIG_VERSION=v4.2.1
RUN git clone https://github.com/swig/swig.git -b ${SWIG_VERSION} --depth=1 /tmp/swig && \
    cd /tmp/swig && ./autogen.sh && \
    ./configure --prefix=/usr/local && \
    make -j"$(nproc)" && make install && \
    rm -rf /tmp/swig

# Latest CMake from Kitware APT
RUN set -euo pipefail; \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
    gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' > /etc/apt/sources.list.d/kitware.list && \
    apt-get update && apt-get install -y --no-install-recommends cmake && \
    rm -rf /var/lib/apt/lists/*

# Verilator (pinned)
ARG VERILATOR_VERSION=v5.018
RUN git clone https://github.com/verilator/verilator -b ${VERILATOR_VERSION} --depth=1 /tmp/verilator && \
    cd /tmp/verilator && autoconf && \
    ./configure --prefix=/usr/local && \
    make -j"$(nproc)" && \
    make install && \
    rm -rf /tmp/verilator

# Verify toolchain
RUN swig -version && cmake --version && verilator --version && java --version && python3 --version

# Install Verible (pinned)
ARG VERIBLE_VERSION=v0.0-3979-g786edf03
RUN set -euo pipefail; \
    ARCH="$(uname -m)"; \
    case "$ARCH" in aarch64) V_ARCH=aarch64 ;; x86_64) V_ARCH=x86_64 ;; *) V_ARCH=x86_64 ;; esac; \
    cd /tmp && \
    wget -q https://github.com/chipsalliance/verible/releases/download/${VERIBLE_VERSION}/verible-${VERIBLE_VERSION}-linux-static-${V_ARCH}.tar.gz && \
    tar -xzf verible-${VERIBLE_VERSION}-linux-static-${V_ARCH}.tar.gz -C /usr/local/ --strip-components=1 && \
    rm -f verible-${VERIBLE_VERSION}-linux-static-${V_ARCH}.tar.gz

# Build and install Picker from local source (copied into image)
ENV BUILD_XSPCOMM_SWIG=python,java
WORKDIR /workspace
COPY . /workspace/picker
RUN cd /workspace/picker && make && make install && make clean

# Create a non-root user for interactive use
RUN useradd -m -s /bin/bash user && \
    adduser user sudo && \
    echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && \
    chown -R user:user /workspace

USER user
SHELL ["/bin/bash", "-c"]
WORKDIR /workspace
