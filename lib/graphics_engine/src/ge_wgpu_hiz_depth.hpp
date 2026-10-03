#ifndef HEADER_GE_WGPU_HIZ_DEPTH_HPP
#define HEADER_GE_WGPU_HIZ_DEPTH_HPP

#include "rect.h"

#include <webgpu/webgpu_cpp.h>

#include <vector>

namespace GE
{
class GEWGPUDeferredFBO;

/** Min depth pyramid of a camera's viewport for screen space reflections
 *  (hiz_depth.comp), as GEVulkanHiZDepth. Level 0 copies the deferred depth,
 *  each other level reads the levels before it (a separate view, so no
 *  subresource is both read and written in a dispatch). */
class GEWGPUHiZDepth
{
private:
    irr::core::recti m_viewport;

    const GEWGPUDeferredFBO* m_dfbo;

    wgpu::Texture m_hiz_depth;

    wgpu::ComputePipeline m_pipeline;

    wgpu::Buffer m_push_constants;

    std::vector<wgpu::BindGroup> m_level_bind_groups,
        m_push_constants_bind_groups;

    /** Group 3 of displace_mask.frag */
    wgpu::BindGroup m_rendering_bind_group;

    // ------------------------------------------------------------------------
    void init();
public:
    // ------------------------------------------------------------------------
    GEWGPUHiZDepth() : m_dfbo(NULL) {}
    // ------------------------------------------------------------------------
    /** viewport: upper left corner and size, as GEWGPUCameraUBO */
    void prepare(const irr::core::recti& viewport,
                 const GEWGPUDeferredFBO* dfbo);
    // ------------------------------------------------------------------------
    void generate(wgpu::CommandEncoder& encoder);
    // ------------------------------------------------------------------------
    const wgpu::BindGroup& getRenderingBindGroup() const
                                              { return m_rendering_bind_group; }
};   // GEWGPUHiZDepth

}

#endif
