#include "ge_wgpu_skybox_renderer.hpp"

#include "ge_main.hpp"
#include "ge_texture.hpp"
#include "ge_wgpu_driver.hpp"
#include "ge_wgpu_shader_manager.hpp"

#include "IImage.h"
#include "ISceneNode.h"
#include "ITexture.h"

#include <algorithm>
#include <array>
#include <vector>

namespace GE
{
namespace
{
// Bindings of f_skybox_texture and f_skybox_texture_srgb in skybox.frag
const uint32_t SKYBOX_BINDING = 2;
const uint32_t SKYBOX_SRGB_BINDING = 3;

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
    // IImage A8R8G8B8 is BGRA in memory
    tex_desc.format = wgpu::TextureFormat::BGRA8Unorm;
    tex_desc.usage = wgpu::TextureUsage::TextureBinding |
        wgpu::TextureUsage::CopyDst;
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
        wgpu::TexelCopyTextureInfo dst;
        dst.texture = m_cubemap;
        dst.origin = { 0, 0, i };
        wgpu::TexelCopyBufferLayout layout;
        layout.bytesPerRow = width * 4;
        layout.rowsPerImage = width;
        wgpu::Extent3D extent = { width, width, 1 };
        driver->getQueue().WriteTexture(&dst, img->lock(), width * width * 4,
            &layout, &extent);
        img->unlock();
        img->drop();
    }

    wgpu::TextureViewDescriptor view_desc;
    view_desc.dimension = wgpu::TextureViewDimension::Cube;
    wgpu::TextureView view = m_cubemap.CreateView(&view_desc);
    std::array<wgpu::BindGroupEntry, 4> entries = {};
    const uint32_t bindings[2] = { SKYBOX_BINDING, SKYBOX_SRGB_BINDING };
    for (unsigned i = 0; i < 2; i++)
    {
        entries[i * 2].binding = bindings[i];
        entries[i * 2].textureView = view;
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
}   // addSkyBox

// ----------------------------------------------------------------------------
void GEWGPUSkyBoxRenderer::render(wgpu::RenderPassEncoder& pass,
                                  wgpu::TextureFormat color_format,
                                  const wgpu::BindGroup& data,
                                  const wgpu::BindGroupLayout& data_layout)
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
    wgpu::RenderPipeline& pipeline = m_pipelines[color_format];
    if (!pipeline)
    {
        wgpu::ColorTargetState target;
        target.format = color_format;
        wgpu::FragmentState fragment;
        fragment.module = GEWGPUShaderManager::getShader("skybox.frag");
        fragment.entryPoint = "main";
        fragment.targetCount = 1;
        fragment.targets = &target;
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
