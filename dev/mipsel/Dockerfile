FROM debian:trixie

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        bash \
        ca-certificates \
        git \
        make \
        cmake \
        ninja-build \
        python3 \
        python3-pip \
        gcc-mipsel-linux-gnu \
        binutils-mipsel-linux-gnu \
        libc6-dev-mipsel-cross \
        file \
        less \
        vim-tiny \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

RUN git config --system --add safe.directory /workspace

CMD ["/bin/bash"]
