#include "ge_wgpu_mesh_cache.hpp"

#include "ge_main.hpp"
#include "ge_spm_buffer.hpp"
#include "ge_wgpu_driver.hpp"

#include "IAnimatedMesh.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

namespace GE
{
// ----------------------------------------------------------------------------
GEWGPUMeshCache::GEWGPUMeshCache()
               : irr::scene::CMeshCache()
{
    m_irrlicht_cache_time = getMonoTimeMs();
    m_ge_cache_time = 0;
    m_buffer_size = m_ibo_offset = m_skinning_vbo_offset = 0;
}   // GEWGPUMeshCache

// ----------------------------------------------------------------------------
void GEWGPUMeshCache::meshCacheChanged()
{
    m_irrlicht_cache_time = getMonoTimeMs();
}   // meshCacheChanged

// ----------------------------------------------------------------------------
void GEWGPUMeshCache::updateCache()
{
    if (m_irrlicht_cache_time <= m_ge_cache_time)
        return;
    m_ge_cache_time = m_irrlicht_cache_time;

    destroy();
    const size_t total_pitch = getVertexPitchFromType(video::EVT_SKINNED_MESH);
    const size_t bone_pitch = sizeof(int16_t) * 8;
    const size_t static_pitch = total_pitch - bone_pitch;
    size_t vbo_size = 0;
    size_t ibo_size = 0;
    size_t skinning_size = 0;
    std::vector<GESPMBuffer*> buffers;
    for (unsigned i = 0; i < Meshes.size(); i++)
    {
        scene::IAnimatedMesh* mesh = Meshes[i].Mesh;
        if (mesh->getMeshType() != scene::EAMT_SPM)
            continue;
        for (unsigned j = 0; j < mesh->getMeshBufferCount(); j++)
        {
            GESPMBuffer* mb = static_cast<GESPMBuffer*>(mesh->getMeshBuffer(j));
            vbo_size += mb->getVertexCount() * static_pitch;
            ibo_size += mb->getIndexCount() * sizeof(uint16_t);
            if (mb->hasSkinning())
                skinning_size += mb->getVertexCount() * bone_pitch;
            buffers.push_back(mb);
        }
    }
    if (buffers.empty())
        return;

    // Skinned meshes last, so their bone data can be addressed with the same
    // base vertex as the static data (see m_skinning_vbo_offset)
    std::stable_partition(buffers.begin(), buffers.end(),
        [](const GESPMBuffer* mb) { return !mb->hasSkinning(); });

    m_ibo_offset = vbo_size;
    m_skinning_vbo_offset = m_ibo_offset + ibo_size;
    m_skinning_vbo_offset += getPadding(m_skinning_vbo_offset, 4);
    m_buffer_size = m_skinning_vbo_offset + skinning_size;
    m_buffer_size += getPadding(m_buffer_size, 4);
    std::vector<uint8_t> data(m_buffer_size, 0);

    size_t offset = 0;
    for (GESPMBuffer* mb : buffers)
    {
        copyToMappedBuffer((uint32_t*)(data.data() + offset), mb);
        mb->setVBOOffset(offset / static_pitch);
        offset += mb->getVertexCount() * static_pitch;
    }
    offset = 0;
    for (GESPMBuffer* mb : buffers)
    {
        size_t copy_size = mb->getIndexCount() * sizeof(uint16_t);
        memcpy(data.data() + m_ibo_offset + offset, mb->getIndices(),
            copy_size);
        mb->setIBOOffset(offset / sizeof(uint16_t));
        offset += copy_size;
    }

    offset = 0;
    size_t static_vertex_count = 0;
    for (GESPMBuffer* mb : buffers)
    {
        if (!mb->hasSkinning())
        {
            static_vertex_count += mb->getVertexCount();
            continue;
        }
        uint8_t* loc = data.data() + m_skinning_vbo_offset + offset;
        const uint8_t* vertices = (const uint8_t*)mb->getVertices();
        for (unsigned i = 0; i < mb->getVertexCount(); i++)
        {
            memcpy(loc, vertices + i * total_pitch + static_pitch, bone_pitch);
            loc += bone_pitch;
        }
        offset += mb->getVertexCount() * bone_pitch;
    }
    // Vertex index n of a skinned mesh reads its bones at
    // m_skinning_vbo_offset + n * bone_pitch
    assert(static_vertex_count * bone_pitch <= m_skinning_vbo_offset);
    m_skinning_vbo_offset -= static_vertex_count * bone_pitch;

    GEWGPUDriver* driver = getWGPUDriver();
    wgpu::BufferDescriptor desc;
    desc.label = "mesh cache";
    desc.size = m_buffer_size;
    desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index |
        wgpu::BufferUsage::CopyDst;
    m_buffer = driver->getDevice().CreateBuffer(&desc);
    driver->getQueue().WriteBuffer(m_buffer, 0, data.data(), m_buffer_size);
}   // updateCache

// ----------------------------------------------------------------------------
void GEWGPUMeshCache::destroy()
{
    // Not Destroy(): a frame recorded with it may not be submitted yet
    m_buffer = nullptr;
    m_buffer_size = m_ibo_offset = m_skinning_vbo_offset = 0;
}   // destroy

}
