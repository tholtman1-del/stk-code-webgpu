#ifndef HEADER_GE_WGPU_FBO_TEXTURE_HPP
#define HEADER_GE_WGPU_FBO_TEXTURE_HPP

#include "ge_wgpu_texture.hpp"

namespace GE
{
/** Render target texture (race minimap, kart previews), with its own depth
 *  buffer. Sampled like any other texture afterwards. */
class GEWGPUFBOTexture : public GEWGPUTexture
{
private:
    wgpu::Texture m_depth_texture;

    wgpu::TextureView m_depth_view;
public:
    // ------------------------------------------------------------------------
    GEWGPUFBOTexture(const core::dimension2d<u32>& size,
                     const std::string& name);
    // ------------------------------------------------------------------------
    virtual void reload()                                                    {}
    // ------------------------------------------------------------------------
    const wgpu::TextureView& getColorView() const          { return m_view; }
    // ------------------------------------------------------------------------
    const wgpu::TextureView& getDepthView() const     { return m_depth_view; }
    // ------------------------------------------------------------------------
    wgpu::TextureFormat getFormat() const                 { return m_format; }
};   // GEWGPUFBOTexture

}

#endif
