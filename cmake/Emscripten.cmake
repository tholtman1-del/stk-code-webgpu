# Settings for the browser (WebAssembly + WebGPU) build.
# Included from the top-level CMakeLists.txt when EMSCRIPTEN is set.
#
# Configure with tools/webgpu/build_web.sh, which runs emcmake and builds the
# non-port dependencies first.

set(STK_WEB_DEPS_PREFIX "${PROJECT_SOURCE_DIR}/build-web/deps/prefix" CACHE PATH
    "Prefix holding wasm builds of dependencies that are not Emscripten ports")

# Features that cannot work in a browser, or are replaced by browser APIs
set(USE_WIIUSE OFF CACHE BOOL "" FORCE)
set(USE_SQLITE3 OFF CACHE BOOL "" FORCE)
set(BUILD_RECORDER OFF CACHE BOOL "" FORCE)
set(USE_DNS_C OFF CACHE BOOL "" FORCE)
set(USE_SYSTEM_ENET OFF CACHE BOOL "" FORCE)
set(USE_SYSTEM_SQUISH OFF CACHE BOOL "" FORCE)
set(USE_SYSTEM_MCPP OFF CACHE BOOL "" FORCE)
set(USE_SYSTEM_ANGELSCRIPT OFF CACHE BOOL "" FORCE)
set(USE_GLES2 OFF CACHE BOOL "" FORCE)
set(USE_MOJOAL OFF CACHE BOOL "" FORCE)
set(USE_CRYPTO_OPENSSL OFF CACHE BOOL "" FORCE)
set(CHECK_ASSETS OFF CACHE BOOL "" FORCE)
# GLSL is compiled to WGSL offline (tools/webgpu/compile_shaders.py)
set(NO_SHADERC ON CACHE BOOL "" FORCE)

# Threads need cross-origin isolation (COOP/COEP headers) on the page.
# SIMD is supported by every browser that ships WebGPU.
set(STK_WEB_PORTS
    -sUSE_SDL=2 -sUSE_ZLIB=1 -sUSE_LIBPNG=1 -sUSE_LIBJPEG=1
    -sUSE_FREETYPE=1 -sUSE_HARFBUZZ=1 -sUSE_OGG=1 -sUSE_VORBIS=1)
string(REPLACE ";" " " STK_WEB_PORTS_STR "${STK_WEB_PORTS}")
set(STK_WEB_CFLAGS "-pthread -msimd128 -fwasm-exceptions ${STK_WEB_PORTS_STR}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${STK_WEB_CFLAGS}")
# emdawnwebgpu refuses to link C programs, which would fail every C
# try_compile (feature checks), so only C++ gets it
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STK_WEB_CFLAGS} --use-port=emdawnwebgpu")


# Threads come from -pthread, there is no separate pthread library to find
set(PTHREAD_LIBRARY "-pthread" CACHE STRING "" FORCE)

# Point the find_* calls at the ports. The "library" is the port flag, which
# emcc resolves at link time.
set(EM_SYSROOT_INCLUDE "${EMSCRIPTEN_SYSROOT}/include")
set(ZLIB_LIBRARY "-sUSE_ZLIB=1" CACHE STRING "" FORCE)
set(ZLIB_INCLUDE_DIR "${EM_SYSROOT_INCLUDE}" CACHE PATH "" FORCE)
set(PNG_LIBRARY "-sUSE_LIBPNG=1" CACHE STRING "" FORCE)
set(PNG_PNG_INCLUDE_DIR "${EM_SYSROOT_INCLUDE}" CACHE PATH "" FORCE)
set(JPEG_LIBRARY "-sUSE_LIBJPEG=1" CACHE STRING "" FORCE)
set(JPEG_INCLUDE_DIR "${EM_SYSROOT_INCLUDE}" CACHE PATH "" FORCE)
set(SDL2_LIBRARY "-sUSE_SDL=2" CACHE STRING "" FORCE)
set(SDL2_INCLUDEDIR "${EM_SYSROOT_INCLUDE}/SDL2" CACHE PATH "" FORCE)
set(FREETYPE_LIBRARY "-sUSE_FREETYPE=1" CACHE STRING "" FORCE)
set(FREETYPE_INCLUDE_DIRS "${EM_SYSROOT_INCLUDE}/freetype2" CACHE PATH "" FORCE)
set(FREETYPE_LIBRARIES "-sUSE_FREETYPE=1")
set(FREETYPE_FOUND TRUE)
set(HARFBUZZ_LIBRARY "-sUSE_HARFBUZZ=1" CACHE STRING "" FORCE)
set(HARFBUZZ_INCLUDEDIR "${EM_SYSROOT_INCLUDE}/harfbuzz" CACHE PATH "" FORCE)
set(OGGVORBIS_FOUND TRUE)
set(OGGVORBIS_INCLUDE_DIRS "${EM_SYSROOT_INCLUDE}")
set(OGGVORBIS_LIBRARIES "-sUSE_OGG=1" "-sUSE_VORBIS=1")
# Emscripten's OpenAL is implemented on top of Web Audio
set(OPENAL_LIBRARY "-lopenal" CACHE STRING "" FORCE)
set(OPENAL_INCLUDE_DIR "${EM_SYSROOT_INCLUDE}" CACHE PATH "" FORCE)
set(MBEDTLS_INCLUDE_DIRS "${STK_WEB_DEPS_PREFIX}/include" CACHE PATH "" FORCE)
set(MBEDCRYPTO_LIBRARY "${STK_WEB_DEPS_PREFIX}/lib/libmbedcrypto.a" CACHE FILEPATH "" FORCE)
set(MBEDTLS_LIBRARY "${STK_WEB_DEPS_PREFIX}/lib/libmbedtls.a" CACHE FILEPATH "" FORCE)
set(MBEDX509_LIBRARY "${STK_WEB_DEPS_PREFIX}/lib/libmbedx509.a" CACHE FILEPATH "" FORCE)

add_definitions(-DNO_IRR_COMPILE_WITH_X11_
    -DNO_IRR_COMPILE_WITH_OPENGL_ -DNO_IRR_COMPILE_WITH_VULKAN_
    -DNO_IRR_COMPILE_WITH_WAYLAND_DEVICE_ -D_IRR_COMPILE_WITH_WEBGPU_)

# Link settings for the final module, called after add_executable()
function(stk_web_configure_target target)
    target_link_options(${target} PRIVATE
        ${STK_WEB_PORTS}
        -pthread -fwasm-exceptions --use-port=emdawnwebgpu
        -sPTHREAD_POOL_SIZE=12
        -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=512MB -sMAXIMUM_MEMORY=4GB
        -sSTACK_SIZE=4MB -sDEFAULT_PTHREAD_STACK_SIZE=1MB
        -sFETCH=1 -sFORCE_FILESYSTEM=1 -lidbfs.js
        -sENVIRONMENT=web,worker -sEXIT_RUNTIME=0
        -sEXPORTED_RUNTIME_METHODS=callMain,FS,IDBFS,ENV,UTF8ToString
        -sINVOKE_RUN=0
        -sMODULARIZE=1 -sEXPORT_NAME=createSTK)
    set_target_properties(${target} PROPERTIES SUFFIX ".js")
endfunction()
