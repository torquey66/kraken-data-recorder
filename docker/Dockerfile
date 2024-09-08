# Start with the latest Ubuntu base image
FROM ubuntu:latest

# Install necessary packages and tools
RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    clang-format \
    clang-tidy \
    emacs-nox \
    python3 \
    python3-pip \
    python3-venv \
    git \
    cmake \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set up a virtual environment
RUN python3 -m venv /venv
ENV PATH="/venv/bin:$PATH"

# Install Python packages in the virtual environment
RUN pip3 install --no-cache-dir jinja2 jinja2-cli conan==2.*
RUN conan profile detect

# Set up the default work directory
WORKDIR /workspace

COPY docker/conan.profile.default /workspace
RUN git clone https://github.com/torquey66/kraken-data-recorder.git && \
    cd kraken-data-recorder && \
    git checkout dev/update-README-and-fix-bugs && \
    conan install . --profile=/workspace/conan.profile.default --build=missing --output-folder=build -sbuild_type=Release

RUN cd kraken-data-recorder/build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j

# Run a command (optional)
CMD ["/bin/bash"]

