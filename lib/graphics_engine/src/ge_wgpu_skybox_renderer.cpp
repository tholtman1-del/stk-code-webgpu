#include "ge_wgpu_skybox_renderer.hpp"

#include "ge_main.hpp"
#include "ge_mipmap_generator.hpp"
#include "ge_texture.hpp"
#include "ge_wgpu_driver.hpp"
#include "ge_wgpu_shader_manager.hpp"

#include "IImage.h"
#include "ISceneNode.h"
#include "ITexture.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace GE
{
namespace
{
// Bindings of f_skybox_texture and f_skybox_texture_srgb in skybox.frag
const uint32_t SKYBOX_BINDING = 2;
const uint32_t SKYBOX_SRGB_BINDING = 3;
// Sizes and sample count of GEVulkanEnvironmentMap
const unsigned DIFFUSE_ENV_SIZE = 32;
const unsigned SPECULAR_ENV_SIZE = 256;
const int DIFFUSE_SAMPLE_COUNT = 256;
// Push constants of the compute shaders, at group 1 binding 4
struct EnvPushConstants
{
    int32_t m_size;
    int32_t m_sample_count;
    int32_t m_level;
    int32_t m_total_mipmaps;
};
const uint32_t PUSH_CONSTANTS_SLOT = 256;

// ----------------------------------------------------------------------------
unsigned mipLevels(unsigned size)
{
    return (unsigned)std::floor(std::log2(size)) + 1;
}   // mipLevels

// ----------------------------------------------------------------------------
/** Same rotation of the top and bottom faces as GEVulkanSkyBoxRenderer. */
void rotateFace(video::IImage* img)
{
    const unsigned width = img->getDimension().Width;
    std::vector<uint32_t> tmp(width * width);
    const uint32_t* data = (const uint32_t*)img->lock();
    for (unsigned i = 0; i < width; i++)
    {
        for (unsigned j = 0; j < width; j++)
            tmp[j * width + i] = data[i * width + (width - j - 1)];
    }
    memcpy(img->lock(), tmp.data(), tmp.size() * sizeof(uint32_t));
    img->unlock();
}   // rotateFace

}   // anonymous namespace

// ----------------------------------------------------------------------------
GEWGPUSkyBoxRenderer::GEWGPUSkyBoxRenderer()
                    : m_skybox(NULL), m_skytop_color(0)
{
    std::array<wgpu::BindGroupLayoutEntry, 4> entries = {};
    const uint32_t bindings[2] = { SKYBOX_BINDING, SKYBOX_SRGB_BINDING };
    for (unsigned i = 0; i < 2; i++)
    {
        entries[i * 2].binding = bindings[i];
        entries[i * 2].visibility = wgpu::ShaderStage::Fragment;
        entries[i * 2].texture.sampleType = wgpu::TextureSampleType::Float;
        entries[i * 2].texture.viewDimension =
            wgpu::TextureViewDimension::Cube;
        entries[i * 2 + 1].binding =
            bindings[i] + GEWGPUShaderManager::getSamplerBindingOffset();
        entries[i * 2 + 1].visibility = wgpu::ShaderStage::Fragment;
        entries[i * 2 + 1].sampler.type = wgpu::SamplerBindingType::Filtering;
    }
    wgpu::BindGroupLayoutDescriptor desc;
    desc.label = "skybox";
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    m_layout = getWGPUDriver()->getDevice().CreateBindGroupLayout(&desc);
}   // GEWGPUSkyBoxRenderer

// ----------------------------------------------------------------------------
void GEWGPUSkyBoxRenderer::addSkyBox(irr::scene::ISceneNode* skybox)
{
    if (skybox->getType() != irr::scene::ESNT_SKY_BOX || m_skybox == skybox)
        return;
    m_skybox = skybox;
    m_bind_group = nullptr;
    m_env_bind_group = nullptr;
    m_diffuse_env = m_specular_env = nullptr;

    // Cube face order of GEVulkanSkyBoxRenderer
    const std::array<int, 6> order = {{ 1, 3, 4, 5, 2, 0 }};
    std::array<video::ITexture*, 6> faces;
    unsigned width = 0;
    for (unsigned i = 0; i < 6; i++)
    {
        faces[i] = skybox->getMaterial(order[i]).getTexture(0);
        if (!faces[i])
            return;
        const core::dimension2du& dim = faces[i]->getOriginalSize();
        width = std::max({ width, dim.Width, dim.Height });
    }
    GEWGPUDriver* driver = getWGPUDriver();
    width = std::min(width, driver->getMaxTextureSize().Width);
    if (width == 0)
        return;

    wgpu::TextureDescriptor tex_desc;
    tex_desc.label = "skybox";
    tex_desc.size = { width, width, 6 };
    // Mipmaps are sampled by specular_prefilter.comp
    tex_desc.mipLevelCount = mipLevels(width);
    // IImage A8R8G8B8 is BGRA in memory
    tex_desc.format = wgpu::TextureFormat::BGRA8Unorm;
    tex_desc.usage = wgpu::TextureUsage::TextureBinding |
        wgpu::TextureUsage::CopyDst;
    // The environment maps are made from linear colors
    wgpu::TextureFormat srgb = wgpu::TextureFormat::BGRA8UnormSrgb;
    tex_desc.viewFormatCount = 1;
    tex_desc.viewFormats = &srgb;
    m_cubemap = driver->getDevice().CreateTexture(&tex_desc);

    const core::dimension2du size(width, width);
    for (unsigned i = 0; i < 6; i++)
    {
        core::dimension2du target = size;
        video::IImage* img = getResizedImageFullPath(faces[i]->getFullPath(),
            size, NULL, &target);
        if (!img)
        {
            m_skybox = NULL;
            return;
        }
        if (i == 2)
        {
            video::IImage* pixel = driver->createImage(video::ECF_A8R8G8B8,
                core::dimension2du(1, 1));
            img->copyToScaling(pixel);
            m_skytop_color.color = *(uint32_t*)pixel->lock();
            pixel->drop();
        }
        if (i == 2 || i == 3)
            rotateFace(img);
        GEMipmapGenerator generator((uint8_t*)img->lock(), 4, size,
            false/*normal_map*/);
        std::vector<GEImageLevel>& levels = generator.getAllLevels();
        for (unsigned level = 0; level < levels.size(); level++)
        {
            const GEImageLevel& l = levels[level];
            wgpu::TexelCopyTextureInfo dst;
            dst.texture = m_cubemap;
            dst.mipLevel = level;
            dst.origin = { 0, 0, i };
            wgpu::TexelCopyBufferLayout layout;
            layout.bytesPerRow = l.m_dim.Width * 4;
            layout.rowsPerImage = l.m_dim.Height;
            wgpu::Extent3D extent = { l.m_dim.Width, l.m_dim.Height, 1 };
            driver->getQueue().WriteTexture(&dst, l.m_data, l.m_size,
                &layout, &extent);
        }
        img->unlock();
        img->drop();
    }

    wgpu::TextureViewDescriptor view_desc;
    view_desc.dimension = wgpu::TextureViewDimension::Cube;
    wgpu::TextureView views[2];
    views[0] = m_cubemap.CreateView(&view_desc);
    // Sampled when deferred (linear HDR)
    view_desc.format = wgpu::TextureFormat::BGRA8UnormSrgb;
    views[1] = m_cubemap.CreateView(&view_desc);
    std::array<wgpu::BindGroupEntry, 4> entries = {};
    const uint32_t bindings[2] = { SKYBOX_BINDING, SKYBOX_SRGB_BINDING };
    for (unsigned i = 0; i < 2; i++)
    {
        entries[i * 2].binding = bindings[i];
        entries[i * 2].textureView = views[i];
        entries[i * 2 + 1].binding =
            bindings[i] + GEWGPUShaderManager::getSamplerBindingOffset();
        entries[i * 2 + 1].sampler = driver->getSampler(GVS_SKYBOX);
    }
    wgpu::BindGroupDescriptor desc;
    desc.label = "skybox";
    desc.layout = m_layout;
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    m_bind_group = driver->getDevice().CreateBindGroup(&desc);

    if (getGEConfig()->m_pbr && getGEConfig()->m_ibl)
        generateEnvironmentMaps();
}   // addSkyBox

// ----------------------------------------------------------------------------
void GEWGPUSkyBoxRenderer::generateEnvironmentMaps()
{
    GEWGPUDriver* driver = getWGPUDriver();
    const wgpu::Device& device = driver->getDevice();

    std::array<wgpu::BindGroupLayoutEntry, 3> entries = {};
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Compute;
    entries[0].texture.sampleType = wgpu::TextureSampleType::Float;
    entries[0].texture.viewDimension = wgpu::TextureViewDimension::Cube;
    entries[1].binding = GEWGPUShaderManager::getSamplerBindingOffset();
    entries[1].visibility = wgpu::ShaderStage::Compute;
    entries[1].sampler.type = wgpu::SamplerBindingType::Filtering;
    entries[2].binding = 1;
    entries[2].visibility = wgpu::ShaderStage::Compute;
    entries[2].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
    entries[2].storageTexture.format = wgpu::TextureFormat::RGBA8Unorm;
    entries[2].storageTexture.viewDimension =
        wgpu::TextureViewDimension::e2DArray;
    wgpu::BindGroupLayoutDescriptor bgl_desc;
    bgl_desc.entryCount = entries.size();
    bgl_desc.entries = entries.data();
    wgpu::BindGroupLayout image_layout = device.CreateBindGroupLayout(&bgl_desc);

    wgpu::BindGroupLayoutEntry pc_entry;
    pc_entry.binding = 4;
    pc_entry.visibility = wgpu::ShaderStage::Compute;
    pc_entry.buffer.type = wgpu::BufferBindingType::Uniform;
    pc_entry.buffer.hasDynamicOffset = true;
    pc_entry.buffer.minBindingSize = sizeof(EnvPushConstants);
    bgl_desc.entryCount = 1;
    bgl_desc.entries = &pc_entry;
    wgpu::BindGroupLayout pc_layout = device.CreateBindGroupLayout(&bgl_desc);

    std::array<wgpu::BindGroupLayout, 2> layouts = {{ image_layout, pc_layout }};
    wgpu::PipelineLayoutDescriptor pl_desc;
    pl_desc.bindGroupLayoutCount = layouts.size();
    pl_desc.bindGroupLayouts = layouts.data();
    wgpu::PipelineLayout pipeline_layout = device.CreatePipelineLayout(&pl_desc);

    auto create_env = [&](unsigned size, const char* label)
    {
        wgpu::TextureDescriptor desc;
        desc.label = label;
        desc.size = { size, size, 6 };
        desc.mipLevelCount = mipLevels(size);
        desc.format = wgpu::TextureFormat::RGBA8Unorm;
        desc.usage = wgpu::TextureUsage::TextureBinding |
            wgpu::TextureUsage::StorageBinding;
        return device.CreateTexture(&desc);
    };
    m_diffuse_env = create_env(DIFFUSE_ENV_SIZE, "diffuse environment map");
    m_specular_env = create_env(SPECULAR_ENV_SIZE, "specular environment map");

    // One push constants slot per mipmap level of both maps
    struct Job
    {
        wgpu::Texture m_texture;
        bool m_specular;
        unsigned m_level;
        EnvPushConstants m_pc;
    };
    std::vector<Job> jobs;
    auto add_jobs = [&](const wgpu::Texture& t, unsigned size, bool specular)
    {
        const unsigned count = mipLevels(size);
        for (unsigned level = 0; level < count; level++)
        {
            Job job;
            job.m_texture = t;
            job.m_specular = specular;
            job.m_level = level;
            job.m_pc.m_size = std::max(size >> level, 1u);
            if (specular)
            {
                // Same as GEVulkanEnvironmentMap: next power of 2 of half
                // the size, minimum 16
                unsigned v = std::max(job.m_pc.m_size / 2, 1);
                unsigned p = 1;
                while (p < v)
                    p *= 2;
                job.m_pc.m_sample_count = std::max(16u, p);
            }
            else
                job.m_pc.m_sample_count = DIFFUSE_SAMPLE_COUNT;
            job.m_pc.m_level = level;
            job.m_pc.m_total_mipmaps = count;
            jobs.push_back(job);
        }
    };
    add_jobs(m_diffuse_env, DIFFUSE_ENV_SIZE, false);
    add_jobs(m_specular_env, SPECULAR_ENV_SIZE, true);

    std::vector<uint8_t> pc_data(jobs.size() * PUSH_CONSTANTS_SLOT, 0);
    for (unsigned i = 0; i < jobs.size(); i++)
    {
        memcpy(&pc_data[i * PUSH_CONSTANTS_SLOT], &jobs[i].m_pc,
            sizeof(EnvPushConstants));
    }
    wgpu::BufferDescriptor buf_desc;
    buf_desc.label = "environment map constants";
    buf_desc.size = pc_data.size();
    buf_desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer pc_buffer = device.CreateBuffer(&buf_desc);
    driver->getQueue().WriteBuffer(pc_buffer, 0, pc_data.data(),
        pc_data.size());
    wgpu::BindGroupEntry pc_bg_entry;
    pc_bg_entry.binding = 4;
    pc_bg_entry.buffer = pc_buffer;
    pc_bg_entry.size = sizeof(EnvPushConstants);
    wgpu::BindGroupDescriptor bg_desc;
    bg_desc.layout = pc_layout;
    bg_desc.entryCount = 1;
    bg_desc.entries = &pc_bg_entry;
    wgpu::BindGroup pc_bind_group = device.CreateBindGroup(&bg_desc);

    wgpu::TextureViewDescriptor srgb_desc;
    srgb_desc.dimension = wgpu::TextureViewDimension::Cube;
    srgb_desc.format = wgpu::TextureFormat::BGRA8UnormSrgb;
    wgpu::TextureView skybox_srgb = m_cubemap.CreateView(&srgb_desc);

    wgpu::ComputePipeline pipelines[2];
    const char* shaders[2] = { "diffuse_irradiance.comp",
        "specular_prefilter.comp" };
    for (unsigned i = 0; i < 2; i++)
    {
        wgpu::ComputePipelineDescriptor desc;
        desc.label = shaders[i];
        desc.layout = pipeline_layout;
        desc.compute.module = GEWGPUShaderManager::getShader(shaders[i]);
        desc.compute.entryPoint = "main";
        pipelines[i] = device.CreateComputePipeline(&desc);
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    for (unsigned i = 0; i < jobs.size(); i++)
    {
        const Job& job = jobs[i];
        wgpu::TextureViewDescriptor view_desc;
        view_desc.dimension = wgpu::TextureViewDimension::e2DArray;
        view_desc.baseMipLevel = job.m_level;
        view_desc.mipLevelCount = 1;
        std::array<wgpu::BindGroupEntry, 3> image_entries = {};
        image_entries[0].binding = 0;
        image_entries[0].textureView = skybox_srgb;
        image_entries[1].binding =
            GEWGPUShaderManager::getSamplerBindingOffset();
        image_entries[1].sampler = driver->getSampler(GVS_SKYBOX);
        image_entries[2].binding = 1;
        image_entries[2].textureView = job.m_texture.CreateView(&view_desc);
        bg_desc.layout = image_layout;
        bg_desc.entryCount = image_entries.size();
        bg_desc.entries = image_entries.data();
        pass.SetPipeline(pipelines[job.m_specular ? 1 : 0]);
        pass.SetBindGroup(0, device.CreateBindGroup(&bg_desc));
        uint32_t offset = i * PUSH_CONSTANTS_SLOT;
        pass.SetBindGroup(1, pc_bind_group, 1, &offset);
        // 16x16 work groups, one layer per cube face
        const uint32_t groups = (job.m_pc.m_size + 15) / 16;
        pass.DispatchWorkgroups(groups, groups, 6);
    }
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    driver->getQueue().Submit(1, &commands);
}   // generateEnvironmentMaps

// ----------------------------------------------------------------------------
const wgpu::BindGroup& GEWGPUSkyBoxRenderer::getEnvBindGroup(
                                         const wgpu::BindGroupLayout& layout,
                                         const wgpu::TextureView& dummy)
{
    if (m_env_bind_group || !m_bind_group)
        return m_env_bind_group;
    wgpu::TextureViewDescriptor view_desc;
    view_desc.dimension = wgpu::TextureViewDimension::Cube;
    std::array<wgpu::TextureView, 4> views;
    views[0] = m_diffuse_env ? m_diffuse_env.CreateView(&view_desc) : dummy;
    views[1] = m_specular_env ? m_specular_env.CreateView(&view_desc) : dummy;
    views[2] = m_cubemap.CreateView(&view_desc);
    view_desc.format = wgpu::TextureFormat::BGRA8UnormSrgb;
    views[3] = m_cubemap.CreateView(&view_desc);
    std::array<wgpu::BindGroupEntry, 8> entries = {};
    for (unsigned i = 0; i < 4; i++)
    {
        entries[i * 2].binding = i;
        entries[i * 2].textureView = views[i];
        entries[i * 2 + 1].binding =
            i + GEWGPUShaderManager::getSamplerBindingOffset();
        entries[i * 2 + 1].sampler = getWGPUDriver()->getSampler(GVS_SKYBOX);
    }
    wgpu::BindGroupDescriptor desc;
    desc.label = "environment maps";
    desc.layout = layout;
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    m_env_bind_group = getWGPUDriver()->getDevice().CreateBindGroup(&desc);
    return m_env_bind_group;
}   // getEnvBindGroup

// ----------------------------------------------------------------------------
float GEWGPUSkyBoxRenderer::getSpecularLevelsMinusOne()
{
    return std::floor(std::log2((float)SPECULAR_ENV_SIZE));
}   // getSpecularLevelsMinusOne

// ----------------------------------------------------------------------------
void GEWGPUSkyBoxRenderer::render(wgpu::RenderPassEncoder& pass,
                                  wgpu::TextureFormat color_format,
                                  const wgpu::BindGroup& data,
                                  const wgpu::BindGroupLayout& data_layout,
                                  bool deferred)
{
    if (!m_skybox || !m_bind_group)
        return;
    GEWGPUDriver* driver = getWGPUDriver();
    if (!m_pipeline_layout)
    {
        std::array<wgpu::BindGroupLayout, 2> layouts =
            {{ m_layout, data_layout }};
        wgpu::PipelineLayoutDescriptor pl_desc;
        pl_desc.bindGroupLayoutCount = layouts.size();
        pl_desc.bindGroupLayouts = layouts.data();
        m_pipeline_layout =
            driver->getDevice().CreatePipelineLayout(&pl_desc);
    }
    wgpu::RenderPipeline& pipeline = m_pipelines[{ color_format, deferred }];
    if (!pipeline)
    {
        wgpu::ColorTargetState target;
        target.format = color_format;
        wgpu::FragmentState fragment;
        fragment.module = GEWGPUShaderManager::getShader("skybox.frag");
        fragment.entryPoint = "main";
        fragment.targetCount = 1;
        fragment.targets = &target;
        GEWGPUShaderManager::Constants c;
        c.m_deferred = deferred;
        std::vector<wgpu::ConstantEntry> constants =
            GEWGPUShaderManager::getConstants("skybox.frag", c);
        fragment.constantCount = constants.size();
        fragment.constants = constants.data();
        // fullscreen_quad.vert is at depth 1, the clear value: only pixels
        // without geometry pass
        wgpu::DepthStencilState depth;
        depth.format = wgpu::TextureFormat::Depth32Float;
        depth.depthWriteEnabled = wgpu::OptionalBool::False;
        depth.depthCompare = wgpu::CompareFunction::Equal;
        wgpu::RenderPipelineDescriptor desc;
        desc.label = "skybox";
        desc.layout = m_pipeline_layout;
        desc.vertex.module =
            GEWGPUShaderManager::getShader("fullscreen_quad.vert");
        desc.vertex.entryPoint = "main";
        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        desc.depthStencil = &depth;
        desc.fragment = &fragment;
        pipeline = driver->getDevice().CreateRenderPipeline(&desc);
    }
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, m_bind_group);
    uint32_t offset = 0;
    pass.SetBindGroup(1, data, 1, &offset);
    pass.Draw(3);
}   // render

}
