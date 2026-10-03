#include "ge_wgpu_dynamic_spm_buffer.hpp"

#include "ge_main.hpp"
#include "ge_wgpu_driver.hpp"

#include <algorithm>
#include <vector>

namespace GE
{
namespace
{
const size_t STATIC_PITCH = sizeof(irr::video::S3DVertexSkinnedMesh) - 16;

// ----------------------------------------------------------------------------
/** Returns true if the buffer was recreated (its old content is lost). */
bool ensureCapacity(wgpu::Buffer& buffer, uint64_t& capacity, uint64_t needed,
                    wgpu::BufferUsage usage)
{
    if (buffer && capacity >= needed)
        return false;
    uint64_t size = 256;
    while (size < needed)
        size *= 2;
    wgpu::BufferDescriptor desc;
    desc.label = "dynamic spm";
    desc.size = size;
    desc.usage = usage | wgpu::BufferUsage::CopyDst;
    buffer = getWGPUDriver()->getDevice().CreateBuffer(&desc);
    capacity = size;
    return true;
}   // ensureCapacity

}   // anonymous namespace

// ----------------------------------------------------------------------------
GEWGPUDynamicSPMBuffer::GEWGPUDynamicSPMBuffer()
{
    m_vertex_capacity = m_index_capacity = 0;
    m_vertex_uploaded = m_index_uploaded = 0;
}   // GEWGPUDynamicSPMBuffer

// ----------------------------------------------------------------------------
void GEWGPUDynamicSPMBuffer::setDirtyOffset(irr::u32 offset,
                                            irr::scene::E_BUFFER_TYPE buffer)
{
    if (buffer == irr::scene::EBT_VERTEX ||
        buffer == irr::scene::EBT_VERTEX_AND_INDEX)
        m_vertex_uploaded = std::min(m_vertex_uploaded, (unsigned)offset);
    if (buffer == irr::scene::EBT_INDEX ||
        buffer == irr::scene::EBT_VERTEX_AND_INDEX)
        m_index_uploaded = std::min(m_index_uploaded, (unsigned)offset);
}   // setDirtyOffset

// ----------------------------------------------------------------------------
bool GEWGPUDynamicSPMBuffer::update()
{
    if (m_vertices.empty() || m_indices.empty())
        return false;
    const wgpu::Queue& queue = getWGPUDriver()->getQueue();

    if (m_vertex_uploaded > m_vertices.size())
        m_vertex_uploaded = 0;
    if (ensureCapacity(m_vertex_buffer, m_vertex_capacity,
        m_vertices.size() * STATIC_PITCH, wgpu::BufferUsage::Vertex))
        m_vertex_uploaded = 0;
    if (m_vertex_uploaded < m_vertices.size())
    {
        // STATIC_PITCH is a multiple of 4 as WriteBuffer requires
        std::vector<uint8_t> data(
            (m_vertices.size() - m_vertex_uploaded) * STATIC_PITCH);
        copyToMappedBuffer((uint32_t*)data.data(), this, m_vertex_uploaded);
        queue.WriteBuffer(m_vertex_buffer, m_vertex_uploaded * STATIC_PITCH,
            data.data(), data.size());
        m_vertex_uploaded = m_vertices.size();
    }

    if (m_index_uploaded > m_indices.size())
        m_index_uploaded = 0;
    // Offset and size of WriteBuffer must be multiples of 4 bytes
    m_index_uploaded &= ~1u;
    if (ensureCapacity(m_index_buffer, m_index_capacity,
        (m_indices.size() + 1) * sizeof(uint16_t), wgpu::BufferUsage::Index))
        m_index_uploaded = 0;
    if (m_index_uploaded < m_indices.size())
    {
        std::vector<uint16_t> data(m_indices.begin() + m_index_uploaded,
            m_indices.end());
        if (data.size() % 2 != 0)
            data.push_back(0);
        queue.WriteBuffer(m_index_buffer, m_index_uploaded * sizeof(uint16_t),
            data.data(), data.size() * sizeof(uint16_t));
        m_index_uploaded = m_indices.size();
    }
    return true;
}   // update

}
