# Start with the latest Ubuntu base image
FROM ubuntu:latest

LABEL org.opencontainers.image.source=https://github.com/torquey66/kraken-data-recorder
LABEL org.opencontainers.image.description="Kraken Data Recorder"
LABEL org.opencontainers.image.licenses="Creative Commons Attribution-NoDerivatives 4.0 International"

# Install necessary packages and tools
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

# Set up a virtual environment
RUN python3 -m venv /venv
ENV PATH="/venv/bin:$PATH"

# Install Python packages in the virtual environment
RUN pip3 install --no-cache-dir jinja2 jinja2-cli conan==2.*
RUN conan profile detect

WORKDIR /workspace

COPY docker/conan.profile.default /workspace
RUN git clone https://github.com/torquey66/kraken-data-recorder.git && \
    cd kraken-data-recorder && \
    git checkout dev/update-README-and-fix-bugs && \
    conan install . --profile=/workspace/conan.profile.default --build=missing --output-folder=build -sbuild_type=Release

RUN cd kraken-data-recorder/build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j && \
    conan cache clean --temp --source --build && \
    rm -rf /root/.conan2

RUN wget https://github.com/duckdb/duckdb/releases/download/v1.1.0/duckdb_cli-linux-amd64.zip && \
    unzip duckdb_cli-linux-amd64.zip

CMD ["/bin/bash"]

