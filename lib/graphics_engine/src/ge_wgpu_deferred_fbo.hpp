#ifndef HEADER_GE_WGPU_DEFERRED_FBO_HPP
#define HEADER_GE_WGPU_DEFERRED_FBO_HPP

#include "dimension2d.h"

#include <webgpu/webgpu_cpp.h>

#include <map>
#include <string>
#include <vector>

namespace GE
{
namespace GEWGPUShaderManager { struct Constants; }

/** Render targets of the deferred PBR path (used for displace, as in the
 *  Vulkan renderer), and the fullscreen passes reading them. The Vulkan
 *  subpasses become separate render passes:
 *  1. G-buffer: color and normal (+ depth) from the mesh shaders
 *  2. lighting: deferred_pbr and point lights into the HDR target, skybox
 *  3. convert: HDR to the displace color target, ghost and transparent
 *  4. displace mask: displace meshes write their screen space shift
 *  5. displace color: displace color shifted by the mask to the output */
class GEWGPUDeferredFBO
{
public:
    static constexpr wgpu::TextureFormat GBUFFER_FORMAT =
        wgpu::TextureFormat::BGRA8Unorm;
    static constexpr wgpu::TextureFormat HDR_FORMAT =
        wgpu::TextureFormat::RGBA16Float;
    static constexpr wgpu::TextureFormat MASK_FORMAT =
        wgpu::TextureFormat::RG8Unorm;
    static constexpr wgpu::TextureFormat DISPLACE_COLOR_FORMAT =
        wgpu::TextureFormat::BGRA8Unorm;
private:
    irr::core::dimension2du m_size;

    wgpu::Texture m_color, m_normal, m_depth, m_hdr, m_mask, m_displace_color;

    wgpu::TextureView m_color_view, m_normal_view, m_depth_view, m_hdr_view,
        m_mask_view, m_displace_color_view;

    wgpu::BindGroupLayout m_gbuffer_layout, m_hdr_layout, m_displace_layout,
        m_displace_mask_layout;

    wgpu::BindGroup m_gbuffer_bind_group, m_hdr_bind_group,
        m_displace_bind_group, m_displace_mask_bind_group;

    wgpu::PipelineLayout m_lighting_layout, m_convert_layout,
        m_displace_color_layout;

    std::map<std::string, wgpu::RenderPipeline> m_pipelines;

    // ------------------------------------------------------------------------
    wgpu::PipelineLayout& getPipelineLayout(wgpu::PipelineLayout& layout,
                         const std::vector<wgpu::BindGroupLayout>& groups);

    // ------------------------------------------------------------------------
    wgpu::RenderPipeline getPipeline(const std::string& vs,
                                     const std::string& fs,
                                     const wgpu::PipelineLayout& layout,
                                     wgpu::TextureFormat format,
                                     const GEWGPUShaderManager::Constants& c,
                                     bool additive, bool depth_test,
                                     bool strip);
public:
    // ------------------------------------------------------------------------
    GEWGPUDeferredFBO(const irr::core::dimension2du& size);
    // ------------------------------------------------------------------------
    const irr::core::dimension2du& getSize() const          { return m_size; }
    // ------------------------------------------------------------------------
    const wgpu::TextureView& getColorView() const     { return m_color_view; }
    // ------------------------------------------------------------------------
    const wgpu::TextureView& getNormalView() const   { return m_normal_view; }
    // ------------------------------------------------------------------------
    const wgpu::TextureView& getDepthView() const     { return m_depth_view; }
    // ------------------------------------------------------------------------
    const wgpu::TextureView& getHDRView() const         { return m_hdr_view; }
    // ------------------------------------------------------------------------
    const wgpu::TextureView& getMaskView() const       { return m_mask_view; }
    // ------------------------------------------------------------------------
    const wgpu::TextureView& getDisplaceColorView() const
                                               { return m_displace_color_view; }
    // ------------------------------------------------------------------------
    /** Group 3 of displace_transparent.frag (mask, ssr, displace color). */
    const wgpu::BindGroup& getDisplaceBindGroup() const
                                               { return m_displace_bind_group; }
    // ------------------------------------------------------------------------
    const wgpu::BindGroupLayout& getDisplaceLayout() const
                                                   { return m_displace_layout; }
    // ------------------------------------------------------------------------
    /** Group 3 of displace_mask.frag (displace color, depth, hiz depth). */
    const wgpu::BindGroup& getDisplaceMaskBindGroup() const
                                          { return m_displace_mask_bind_group; }
    // ------------------------------------------------------------------------
    const wgpu::BindGroupLayout& getDisplaceMaskLayout() const
                                              { return m_displace_mask_layout; }
    // ------------------------------------------------------------------------
    /** Sun, ambient and fullscreen lights (deferred_pbr.frag), then the other
     *  point lights as camera facing quads. */
    void renderLighting(wgpu::RenderPassEncoder& pass,
                        const wgpu::BindGroup& data,
                        const wgpu::BindGroupLayout& data_layout,
                        const wgpu::BindGroup& env,
                        const wgpu::BindGroupLayout& env_layout,
                        const GEWGPUShaderManager::Constants& c,
                        uint32_t deferred_pbr_offset,
                        uint32_t pointlight_offset, unsigned point_lights);
    // ------------------------------------------------------------------------
    /** HDR to the displace color target. */
    void renderConvertColor(wgpu::RenderPassEncoder& pass,
                            const GEWGPUShaderManager::Constants& c);
    // ------------------------------------------------------------------------
    /** Displace color, shifted by the mask if has_displace, to the output. */
    void renderDisplaceColor(wgpu::RenderPassEncoder& pass,
                             wgpu::TextureFormat format,
                             const wgpu::BindGroup& data,
                             const wgpu::BindGroupLayout& data_layout,
                             const GEWGPUShaderManager::Constants& c,
                             uint32_t push_offset);
};   // GEWGPUDeferredFBO

}

#endif
