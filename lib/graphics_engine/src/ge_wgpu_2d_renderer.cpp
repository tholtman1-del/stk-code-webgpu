#include "ge_wgpu_2d_renderer.hpp"

#include "ge_wgpu_driver.hpp"
#include "ge_wgpu_shader_manager.hpp"
#include "ge_wgpu_texture.hpp"

#include "rect.h"
#include "vector2d.h"
#include "SColor.h"

#include <cstddef>
#include <map>
#include <unordered_map>
#include <vector>

namespace GE
{
// ============================================================================
namespace GEWGPU2dRenderer
{
using namespace irr;
// ============================================================================
GEWGPUDriver* g_driver = NULL;
wgpu::BindGroupLayout g_bind_group_layout;
wgpu::PipelineLayout g_pipeline_layout;
std::map<wgpu::TextureFormat, wgpu::RenderPipeline> g_pipelines;
// All WriteBuffer calls of a frame land before its single submit, so every
// render() call within a frame gets its own buffers
struct BufferSlot
{
    wgpu::Buffer m_vertex, m_index;
    uint64_t m_vertex_size = 0, m_index_size = 0;
};
std::vector<BufferSlot> g_slots;
unsigned g_slot_idx = 0;

struct BindGroupCache
{
    WGPUTextureView m_view;
    wgpu::BindGroup m_bind_group;
};
std::unordered_map<const GEWGPUTexture*, BindGroupCache> g_bind_groups;

// Matches the vertex input of 2d_render.vert
struct Tri
{
    core::vector2df pos;
    video::SColor color;
    core::vector2df uv;
    int sampler_idx;
};
static_assert(sizeof(Tri) == 24, "2D vertex layout mismatch");

std::vector<Tri> g_tris_queue;
std::vector<uint16_t> g_tris_index_queue;
// Per vertex, to split batches
std::vector<core::recti> g_tris_clip;
std::vector<const GEWGPUTexture*> g_tris_texture;

// ----------------------------------------------------------------------------
wgpu::RenderPipeline getPipeline(wgpu::TextureFormat format)
{
    auto it = g_pipelines.find(format);
    if (it != g_pipelines.end())
        return it->second;

    std::array<wgpu::VertexAttribute, 4> attributes = {};
    attributes[0].format = wgpu::VertexFormat::Float32x2;
    attributes[0].offset = offsetof(Tri, pos);
    attributes[0].shaderLocation = 0;
    attributes[1].format = wgpu::VertexFormat::Unorm8x4;
    attributes[1].offset = offsetof(Tri, color);
    attributes[1].shaderLocation = 1;
    attributes[2].format = wgpu::VertexFormat::Float32x2;
    attributes[2].offset = offsetof(Tri, uv);
    attributes[2].shaderLocation = 2;
    attributes[3].format = wgpu::VertexFormat::Sint32;
    attributes[3].offset = offsetof(Tri, sampler_idx);
    attributes[3].shaderLocation = 3;

    wgpu::VertexBufferLayout vertex_layout;
    vertex_layout.arrayStride = sizeof(Tri);
    vertex_layout.stepMode = wgpu::VertexStepMode::Vertex;
    vertex_layout.attributeCount = attributes.size();
    vertex_layout.attributes = attributes.data();

    wgpu::BlendState blend;
    blend.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
    blend.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    blend.color.operation = wgpu::BlendOperation::Add;
    blend.alpha = blend.color;

    wgpu::ColorTargetState target;
    target.format = format;
    target.blend = &blend;

    wgpu::FragmentState fragment;
    fragment.module = GEWGPUShaderManager::getShader("2d_render.frag");
    fragment.entryPoint = "main";
    fragment.targetCount = 1;
    fragment.targets = &target;

    wgpu::RenderPipelineDescriptor desc;
    desc.label = "2d_render";
    desc.layout = g_pipeline_layout;
    desc.vertex.module = GEWGPUShaderManager::getShader("2d_render.vert");
    desc.vertex.entryPoint = "main";
    desc.vertex.bufferCount = 1;
    desc.vertex.buffers = &vertex_layout;
    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    // GUI quads come in both windings
    desc.primitive.cullMode = wgpu::CullMode::None;
    desc.fragment = &fragment;

    wgpu::RenderPipeline pipeline =
        g_driver->getDevice().CreateRenderPipeline(&desc);
    g_pipelines[format] = pipeline;
    return pipeline;
}   // getPipeline

// ----------------------------------------------------------------------------
const wgpu::BindGroup& getBindGroup(const GEWGPUTexture* texture)
{
    const wgpu::TextureView& view = texture->getView();
    BindGroupCache& cache = g_bind_groups[texture];
    if (cache.m_view == view.Get() && cache.m_bind_group)
        return cache.m_bind_group;

    std::array<wgpu::BindGroupEntry, 2> entries = {};
    entries[0].binding = 0;
    entries[0].textureView = view;
    entries[1].binding = GEWGPUShaderManager::getSamplerBindingOffset();
    entries[1].sampler = g_driver->getSampler(GVS_2D_RENDER);
    wgpu::BindGroupDescriptor desc;
    desc.layout = g_bind_group_layout;
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    cache.m_view = view.Get();
    cache.m_bind_group = g_driver->getDevice().CreateBindGroup(&desc);
    return cache.m_bind_group;
}   // getBindGroup

// ----------------------------------------------------------------------------
void ensureBuffer(wgpu::Buffer& buffer, uint64_t& current_size,
                  uint64_t needed, wgpu::BufferUsage usage, const char* label)
{
    if (buffer && current_size >= needed)
        return;
    uint64_t size = std::max<uint64_t>(needed, 64 * 1024);
    // Grow geometrically so busy screens settle on one allocation
    size = std::max(size, current_size * 2);
    wgpu::BufferDescriptor desc;
    desc.label = label;
    desc.size = (size + 3) & ~3ull;
    desc.usage = usage | wgpu::BufferUsage::CopyDst;
    if (buffer)
        buffer.Destroy();
    buffer = g_driver->getDevice().CreateBuffer(&desc);
    current_size = desc.size;
}   // ensureBuffer

}   // GEWGPU2dRenderer

// ============================================================================
void GEWGPU2dRenderer::init(GEWGPUDriver* driver)
{
    g_driver = driver;

    std::array<wgpu::BindGroupLayoutEntry, 2> entries = {};
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Fragment;
    entries[0].texture.sampleType = wgpu::TextureSampleType::Float;
    entries[0].texture.viewDimension = wgpu::TextureViewDimension::e2D;
    entries[1].binding = GEWGPUShaderManager::getSamplerBindingOffset();
    entries[1].visibility = wgpu::ShaderStage::Fragment;
    entries[1].sampler.type = wgpu::SamplerBindingType::Filtering;
    wgpu::BindGroupLayoutDescriptor bgl_desc;
    bgl_desc.entryCount = entries.size();
    bgl_desc.entries = entries.data();
    g_bind_group_layout = driver->getDevice().CreateBindGroupLayout(&bgl_desc);

    wgpu::PipelineLayoutDescriptor pl_desc;
    pl_desc.bindGroupLayoutCount = 1;
    pl_desc.bindGroupLayouts = &g_bind_group_layout;
    g_pipeline_layout = driver->getDevice().CreatePipelineLayout(&pl_desc);
}   // init

// ----------------------------------------------------------------------------
void GEWGPU2dRenderer::destroy()
{
    clear();
    g_bind_groups.clear();
    g_pipelines.clear();
    g_pipeline_layout = nullptr;
    g_bind_group_layout = nullptr;
    g_slots.clear();
    g_slot_idx = 0;
    g_driver = NULL;
}   // destroy

// ----------------------------------------------------------------------------
bool GEWGPU2dRenderer::empty()
{
    return g_tris_index_queue.empty();
}   // empty

// ----------------------------------------------------------------------------
void GEWGPU2dRenderer::onTextureDestroyed(const GEWGPUTexture* t)
{
    g_bind_groups.erase(t);
    // A texture may be dropped after being queued in the same frame
    for (const GEWGPUTexture*& queued : g_tris_texture)
    {
        if (queued == t)
        {
            queued = static_cast<const GEWGPUTexture*>(
                g_driver->getTransparentTexture());
        }
    }
}   // onTextureDestroyed

// ----------------------------------------------------------------------------
void GEWGPU2dRenderer::render(wgpu::RenderPassEncoder& pass,
                              wgpu::TextureFormat format,
                              const core::dimension2du& target_size)
{
    if (g_tris_index_queue.empty())
        return;

    // WriteBuffer sizes must be a multiple of 4 bytes
    if (g_tris_index_queue.size() % 2 != 0)
        g_tris_index_queue.push_back(0);
    const uint64_t vertex_size = g_tris_queue.size() * sizeof(Tri);
    const uint64_t index_size = g_tris_index_queue.size() * sizeof(uint16_t);
    if (g_slot_idx >= g_slots.size())
        g_slots.resize(g_slot_idx + 1);
    BufferSlot& slot = g_slots[g_slot_idx++];
    ensureBuffer(slot.m_vertex, slot.m_vertex_size, vertex_size,
        wgpu::BufferUsage::Vertex, "2d vertices");
    ensureBuffer(slot.m_index, slot.m_index_size, index_size,
        wgpu::BufferUsage::Index, "2d indices");
    const wgpu::Queue& queue = g_driver->getQueue();
    queue.WriteBuffer(slot.m_vertex, 0, g_tris_queue.data(), vertex_size);
    queue.WriteBuffer(slot.m_index, 0, g_tris_index_queue.data(), index_size);

    pass.SetPipeline(getPipeline(format));
    pass.SetVertexBuffer(0, slot.m_vertex, 0, vertex_size);
    pass.SetIndexBuffer(slot.m_index, wgpu::IndexFormat::Uint16, 0,
        index_size);

    const core::recti full(0, 0, target_size.Width, target_size.Height);
    auto flush = [&](unsigned first, unsigned count,
                     const GEWGPUTexture* texture, core::recti clip)
    {
        clip.clipAgainst(full);
        if (count == 0 || clip.getWidth() <= 0 || clip.getHeight() <= 0)
            return;
        pass.SetBindGroup(0, getBindGroup(texture));
        pass.SetScissorRect(clip.UpperLeftCorner.X, clip.UpperLeftCorner.Y,
            clip.getWidth(), clip.getHeight());
        pass.DrawIndexed(count, 1, first, 0, 0);
    };

    // Indices are always whole triangles (the padding index is never read)
    const unsigned total = (g_tris_index_queue.size() / 3) * 3;
    unsigned batch_start = 0;
    const GEWGPUTexture* texture = g_tris_texture[g_tris_index_queue[0]];
    core::recti clip = g_tris_clip[g_tris_index_queue[0]];
    for (unsigned idx = 0; idx < total; idx += 3)
    {
        uint16_t v = g_tris_index_queue[idx];
        if (g_tris_texture[v] != texture || g_tris_clip[v] != clip)
        {
            flush(batch_start, idx - batch_start, texture, clip);
            batch_start = idx;
            texture = g_tris_texture[v];
            clip = g_tris_clip[v];
        }
    }
    flush(batch_start, total - batch_start, texture, clip);
    clear();
}   // render

// ----------------------------------------------------------------------------
void GEWGPU2dRenderer::endFrame()
{
    g_slot_idx = 0;
}   // endFrame

// ----------------------------------------------------------------------------
void GEWGPU2dRenderer::clear()
{
    g_tris_queue.clear();
    g_tris_index_queue.clear();
    g_tris_clip.clear();
    g_tris_texture.clear();
}   // clear

// ----------------------------------------------------------------------------
void GEWGPU2dRenderer::addVerticesIndices(irr::video::S3DVertex* vertices,
                                          unsigned vertices_count,
                                          uint16_t* indices,
                                          unsigned indices_count,
                                          const irr::video::ITexture* t)
{
    const GEWGPUTexture* texture = dynamic_cast<const GEWGPUTexture*>(t);
    if (!texture)
        return;
    uint16_t last_index = (uint16_t)g_tris_queue.size();
    if (last_index + vertices_count > 65535)
        return;

    const core::dimension2du& rt_size = g_driver->getCurrentRenderTargetSize();
    const core::recti& clip = g_driver->getCurrentClip();
    for (unsigned idx = 0; idx < vertices_count; idx++)
    {
        Tri tri;
        const video::S3DVertex& vertex = vertices[idx];
        tri.pos = core::vector2df(vertex.Pos.X / rt_size.Width,
            vertex.Pos.Y / rt_size.Height) * 2.0f;
        tri.pos -= 1.0f;
        tri.color = vertex.Color;
        tri.uv = vertex.TCoords;
        tri.sampler_idx = 0;
        g_tris_queue.push_back(tri);
        g_tris_clip.push_back(clip);
        g_tris_texture.push_back(texture);
    }
    for (unsigned idx = 0; idx < indices_count * 3; idx++)
        g_tris_index_queue.push_back(last_index + indices[idx]);
}   // addVerticesIndices

}
