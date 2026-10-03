#ifndef HEADER_GE_WGPU_SHADER_MANAGER_HPP
#define HEADER_GE_WGPU_SHADER_MANAGER_HPP

#include <webgpu/webgpu_cpp.h>

#include <string>

namespace irr
{
    namespace io { class IFileSystem; }
}

namespace GE
{
/** Loads the WGSL shaders generated offline from data/shaders/ge_shaders by
 *  tools/webgpu/compile_shaders.py. One variant directory exists for each
 *  combination of compile time defines (currently PBR on or off). */
namespace GEWGPUShaderManager
{
// ----------------------------------------------------------------------------
void init(const wgpu::Device& device, irr::io::IFileSystem* fs);
// ----------------------------------------------------------------------------
void destroy();
// ----------------------------------------------------------------------------
/** Drops all cached modules so they are re-read from disk on next use. */
void reload();
// ----------------------------------------------------------------------------
/** Returns the module for a GLSL file name, e.g. "2d_render.frag". */
wgpu::ShaderModule getShader(const std::string& filename);
// ----------------------------------------------------------------------------
/** Number of mesh texture layers bound per material (2, or 8 with PBR). */
unsigned getMeshTextureLayer();
// ----------------------------------------------------------------------------
/** Binding offset of a sampler relative to the texture it samples, must match
 *  SAMPLER_BINDING_OFFSET in compile_shaders.py. */
constexpr unsigned getSamplerBindingOffset()                    { return 16; }
};   // GEWGPUShaderManager

}

#endif
