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
emcmake cmake -S . -B build-web/stk -G Ninja
cmake --build build-web/stk --target supertuxkart  # -> build-web/stk/bin
tools/webgpu/fetch_assets.sh        # stk-assets from SVN into ../stk-assets
python3 tools/webgpu/package_data.py      # stk-data.bin/.json into bin/
python3 tools/webgpu/web/serve.py         # http://localhost:8080/
node tools/webgpu/web/boot_test.mjs       # headless boot + screenshot
```

Cloud session notes:
- `build_deps.sh` clones the Emscripten ports' sources with git
  (`fetch_ports.py`) because GitHub archive downloads are blocked.
- svn ignores `HTTPS_PROXY`; set the proxy in `~/.subversion/servers`
  (see `fetch_assets.sh`).
- `boot_test.mjs` uses SwiftShader WebGPU plus ANGLE/SwiftShader GL; without
  the GL flags Chromium loses the device on the first canvas present.
- First-run clicks to reach the main menu at 1280x720: No (457,540),
  OK (760,630), OK (1075,620), No (510,490).

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
- Emscripten configure works; `graphics_engine` compiles for wasm. Its
  Emscripten source list is the shared GE files plus `ge_wgpu_*`; the
  compressors and culling tool are Vulkan-only for now.
  `GESPMBuffer::createVertexIndexBuffer()` is a no-op without Vulkan until
  WebGPU mesh buffers exist (Next 2).
- `CIrrDeviceSDL`: `EDT_WEBGPU` window (no GL/Vulkan flags) and driver;
  Irrlicht's `CSceneManager` until a WebGPU GE scene manager exists.
  `stkirrlicht` compiles for wasm. HiDPI: `SDL_WINDOW_ALLOW_HIGHDPI`, the
  driver's ScreenSize is the canvas size in pixels
  (`SDL_GetWindowSizeInPixels`), mouse input is scaled by CIrrDeviceSDL.
  `DPR=2 node boot_test.mjs ...` tests it.
- `src/online/http_request_fetch.cpp`: synchronous `emscripten_fetch` on the
  request thread. No progress during a transfer, cancel only checked before
  and after, servers need CORS headers for the game's origin.
- `MainLoop::runFrame()`, driven by `emscripten_set_main_loop_arg` in the
  browser (no fps sleeps). On abort it saves player data and config and
  stops; exceptions in a frame are no longer caught by `main()`.
- `tools/webgpu/web/`: `index.html` loader (adapter/device, limits,
  device-loss and environment errors, wasm download progress, IDBFS at
  `/persistent`) and `serve.py` (COOP/COEP).
- The `supertuxkart` target links (10 MB wasm). glad's `gl.c` is linked so
  the OpenGL code paths resolve; they never run without the OpenGL driver.
  DNS queries are skipped in the browser.
- Game data: `package_data.py` packs the core files (95 MB: `data/`
  without translations and replays, karts, models, sfx, small and XML
  files, and the files in `core_files.txt` read before the main menu) into
  one blob, which the page downloads in parallel with the engine and
  unpacks into MEMFS at `/stk` before `main()`. The other files (667 MB:
  textures, tracks, music, ...) are in `stk-files/` and fetched with a
  synchronous XHR when the game first reads them (a hacienda race streams
  85 files, 18 MB). After asset changes, refresh `core_files.txt` with
  `update_core_list.py` (see the script). `--no-streaming` makes one blob.
- Boots to the main menu (2D GUI on WebGPU, mouse input, first-run
  dialogs, player creation, config saved to IDBFS).
- 3D without PBR: `GEWGPUSceneManager`, `GEWGPUCameraSceneNode`,
  `GEWGPUMeshCache` (all SPM meshes in one buffer, base vertex draws),
  `GEWGPUDrawCall` (culling, instancing with per instance data in a storage
  buffer, skinning, one bind group per material texture set, solid / ghost /
  transparent passes), dynamic mesh buffers (skid marks, shadows), billboards,
  particles and the skybox. Races render (tested on hacienda).
  `ObjectData` and `GECullingTool` are shared with the Vulkan renderer.
- Render to texture: `GEWGPUFBOTexture` (color + depth), drawn right away
  in `GEWGPUSceneManager::drawAll()` while it is the render target, like
  the Vulkan renderer. Race minimap and kart selection previews work.
- Forward PBR: lights (the light handler is shared with Vulkan), sRGB
  material views, and image based lighting (skybox mipmaps, diffuse and
  specular environment maps rendered by the compute shaders when IBL is on,
  `--enable-ibl`). Specialization constants are WGSL override constants set
  per pipeline (`GEWGPUShaderManager::getConstants`).
- Shader conversion: normals and tangents (A2B10G10R10 snorm, no WebGPU
  vertex format) are read as u32 and unpacked in WGSL; push constants become
  a uniform at `@group(1) @binding(4)` with a dynamic offset.
- Deferred rendering (`GEWGPUDeferredFBO`), used like the Vulkan renderer
  only for tracks with displace (water) materials: G-buffer, deferred PBR
  and point lights, skybox, convert to the displace color target with ghost
  and transparent meshes, displace mask, displace color to the output. The
  Vulkan subpasses are separate render passes. Tested on zengarden and
  gran_paradiso_island. Fragment WGSL turns off `derivative_uniformity`.
- Screen space reflections (`--enable-ssr`, HiZ with geometry level 3-5):
  SSR target in the deferred FBO, `GEWGPUHiZDepth` builds the min depth
  pyramid per camera with `hiz_depth.comp`.
- PBR depth prepass (`GWPT_DEPTH`, then solid with an equal depth test), as
  the Vulkan renderer on non-tiled GPUs.
- Textures are decoded (and mipmapped) by loader threads
  (`GEWGPUTextureLoader`); files are read on the creating thread, the
  getters wait for decoding, `getView()` uploads on demand. Hacienda race
  start 9.5 s -> 5.5 s under SwiftShader.
- Optional BC3 texture compression (see Next).
- Render resolution (video options slider, `--rtt-scale=50`): the 3D scene
  is drawn to a smaller target (also the deferred FBO) and upscaled
  bilinearly before the GUI, which stays at full resolution. Useful with
  HiDPI, which renders devicePixelRatio² as many pixels.
- Browser logs go to `console.log/warn/error` without terminal colour codes.
  Exceptions in a frame are caught like `main()` does and stop the game;
  the page shows `Module.onGameStopped` (also after quitting).

## Next

0. Test on real GPUs and browsers (everything so far ran on SwiftShader in
   headless Chromium): Chrome/Edge, Safari, Firefox; split screen, story
   mode, gamepads; hosting with COOP/COEP and caching of `stk-files/`.

1. Texture compression: BC3 (GECompressorS3TCBC3 on the loader threads)
   works but is off by default in the browser (`enable_texture_compression`):
   it slows loading (hacienda start 5.5 s -> 8.9 s) and makes packed PBR
   maps look washed out. BC7 (bc7enc is Vulkan-only in CMake) or
   compressing offline in `package_data.py` would fix both.
   Streaming: one request per file while a track loads (could be bundled
   per track), streamed files stay in memory once read.
   The depth prepass could be skipped on tiled GPUs (Apple, mobile) if the
   adapter info allows telling them apart
