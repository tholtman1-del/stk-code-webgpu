#include "ge_wgpu_fbo_texture.hpp"

#include "ge_wgpu_driver.hpp"

namespace GE
{
// ----------------------------------------------------------------------------
GEWGPUFBOTexture::GEWGPUFBOTexture(const core::dimension2d<u32>& size,
                                   const std::string& name)
                : GEWGPUTexture(name, false/*single_channel*/)
{
    m_size = m_orig_size = size;
    m_has_mipmaps = false;
    createTexture(wgpu::TextureFormat::BGRA8Unorm,
        wgpu::TextureUsage::RenderAttachment |
        wgpu::TextureUsage::TextureBinding);

    wgpu::TextureDescriptor desc;
    desc.label = "rtt depth";
    desc.size = { size.Width, size.Height, 1 };
    desc.format = wgpu::TextureFormat::Depth32Float;
    desc.usage = wgpu::TextureUsage::RenderAttachment;
    m_depth_texture = m_driver->getDevice().CreateTexture(&desc);
    m_depth_view = m_depth_texture.CreateView();
}   // GEWGPUFBOTexture

}
