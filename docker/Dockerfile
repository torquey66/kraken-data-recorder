FROM ubuntu:latest as build

LABEL org.opencontainers.image.source=https://github.com/torquey66/kraken-data-recorder
LABEL org.opencontainers.image.description="Kraken Data Recorder"
LABEL org.opencontainers.image.licenses="Creative Commons Attribution-NoDerivatives 4.0 International"

RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    clang-format \
    clang-tidy \
    emacs-nox \
    procps \
    python3 \
    python3-pip \
    python3-venv \
    git \
    cmake \
    unzip \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m venv /venv
ENV PATH="/venv/bin:$PATH"

RUN pip3 install --no-cache-dir jinja2 jinja2-cli conan==2.*
RUN conan profile detect

WORKDIR /workspace

COPY docker/conan.profile.default /workspace
RUN git clone https://github.com/torquey66/kraken-data-recorder.git && \
    cd kraken-data-recorder && \
    git checkout dev/update-README-and-fix-bugs && \
    conan install . --profile=/workspace/conan.profile.default --build=missing --output-folder=build -sbuild_type=Release

RUN cd kraken-data-recorder/build && \
    cmake -DCMAKE_BUILD_TYPE=Release  -DBUILD_SHARED_LIBS=OFF .. && \
    make -j && \
    conan cache clean --temp --source --build && \
    rm -rf /root/.conan2

RUN wget https://github.com/duckdb/duckdb/releases/download/v1.1.0/duckdb_cli-linux-amd64.zip && \
    unzip duckdb_cli-linux-amd64.zip


FROM alpine as main

COPY --from=build /workspace /

CMD ["/bin/bash"]

