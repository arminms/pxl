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
LABEL description="Docker image for PXL with Xeus-Cling"

# change default shell to bash
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

USER root

# copy pxl
COPY --from=pxl /opt/xeus-cling /opt/xeus-cling

# switch back to jovyan
USER ${NB_USER}

# copy tutorial markdowns to the home directory and convert them to notebooks
COPY --chown=${NB_UID}:${NB_GID} doc/sandbox.md ${HOME}/
RUN set -ex \
    && cd ${HOME} \
    && jupytext --to notebook *.md \
    && rm -rf *.md
