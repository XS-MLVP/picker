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
    wget \
    curl \
    vim \
    time \
    software-properties-common \
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

ENV NVM_DIR=/usr/local/nvm
ENV NODE_VERSION=20.12.2

RUN mkdir -p $NVM_DIR && \
    curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash && \
    . $NVM_DIR/nvm.sh && \
    nvm install $NODE_VERSION && \
    nvm alias default $NODE_VERSION && \
    nvm use default && \
    npm install -g npm@10 && \
    chmod -R 777 $NVM_DIR && \
    echo 'export NVM_DIR="/usr/local/nvm"' >> /etc/bash.bashrc && \
    echo '[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"' >> /etc/bash.bashrc

ENV PATH=$NVM_DIR/versions/node/v$NODE_VERSION/bin:$PATH

RUN add-apt-repository ppa:deadsnakes/ppa -y && \
    apt-get update && apt-get install -y --no-install-recommends python3.11 python3.11-dev python3.11-venv && \
    python3.11 -m ensurepip --upgrade && \
    python3.11 -m pip install --upgrade pip && \
    ln -sf /usr/bin/python3.11 /usr/local/bin/python3 && \
    ln -sf /usr/local/bin/pip3.11 /usr/local/bin/pip3 && \
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

# Build and install Picker from local source (copied into image)
ENV BUILD_XSPCOMM_SWIG=python,java
WORKDIR /workspace
COPY . /workspace/picker
RUN cd /workspace/picker && make && make install && make clean



SHELL ["/bin/bash", "-c"]
WORKDIR /workspace
