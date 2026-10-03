#!/bin/bash
# Installs the tools the WebGPU browser build needs on a fresh Linux or macOS
# machine (e.g. a cloud session): Emscripten, glslang, SPIRV-Tools and naga.
# Emscripten goes to ../emsdk next to the repository, which is where
# build_deps.sh looks for it.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
EMSDK="${EMSDK:-$ROOT/../emsdk}"

if [ ! -d "$EMSDK" ]; then
    git clone --depth 1 https://github.com/emscripten-core/emsdk.git "$EMSDK"
fi
"$EMSDK/emsdk" install latest
"$EMSDK/emsdk" activate latest

if ! command -v glslangValidator > /dev/null || ! command -v spirv-opt > /dev/null; then
    if command -v apt-get > /dev/null; then
        sudo apt-get update
        sudo apt-get install -y glslang-tools spirv-tools cmake ninja-build
    elif command -v brew > /dev/null; then
        brew install glslang spirv-tools cmake ninja
    fi
fi

# compile_shaders.py needs --split-combined-image-sampler (SPIRV-Tools
# 2024.4+), newer than some distributions ship
if ! spirv-opt --help 2>/dev/null | grep -q split-combined-image-sampler; then
    SRC="$ROOT/../spirv-tools-src"
    if [ ! -d "$SRC" ]; then
        git clone --depth 1 https://github.com/KhronosGroup/SPIRV-Tools "$SRC"
        git clone --depth 1 https://github.com/KhronosGroup/SPIRV-Headers \
            "$SRC/external/spirv-headers"
    fi
    cmake -S "$SRC" -B "$SRC/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DSPIRV_SKIP_TESTS=ON -DSPIRV_WERROR=OFF
    cmake --build "$SRC/build"
    sudo cmake --install "$SRC/build" --prefix /usr/local
    hash -r
fi

if ! command -v naga > /dev/null; then
    if ! command -v cargo > /dev/null; then
        curl -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal
        source "$HOME/.cargo/env"
    fi
    # naga-cli needs a recent Rust, install one next to the default toolchain
    rustup toolchain install 1.90 --profile minimal
    cargo +1.90 install naga-cli --locked
fi

echo "Toolchain ready. Next: source $EMSDK/emsdk_env.sh && tools/webgpu/build_deps.sh"
