#ifndef HEADER_GE_WGPU_CAMERA_SCENE_NODE_HPP
#define HEADER_GE_WGPU_CAMERA_SCENE_NODE_HPP

#include "../source/Irrlicht/CCameraSceneNode.h"

namespace GE
{
/** Same layout as CameraBuffer in the shaders (and GEVulkanCameraUBO). */
struct GEWGPUCameraUBO
{
    irr::core::matrix4 m_view_matrix;
    irr::core::matrix4 m_projection_matrix;
    irr::core::matrix4 m_inverse_view_matrix;
    irr::core::matrix4 m_inverse_projection_matrix;
    irr::core::matrix4 m_projection_view_matrix;
    irr::core::matrix4 m_inverse_projection_view_matrix;
    irr::core::rectf   m_viewport;
    irr::core::rectf   m_screensize;
};

class GEWGPUCameraSceneNode : public irr::scene::CCameraSceneNode
{
private:
    GEWGPUCameraUBO m_ubo_data;

    irr::core::rect<irr::s32> m_viewport;
public:
    // ------------------------------------------------------------------------
    GEWGPUCameraSceneNode(irr::scene::ISceneNode* parent,
                          irr::scene::ISceneManager* mgr, irr::s32 id,
          const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
         const irr::core::vector3df& lookat = irr::core::vector3df(0, 0, 100));
    // ------------------------------------------------------------------------
    ~GEWGPUCameraSceneNode();
    // ------------------------------------------------------------------------
    virtual void render();
    // ------------------------------------------------------------------------
    void setViewPort(const irr::core::rect<irr::s32>& area)
                                                         { m_viewport = area; }
    // ------------------------------------------------------------------------
    const irr::core::rect<irr::s32>& getViewPort() const { return m_viewport; }
    // ------------------------------------------------------------------------
    const GEWGPUCameraUBO* getUBOData() const           { return &m_ubo_data; }
};   // GEWGPUCameraSceneNode

}

#endif
