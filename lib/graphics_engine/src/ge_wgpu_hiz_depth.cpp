#include "ge_wgpu_hiz_depth.hpp"

#include "ge_wgpu_deferred_fbo.hpp"
#include "ge_wgpu_driver.hpp"
#include "ge_wgpu_shader_manager.hpp"

#include <algorithm>
#include <array>

namespace GE
{
namespace
{
// Same as the draw call's push constants slots (minUniformBufferOffsetAlignment)
constexpr uint32_t PUSH_CONSTANTS_SLOT = 256;
// displace_mask.frag traces up to HIZ_MAX_LEVEL (6)
constexpr uint32_t MAX_LEVELS = 7;

struct HiZPushConstants
{
    int32_t m_offset_x;
    int32_t m_offset_y;
    int32_t m_level;
    int32_t m_padding;
};
}   // anonymous namespace

// ----------------------------------------------------------------------------
void GEWGPUHiZDepth::prepare(const irr::core::recti& viewport,
                             const GEWGPUDeferredFBO* dfbo)
{
    if (m_viewport != viewport || m_dfbo != dfbo || !m_hiz_depth)
    {
        m_viewport = viewport;
        m_dfbo = dfbo;
        init();
    }
}   // prepare

// ----------------------------------------------------------------------------
void GEWGPUHiZDepth::init()
{
    GEWGPUDriver* driver = getWGPUDriver();
    const wgpu::Device& device = driver->getDevice();
    const uint32_t width = std::max(m_viewport.getWidth(), 1);
    const uint32_t height = std::max(m_viewport.getHeight(), 1);
    uint32_t levels = 1;
    while (levels < MAX_LEVELS && (std::max(width, height) >> levels) > 0)
        levels++;

    wgpu::TextureDescriptor tex_desc;
    tex_desc.label = "hiz depth";
    tex_desc.size = { width, height, 1 };
    tex_desc.mipLevelCount = levels;
    tex_desc.format = wgpu::TextureFormat::R32Float;
    tex_desc.usage = wgpu::TextureUsage::TextureBinding |
        wgpu::TextureUsage::StorageBinding;
    m_hiz_depth = device.CreateTexture(&tex_desc);

    std::array<wgpu::BindGroupLayoutEntry, 3> entries = {};
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Compute;
    entries[0].texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
    entries[0].texture.viewDimension = wgpu::TextureViewDimension::e2D;
    entries[1].binding = GEWGPUShaderManager::getSamplerBindingOffset();
    entries[1].visibility = wgpu::ShaderStage::Compute;
    entries[1].sampler.type = wgpu::SamplerBindingType::NonFiltering;
    entries[2].binding = 1;
    entries[2].visibility = wgpu::ShaderStage::Compute;
    entries[2].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
    entries[2].storageTexture.format = wgpu::TextureFormat::R32Float;
    entries[2].storageTexture.viewDimension = wgpu::TextureViewDimension::e2D;
    wgpu::BindGroupLayoutDescriptor bgl_desc;
    bgl_desc.entryCount = entries.size();
    bgl_desc.entries = entries.data();
    wgpu::BindGroupLayout image_layout = device.CreateBindGroupLayout(&bgl_desc);

    wgpu::BindGroupLayoutEntry pc_entry;
    pc_entry.binding = 4;
    pc_entry.visibility = wgpu::ShaderStage::Compute;
    pc_entry.buffer.type = wgpu::BufferBindingType::Uniform;
    pc_entry.buffer.minBindingSize = sizeof(HiZPushConstants);
    bgl_desc.entryCount = 1;
    bgl_desc.entries = &pc_entry;
    wgpu::BindGroupLayout pc_layout = device.CreateBindGroupLayout(&bgl_desc);

    std::array<wgpu::BindGroupLayout, 2> layouts = {{ image_layout, pc_layout }};
    wgpu::PipelineLayoutDescriptor pl_desc;
    pl_desc.bindGroupLayoutCount = layouts.size();
    pl_desc.bindGroupLayouts = layouts.data();
    wgpu::ComputePipelineDescriptor cp_desc;
    cp_desc.label = "hiz_depth.comp";
    cp_desc.layout = device.CreatePipelineLayout(&pl_desc);
    cp_desc.compute.module = GEWGPUShaderManager::getShader("hiz_depth.comp");
    cp_desc.compute.entryPoint = "main";
    m_pipeline = device.CreateComputePipeline(&cp_desc);

    std::vector<uint8_t> pc_data(levels * PUSH_CONSTANTS_SLOT, 0);
    for (uint32_t i = 0; i < levels; i++)
    {
        HiZPushConstants pc = { m_viewport.UpperLeftCorner.X,
            m_viewport.UpperLeftCorner.Y, (int32_t)i, 0 };
        memcpy(&pc_data[i * PUSH_CONSTANTS_SLOT], &pc, sizeof(pc));
    }
    wgpu::BufferDescriptor buf_desc;
    buf_desc.label = "hiz depth constants";
    buf_desc.size = pc_data.size();
    buf_desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    m_push_constants = device.CreateBuffer(&buf_desc);
    driver->getQueue().WriteBuffer(m_push_constants, 0, pc_data.data(),
        pc_data.size());

    const wgpu::Sampler& nearest = driver->getSampler(GVS_NEAREST);
    m_level_bind_groups.clear();
    m_push_constants_bind_groups.clear();
    for (uint32_t i = 0; i < levels; i++)
    {
        // Level 0 reads the depth buffer, others the levels before them
        wgpu::TextureView input = m_dfbo->getDepthView();
        if (i > 0)
        {
            wgpu::TextureViewDescriptor view_desc;
            view_desc.baseMipLevel = 0;
            view_desc.mipLevelCount = i;
            input = m_hiz_depth.CreateView(&view_desc);
        }
        wgpu::TextureViewDescriptor output_desc;
        output_desc.baseMipLevel = i;
        output_desc.mipLevelCount = 1;
        std::array<wgpu::BindGroupEntry, 3> image_entries = {};
        image_entries[0].binding = 0;
        image_entries[0].textureView = input;
        image_entries[1].binding =
            GEWGPUShaderManager::getSamplerBindingOffset();
        image_entries[1].sampler = nearest;
        image_entries[2].binding = 1;
        image_entries[2].textureView = m_hiz_depth.CreateView(&output_desc);
        wgpu::BindGroupDescriptor bg_desc;
        bg_desc.layout = image_layout;
        bg_desc.entryCount = image_entries.size();
        bg_desc.entries = image_entries.data();
        m_level_bind_groups.push_back(device.CreateBindGroup(&bg_desc));

        wgpu::BindGroupEntry pc_bg_entry;
        pc_bg_entry.binding = 4;
        pc_bg_entry.buffer = m_push_constants;
        pc_bg_entry.offset = i * PUSH_CONSTANTS_SLOT;
        pc_bg_entry.size = sizeof(HiZPushConstants);
        bg_desc.layout = pc_layout;
        bg_desc.entryCount = 1;
        bg_desc.entries = &pc_bg_entry;
        m_push_constants_bind_groups.push_back(device.CreateBindGroup(&bg_desc));
    }

    // Displace color, depth (compared) and the whole pyramid
    std::array<wgpu::BindGroupEntry, 6> entries_rendering = {};
    const uint32_t offset = GEWGPUShaderManager::getSamplerBindingOffset();
    entries_rendering[0].binding = 0;
    entries_rendering[0].textureView = m_dfbo->getDisplaceColorView();
    entries_rendering[1].binding = offset;
    entries_rendering[1].sampler = nearest;
    entries_rendering[2].binding = 1;
    entries_rendering[2].textureView = m_dfbo->getDepthView();
    entries_rendering[3].binding = 1 + offset;
    entries_rendering[3].sampler = driver->getSampler(GVS_SHADOW);
    entries_rendering[4].binding = 2;
    entries_rendering[4].textureView = m_hiz_depth.CreateView();
    entries_rendering[5].binding = 2 + offset;
    entries_rendering[5].sampler = nearest;
    wgpu::BindGroupDescriptor bg_desc;
    bg_desc.label = "displace mask hiz";
    bg_desc.layout = m_dfbo->getDisplaceMaskLayout();
    bg_desc.entryCount = entries_rendering.size();
    bg_desc.entries = entries_rendering.data();
    m_rendering_bind_group = device.CreateBindGroup(&bg_desc);
}   // init

// ----------------------------------------------------------------------------
void GEWGPUHiZDepth::generate(wgpu::CommandEncoder& encoder)
{
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(m_pipeline);
    const uint32_t width = m_hiz_depth.GetWidth();
    const uint32_t height = m_hiz_depth.GetHeight();
    for (uint32_t i = 0; i < m_level_bind_groups.size(); i++)
    {
        pass.SetBindGroup(0, m_level_bind_groups[i]);
        pass.SetBindGroup(1, m_push_constants_bind_groups[i]);
        const uint32_t w = std::max(width >> i, 1u);
        const uint32_t h = std::max(height >> i, 1u);
        pass.DispatchWorkgroups((w + 15) / 16, (h + 15) / 16);
    }
    pass.End();
}   // generate

}
