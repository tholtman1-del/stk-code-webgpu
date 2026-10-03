#ifndef HEADER_GE_WGPU_MESH_CACHE_HPP
#define HEADER_GE_WGPU_MESH_CACHE_HPP

#include "../source/Irrlicht/CMeshCache.h"

#include <webgpu/webgpu_cpp.h>
#include <cstdint>

namespace GE
{
/** Packs all SPM meshes into one buffer (static vertices, indices, then bone
 *  data of skinned meshes), drawn with base vertex and first index offsets.
 *  Same layout as GEVulkanMeshCache. */
class GEWGPUMeshCache : public irr::scene::CMeshCache
{
private:
    uint64_t m_irrlicht_cache_time, m_ge_cache_time;

    wgpu::Buffer m_buffer;

    uint64_t m_buffer_size, m_ibo_offset, m_skinning_vbo_offset;
public:
    // ------------------------------------------------------------------------
    GEWGPUMeshCache();
    // ------------------------------------------------------------------------
    virtual void meshCacheChanged();
    // ------------------------------------------------------------------------
    void updateCache();
    // ------------------------------------------------------------------------
    void destroy();
    // ------------------------------------------------------------------------
    const wgpu::Buffer& getBuffer() const                  { return m_buffer; }
    // ------------------------------------------------------------------------
    uint64_t getBufferSize() const                    { return m_buffer_size; }
    // ------------------------------------------------------------------------
    uint64_t getIBOOffset() const                      { return m_ibo_offset; }
    // ------------------------------------------------------------------------
    uint64_t getSkinningVBOOffset() const     { return m_skinning_vbo_offset; }
};   // GEWGPUMeshCache

}

#endif
