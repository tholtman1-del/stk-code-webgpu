#!/bin/bash
# Builds the third-party libraries that Emscripten does not ship as ports
# into build-web/deps. Ports (SDL2, zlib, png, jpeg, freetype, harfbuzz,
# ogg, vorbis) are prebuilt into the emsdk cache with embuilder.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
EMSDK="${EMSDK:-$ROOT/../emsdk}"
source "$EMSDK/emsdk_env.sh" > /dev/null 2>&1

DEPS="$ROOT/build-web/deps"
SRC="$DEPS/src"
PREFIX="$DEPS/prefix"
mkdir -p "$SRC" "$PREFIX"
FLAGS="-pthread -O3 -msimd128"

# Emscripten ports, multithreaded variants. Their sources are cloned with git
# because GitHub archive downloads are not reachable from every environment.
python3 "$ROOT/tools/webgpu/fetch_ports.py"
embuilder build sdl2-mt zlib libpng-mt libjpeg freetype harfbuzz-mt \
    ogg vorbis > /dev/null

# MbedTLS (crypto for online play)
MBEDTLS_VER=3.6.4
if [ ! -f "$PREFIX/lib/libmbedcrypto.a" ]; then
    cd "$SRC"
    if [ ! -d "mbedtls-$MBEDTLS_VER" ]; then
        git -c advice.detachedHead=false clone -q --depth 1 --recurse-submodules \
            --shallow-submodules --branch "mbedtls-$MBEDTLS_VER" \
            https://github.com/Mbed-TLS/mbedtls "mbedtls-$MBEDTLS_VER"
    fi
    emcmake cmake -S "mbedtls-$MBEDTLS_VER" -B "mbedtls-build" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DCMAKE_C_FLAGS="$FLAGS" -DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF \
        -DUSE_SHARED_MBEDTLS_LIBRARY=OFF > /dev/null
    cmake --build mbedtls-build --target install > /dev/null
fi
echo "deps ready in $PREFIX"
