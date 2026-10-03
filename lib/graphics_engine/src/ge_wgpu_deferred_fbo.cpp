#include "ge_wgpu_deferred_fbo.hpp"

#include "ge_wgpu_driver.hpp"
#include "ge_wgpu_shader_manager.hpp"
#include "ge_wgpu_texture.hpp"

#include <array>
#include <vector>

namespace GE
{
namespace
{
// ----------------------------------------------------------------------------
wgpu::Texture createTarget(const irr::core::dimension2du& size,
                           wgpu::TextureFormat format, const char* label,
                           bool depth = false)
{
    wgpu::TextureDescriptor desc;
    desc.label = label;
    desc.size = { size.Width, size.Height, 1 };
    desc.format = format;
    desc.usage = wgpu::TextureUsage::RenderAttachment |
        wgpu::TextureUsage::TextureBinding;
    return getWGPUDriver()->getDevice().CreateTexture(&desc);
}   // createTarget

// ----------------------------------------------------------------------------
/** Texture (read with texelFetch) and sampler pairs, samplers at binding +16
 *  like every converted shader. */
struct Slot
{
    uint32_t m_binding;
    wgpu::TextureSampleType m_sample_type;
    wgpu::SamplerBindingType m_sampler_type;
};

// ----------------------------------------------------------------------------
wgpu::BindGroupLayout createLayout(const std::vector<Slot>& slots,
                                   const char* label)
{
    std::vector<wgpu::BindGroupLayoutEntry> entries;
    for (const Slot& s : slots)
    {
        wgpu::BindGroupLayoutEntry texture;
        texture.binding = s.m_binding;
        texture.visibility = wgpu::ShaderStage::Fragment;
        texture.texture.sampleType = s.m_sample_type;
        texture.texture.viewDimension = wgpu::TextureViewDimension::e2D;
        entries.push_back(texture);
        wgpu::BindGroupLayoutEntry sampler;
        sampler.binding =
            s.m_binding + GEWGPUShaderManager::getSamplerBindingOffset();
        sampler.visibility = wgpu::ShaderStage::Fragment;
        sampler.sampler.type = s.m_sampler_type;
        entries.push_back(sampler);
    }
    wgpu::BindGroupLayoutDescriptor desc;
    desc.label = label;
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    return getWGPUDriver()->getDevice().CreateBindGroupLayout(&desc);
}   // createLayout

// ----------------------------------------------------------------------------
wgpu::BindGroup createBindGroup(const wgpu::BindGroupLayout& layout,
                                const std::vector<Slot>& slots,
                                const std::vector<wgpu::TextureView>& views,
                                const std::vector<wgpu::Sampler>& samplers,
                                const char* label)
{
    std::vector<wgpu::BindGroupEntry> entries;
    for (unsigned i = 0; i < slots.size(); i++)
    {
        wgpu::BindGroupEntry texture;
        texture.binding = slots[i].m_binding;
        texture.textureView = views[i];
        entries.push_back(texture);
        wgpu::BindGroupEntry sampler;
        sampler.binding = slots[i].m_binding +
            GEWGPUShaderManager::getSamplerBindingOffset();
        sampler.sampler = samplers[i];
        entries.push_back(sampler);
    }
    wgpu::BindGroupDescriptor desc;
    desc.label = label;
    desc.layout = layout;
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    return getWGPUDriver()->getDevice().CreateBindGroup(&desc);
}   // createBindGroup

const wgpu::TextureSampleType UNFILTERABLE =
    wgpu::TextureSampleType::UnfilterableFloat;
const wgpu::SamplerBindingType NON_FILTERING =
    wgpu::SamplerBindingType::NonFiltering;

}   // anonymous namespace

// ----------------------------------------------------------------------------
GEWGPUDeferredFBO::GEWGPUDeferredFBO(const irr::core::dimension2du& size)
                 : m_size(size)
{
    GEWGPUDriver* driver = getWGPUDriver();
    m_color = createTarget(size, GBUFFER_FORMAT, "gbuffer color");
    m_normal = createTarget(size, GBUFFER_FORMAT, "gbuffer normal");
    m_depth = createTarget(size, wgpu::TextureFormat::Depth32Float,
        "deferred depth");
    m_hdr = createTarget(size, HDR_FORMAT, "hdr");
    m_mask = createTarget(size, MASK_FORMAT, "displace mask");
    m_displace_color = createTarget(size, DISPLACE_COLOR_FORMAT,
        "displace color");
    m_color_view = m_color.CreateView();
    m_normal_view = m_normal.CreateView();
    m_depth_view = m_depth.CreateView();
    m_hdr_view = m_hdr.CreateView();
    m_mask_view = m_mask.CreateView();
    m_displace_color_view = m_displace_color.CreateView();

    const wgpu::Sampler& nearest = driver->getSampler(GVS_NEAREST);
    const wgpu::TextureView& transparent =
        static_cast<GEWGPUTexture*>(driver->getTransparentTexture())->getView();

    // u_color, u_normal and u_depth of deferred_pbr / deferred_pointlight
    std::vector<Slot> gbuffer = { { 0, UNFILTERABLE, NON_FILTERING },
        { 1, UNFILTERABLE, NON_FILTERING }, { 2, UNFILTERABLE, NON_FILTERING } };
    m_gbuffer_layout = createLayout(gbuffer, "gbuffer");
    m_gbuffer_bind_group = createBindGroup(m_gbuffer_layout, gbuffer,
        { m_color_view, m_normal_view, m_depth_view },
        { nearest, nearest, nearest }, "gbuffer");

    // u_hdr of deferred_convert_color
    std::vector<Slot> hdr = { { 0, UNFILTERABLE, NON_FILTERING } };
    m_hdr_layout = createLayout(hdr, "hdr");
    m_hdr_bind_group = createBindGroup(m_hdr_layout, hdr, { m_hdr_view },
        { nearest }, "hdr");

    // Mask, screen space reflection (not implemented, transparent) and
    // displace color, as GVDFP_DISPLACE_COLOR in GEVulkanDeferredFBO
    std::vector<Slot> displace = { { 0, UNFILTERABLE, NON_FILTERING },
        { 1, UNFILTERABLE, NON_FILTERING }, { 2, UNFILTERABLE, NON_FILTERING } };
    m_displace_layout = createLayout(displace, "displace");
    m_displace_bind_group = createBindGroup(m_displace_layout, displace,
        { m_mask_view, transparent, m_displace_color_view },
        { nearest, nearest, nearest }, "displace");

    // Displace color, depth compared with GVS_SHADOW and hiz depth (not
    // implemented, transparent), as GVDFP_DISPLACE_MASK
    std::vector<Slot> mask = { { 0, UNFILTERABLE, NON_FILTERING },
        { 1, wgpu::TextureSampleType::Depth,
        wgpu::SamplerBindingType::Comparison },
        { 2, UNFILTERABLE, NON_FILTERING } };
    m_displace_mask_layout = createLayout(mask, "displace mask");
    m_displace_mask_bind_group = createBindGroup(m_displace_mask_layout, mask,
        { m_displace_color_view, m_depth_view, transparent },
        { nearest, driver->getSampler(GVS_SHADOW), nearest },
        "displace mask");
}   // GEWGPUDeferredFBO

// ----------------------------------------------------------------------------
wgpu::PipelineLayout& GEWGPUDeferredFBO::getPipelineLayout(
                                               wgpu::PipelineLayout& layout,
                             const std::vector<wgpu::BindGroupLayout>& groups)
{
    if (!layout)
    {
        wgpu::PipelineLayoutDescriptor desc;
        desc.bindGroupLayoutCount = groups.size();
        desc.bindGroupLayouts = groups.data();
        layout = getWGPUDriver()->getDevice().CreatePipelineLayout(&desc);
    }
    return layout;
}   // getPipelineLayout

// ----------------------------------------------------------------------------
wgpu::RenderPipeline GEWGPUDeferredFBO::getPipeline(const std::string& vs,
                                                    const std::string& fs,
                                         const wgpu::PipelineLayout& layout,
                                                   wgpu::TextureFormat format,
                                       const GEWGPUShaderManager::Constants& c,
                                                    bool additive,
                                                    bool depth_test, bool strip)
{
    std::string key = vs + fs + std::to_string((int)format) +
        (c.m_ibl ? "i" : "") + (c.m_has_skybox ? "s" : "");
    auto it = m_pipelines.find(key);
    if (it != m_pipelines.end())
        return it->second;

    wgpu::BlendState blend;
    blend.color = { wgpu::BlendOperation::Add, wgpu::BlendFactor::One,
        wgpu::BlendFactor::One };
    blend.alpha = blend.color;
    wgpu::ColorTargetState target;
    target.format = format;
    if (additive)
        target.blend = &blend;
    wgpu::FragmentState fragment;
    fragment.module = GEWGPUShaderManager::getShader(fs);
    fragment.entryPoint = "main";
    fragment.targetCount = 1;
    fragment.targets = &target;
    std::vector<wgpu::ConstantEntry> fs_constants =
        GEWGPUShaderManager::getConstants(fs, c);
    fragment.constantCount = fs_constants.size();
    fragment.constants = fs_constants.data();

    // Every pass has the depth buffer attached, read only here
    wgpu::DepthStencilState depth;
    depth.format = wgpu::TextureFormat::Depth32Float;
    depth.depthWriteEnabled = wgpu::OptionalBool::False;
    depth.depthCompare = depth_test ? wgpu::CompareFunction::Less :
        wgpu::CompareFunction::Always;

    wgpu::RenderPipelineDescriptor desc;
    desc.label = key.c_str();
    desc.layout = layout;
    desc.vertex.module = GEWGPUShaderManager::getShader(vs);
    desc.vertex.entryPoint = "main";
    std::vector<wgpu::ConstantEntry> vs_constants =
        GEWGPUShaderManager::getConstants(vs, c);
    desc.vertex.constantCount = vs_constants.size();
    desc.vertex.constants = vs_constants.data();
    desc.primitive.topology = strip ? wgpu::PrimitiveTopology::TriangleStrip :
        wgpu::PrimitiveTopology::TriangleList;
    desc.depthStencil = &depth;
    desc.fragment = &fragment;
    wgpu::RenderPipeline pipeline =
        getWGPUDriver()->getDevice().CreateRenderPipeline(&desc);
    m_pipelines[key] = pipeline;
    return pipeline;
}   // getPipeline

// ----------------------------------------------------------------------------
void GEWGPUDeferredFBO::renderLighting(wgpu::RenderPassEncoder& pass,
                                       const wgpu::BindGroup& data,
                                       const wgpu::BindGroupLayout& data_layout,
                                       const wgpu::BindGroup& env,
                                       const wgpu::BindGroupLayout& env_layout,
                                       const GEWGPUShaderManager::Constants& c,
                                       uint32_t deferred_pbr_offset,
                                       uint32_t pointlight_offset,
                                       unsigned point_lights)
{
    const wgpu::PipelineLayout& layout = getPipelineLayout(m_lighting_layout,
        { m_gbuffer_layout, data_layout, env_layout });

    pass.SetPipeline(getPipeline("fullscreen_quad.vert", "deferred_pbr.frag",
        layout, HDR_FORMAT, c, false/*additive*/, false/*depth_test*/,
        false/*strip*/));
    pass.SetBindGroup(0, m_gbuffer_bind_group);
    pass.SetBindGroup(1, data, 1, &deferred_pbr_offset);
    pass.SetBindGroup(2, env);
    pass.Draw(3);
    if (point_lights == 0)
        return;
    pass.SetPipeline(getPipeline("deferred_pointlight.vert",
        "deferred_pointlight.frag", layout, HDR_FORMAT, c, true/*additive*/,
        true/*depth_test*/, true/*strip*/));
    pass.SetBindGroup(1, data, 1, &pointlight_offset);
    pass.Draw(4, point_lights);
}   // renderLighting

// ----------------------------------------------------------------------------
void GEWGPUDeferredFBO::renderConvertColor(wgpu::RenderPassEncoder& pass,
                                       const GEWGPUShaderManager::Constants& c)
{
    const wgpu::PipelineLayout& layout =
        getPipelineLayout(m_convert_layout, { m_hdr_layout });
    pass.SetPipeline(getPipeline("fullscreen_quad.vert",
        "deferred_convert_color.frag", layout, DISPLACE_COLOR_FORMAT, c,
        false, false, false));
    pass.SetBindGroup(0, m_hdr_bind_group);
    pass.Draw(3);
}   // renderConvertColor

// ----------------------------------------------------------------------------
void GEWGPUDeferredFBO::renderDisplaceColor(wgpu::RenderPassEncoder& pass,
                                            wgpu::TextureFormat format,
                                            const wgpu::BindGroup& data,
                                       const wgpu::BindGroupLayout& data_layout,
                                       const GEWGPUShaderManager::Constants& c,
                                            uint32_t push_offset)
{
    const wgpu::PipelineLayout& layout = getPipelineLayout(
        m_displace_color_layout, { m_displace_layout, data_layout });
    pass.SetPipeline(getPipeline("fullscreen_quad.vert", "displace_color.frag",
        layout, format, c, false, false, false));
    pass.SetBindGroup(0, m_displace_bind_group);
    pass.SetBindGroup(1, data, 1, &push_offset);
    pass.Draw(3);
}   // renderDisplaceColor

}
