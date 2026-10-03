#ifndef HEADER_GE_WGPU_DYNAMIC_SPM_BUFFER_HPP
#define HEADER_GE_WGPU_DYNAMIC_SPM_BUFFER_HPP

#include "ge_spm_buffer.hpp"

#include <webgpu/webgpu_cpp.h>

namespace GE
{
/** Mesh buffer whose data changes at runtime (skid marks, shadows, rubber
 *  bands), kept in its own GPU buffers and updated from the dirty offsets.
 *  Writes are queued before the frame's single submit, so one copy is
 *  enough. */
class GEWGPUDynamicSPMBuffer : public GESPMBuffer
{
private:
    wgpu::Buffer m_vertex_buffer, m_index_buffer;

    uint64_t m_vertex_capacity, m_index_capacity;

    // Number of vertices / indices already uploaded and unchanged
    unsigned m_vertex_uploaded, m_index_uploaded;
public:
    // ------------------------------------------------------------------------
    GEWGPUDynamicSPMBuffer();
    // ------------------------------------------------------------------------
    virtual irr::scene::E_HARDWARE_MAPPING getHardwareMappingHint_Vertex() const
                                             { return irr::scene::EHM_STREAM; }
    // ------------------------------------------------------------------------
    virtual irr::scene::E_HARDWARE_MAPPING getHardwareMappingHint_Index() const
                                             { return irr::scene::EHM_STREAM; }
    // ------------------------------------------------------------------------
    virtual void setDirtyOffset(irr::u32 offset,
          irr::scene::E_BUFFER_TYPE buffer = irr::scene::EBT_VERTEX_AND_INDEX);
    // ------------------------------------------------------------------------
    /** Uploads changed data, returns false if there is nothing to draw. */
    bool update();
    // ------------------------------------------------------------------------
    const wgpu::Buffer& getVertexBuffer() const     { return m_vertex_buffer; }
    // ------------------------------------------------------------------------
    const wgpu::Buffer& getIndexBuffer() const       { return m_index_buffer; }
};

} // end namespace GE

#endif
