# Use Ubuntu 22.04 as the base image
FROM ubuntu:22.04

# Set non-interactive installation
ENV DEBIAN_FRONTEND=noninteractive

# Set the locale
ENV LANG=C.UTF-8
ENV LANGUAGE=C.UTF-8
ENV LC_ALL=C.UTF-8
# Use https for apt repositories
RUN apt update && \
    apt install -y --no-install-recommends apt-transport-https ca-certificates && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*
RUN sed -i 's|http|https|g' /etc/apt/sources.list 

# Set the timezone to France
ENV TZ=Europe/Paris
RUN apt-get update && \
    apt-get install -y --no-install-recommends tzdata && \
    ln -fs /usr/share/zoneinfo/$TZ /etc/localtime && \
    dpkg-reconfigure --frontend noninteractive tzdata && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
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
    pkg-config \
    libfl-dev \
    bison \  
    flex \
    gperf \
    clang \
    g++ \
    zlib1g-dev \
    openssh-server \
    gnupg \
    autoconf \
    automake \
    libtool \
    openjdk-17-jdk \
    libpcre2-dev \
    help2man && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install SWIG (4.2.1)d
RUN git clone https://github.com/swig/swig.git -b v4.2.1 --depth=1 /tmp/swig && \
    cd /tmp/swig && \
    ./autogen.sh && \
    ./configure --prefix=/usr/local && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/swig

# Set up Kitware repository and install the latest CMake
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
    gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | \
    tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
    apt-get update && \
    apt-get install -y --no-install-recommends cmake && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install Verilator (5.0 series latest version, e.g. v5.018)
RUN git clone https://github.com/verilator/verilator -b v5.018 --depth=1 /tmp/verilator && \
    cd /tmp/verilator && \
    autoconf && \
    ./configure --prefix=/usr/local && \
    make -j$(nproc) && make test && \
    make install && \
    rm -rf /tmp/verilator

# Verify Dependency installations
RUN swig -version && \
    cmake --version && \
    verilator --version && \
    java --version && \
    python3 --version

# Install Picker
ENV BUILD_XSPCOMM_SWIG=python,java
RUN mkdir /workspace && \
    cd /workspace && \
    git clone https://github.com/XS-MLVP/picker.git --depth=1 && \
    wget https://github.com/chipsalliance/verible/releases/download/v0.0-3979-g786edf03/verible-v0.0-3979-g786edf03-linux-static-x86_64.tar.gz && \
    tar -xzf verible-v0.0-3979-g786edf03-linux-static-x86_64.tar.gz -C /usr/local/ --strip-components=1 && \
    rm verible-v0.0-3979-g786edf03-linux-static-x86_64.tar.gz && \
    cd picker && make && \
    make install && \
    make clean && \
    chmod 755 /usr/local/bin -R 

# set user and password
RUN useradd -m -s /bin/bash user && \
    echo "user:user" | chpasswd && \
    adduser user sudo && \
    echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && \
    chown user:user -R /workspace && \
    chmod 755 /workspace

# Switch to the new user
USER user
# Set the default shell to bash
SHELL ["/bin/bash", "-c"]
# Set working directory
WORKDIR /workspace
