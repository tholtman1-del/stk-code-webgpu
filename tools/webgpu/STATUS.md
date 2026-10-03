# WebGPU browser port: status

Goal: run SuperTuxKart fully in the browser with a WebGPU renderer only (no
WebGL fallback), for the best performance. Online multiplayer is out of scope
for the first version.

## Setup

```sh
tools/webgpu/setup_toolchain.sh     # emsdk, glslang, spirv-tools, naga
source ../emsdk/emsdk_env.sh
tools/webgpu/build_deps.sh          # Emscripten ports + mbedtls for wasm
python3 tools/webgpu/compile_shaders.py   # GLSL -> WGSL (output is committed)
```

## Architecture

- `EDT_WEBGPU` is a new Irrlicht driver type, selected automatically in
  Emscripten builds (`src/graphics/irr_driver.cpp`).
- `GE::GEDriver` (`lib/graphics_engine/include/ge_driver.hpp`) is the interface
  shared by the Vulkan and WebGPU GE renderers. Game code uses
  `GE::getGEDriver()` and `GE::isGEDriver()` instead of Vulkan types.
- `GEWGPUDriver` and related `ge_wgpu_*` files sit next to the Vulkan ones in
  `lib/graphics_engine`. GE's API-neutral parts (SPM meshes, culling, material
  manager, mipmap generation) are reused.
- Shaders: `data/shaders/ge_shaders` (GLSL) is converted offline to
  `data/shaders/ge_wgsl/{basic,pbr}`. Samplers are moved 16 bindings above
  their textures (`SAMPLER_BINDING_OFFSET`). Subpass inputs become texelFetch
  under `GE_WEBGPU`.
- The WebGPU device is created by the page and passed in through
  `Module.preinitializedWebGPUDevice`, since device creation is asynchronous.
- All WebGPU calls run on the browser main thread. Other threads queue work
  with `GEWGPUDriver::runOnMainThread()`.

## Done

- Toolchain and dependency scripts, Emscripten CMake configuration
- Shader conversion for all 27 GE shaders (both variants)
- Game code decoupled from Vulkan types
- Draft WebGPU driver: surface, samplers, textures, 2D/GUI renderer

Nothing has been compiled with Emscripten yet.

## Next

1. `lib/graphics_engine/CMakeLists.txt`: in Emscripten builds use the
   `ge_wgpu_*` sources instead of `ge_vulkan_*`, `gl.c`, `vulkan.c`,
   `ge_gl_texture.cpp` and `ge_dx9_texture.cpp`
2. `CIrrDeviceSDL.cpp`: `EDT_WEBGPU` case for window, driver and GE scene
3. `src/online/http_request_fetch.cpp` (emscripten_fetch, synchronous on the
   request thread)
4. Split `MainLoop::run()` into `runFrame()`, drive it with
   `emscripten_set_main_loop`
5. HTML/JS loader: request adapter and device, set
   `Module.preinitializedWebGPUDevice`, COOP/COEP headers for threads
6. Package game data, first build, boot to the main menu
7. 3D: mesh buffers, draw calls, render targets, then deferred PBR, skybox,
   IBL, shadows
8. Performance: async texture decoding, compressed textures, asset streaming
