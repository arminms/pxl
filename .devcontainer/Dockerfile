# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Armin Sobhani
#

#-- pxl image ------------------------------------------------------------------

FROM ubuntu:22.04 AS pxl

# change default shell to bash
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# install build-essential and other dependencies
RUN set -ex \
    && apt-get update && apt-get upgrade -y \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        git \
        wget \
        pkg-config \
    && rm -rf /var/lib/apt/lists/*

# install CMake >= 3.19
RUN set -ex \
    && wget -qO- https://cmake.org/files/v3.25/cmake-3.25.2-linux-x86_64.tar.gz \
        | tar --strip-components=1 -xz -C /usr/local

# install PXL
RUN set -ex \
    && mkdir -p /opt/xeus-cling \
    && cd /opt \
    && git clone https://github.com/arminms/pxl.git \
    && cd pxl \
    && cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/xeus-cling \
        -DPXL_ENABLE_TESTS=OFF \
    && cmake --build build \
    && cmake --install build

#-- pxl-xeus-cling -------------------------------------------------------------

FROM asobhani/xeus-cling-jupyter:latest AS pxl-xeus-cling

LABEL maintainer="Armin Sobhani <arminms@gmail.com>"
LABEL description="Docker image for GitHub Codespaces development with PXL"

# change default shell to bash
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

USER root

# install necessary libraries for development
RUN set -ex \
    && apt-get update && apt-get upgrade -y \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        tar \
        git \
        zip \
        unzip \
        curl \
        wget \
        pkg-config \
        ninja-build \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# install CMake 3.31
RUN set -ex \
    && wget -qO- https://cmake.org/files/v3.31/cmake-3.31.0-linux-x86_64.tar.gz \
        | tar --strip-components=1 -xz -C /usr/local

# set the PATH
ENV VCPKG_INSTALLATION_ROOT="${HOME}/vcpkg"
ENV PATH="${HOME}/vcpkg:$PATH"

# copy pxl
COPY --from=pxl /opt/xeus-cling /opt/xeus-cling

# switch back to jovyan
USER ${NB_USER}

# install vcpkg as NB_USER
RUN set -ex \
    && git clone https://github.com/Microsoft/vcpkg.git ${HOME}/vcpkg \
    && cd ${HOME}/vcpkg \
    && ./bootstrap-vcpkg.sh

# copy tutorial markdowns to the home directory and convert them to notebooks
COPY --chown=${NB_UID}:${NB_GID} doc/sandbox.md ${HOME}/
RUN set -ex \
    && cd ${HOME} \
    && jupytext --to notebook *.md \
    && rm -rf *.md
