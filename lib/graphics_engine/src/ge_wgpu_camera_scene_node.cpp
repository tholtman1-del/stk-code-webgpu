#include "ge_wgpu_camera_scene_node.hpp"

#include "ge_wgpu_driver.hpp"
#include "ge_wgpu_scene_manager.hpp"

namespace GE
{
// ----------------------------------------------------------------------------
GEWGPUCameraSceneNode::GEWGPUCameraSceneNode(irr::scene::ISceneNode* parent,
                                             irr::scene::ISceneManager* mgr,
                                             irr::s32 id,
                                           const irr::core::vector3df& position,
                                             const irr::core::vector3df& lookat)
                     : CCameraSceneNode(parent, mgr, id, position, lookat)
{
    static_cast<GEWGPUSceneManager*>(SceneManager)->addDrawCall(this);
}   // GEWGPUCameraSceneNode

// ----------------------------------------------------------------------------
GEWGPUCameraSceneNode::~GEWGPUCameraSceneNode()
{
    static_cast<GEWGPUSceneManager*>(SceneManager)->removeDrawCall(this);
}   // ~GEWGPUCameraSceneNode

// ----------------------------------------------------------------------------
void GEWGPUCameraSceneNode::render()
{
    irr::scene::CCameraSceneNode::render();

    m_ubo_data.m_view_matrix = ViewArea.getTransform(irr::video::ETS_VIEW);
    // The shaders are converted from Vulkan GLSL and naga negates the output
    // y, so the Vulkan clip space correction (inverted y, half z) is kept
    irr::core::matrix4 clip;
    clip[5] = -1.0f;
    clip[10] = 0.5f;
    clip[14] = 0.5f;
    m_ubo_data.m_projection_matrix =
        clip * ViewArea.getTransform(irr::video::ETS_PROJECTION);

    irr::core::matrix4 mat;
    m_ubo_data.m_view_matrix.getInverse(mat);
    m_ubo_data.m_inverse_view_matrix = mat;
    m_ubo_data.m_projection_matrix.getInverse(mat);
    m_ubo_data.m_inverse_projection_matrix = mat;
    m_ubo_data.m_projection_view_matrix =
        m_ubo_data.m_projection_matrix * m_ubo_data.m_view_matrix;
    m_ubo_data.m_projection_view_matrix.getInverse(
        m_ubo_data.m_inverse_projection_view_matrix);

    // In the scaled scene target with a render scale, as
    // GEVulkanCameraSceneNode
    const float scale = getWGPUDriver()->getRenderScale();
    m_ubo_data.m_viewport.UpperLeftCorner.X = m_viewport.UpperLeftCorner.X *
        scale;
    m_ubo_data.m_viewport.UpperLeftCorner.Y = m_viewport.UpperLeftCorner.Y *
        scale;
    m_ubo_data.m_viewport.LowerRightCorner.X = m_viewport.getWidth() * scale;
    m_ubo_data.m_viewport.LowerRightCorner.Y = m_viewport.getHeight() * scale;
    const irr::core::dimension2du size = getWGPUDriver()->getSceneSize();
    m_ubo_data.m_screensize.UpperLeftCorner.X = size.Width;
    m_ubo_data.m_screensize.UpperLeftCorner.Y = size.Height;
    m_ubo_data.m_screensize.LowerRightCorner.X = 0.0f;
    m_ubo_data.m_screensize.LowerRightCorner.Y = 0.0f;
}   // render

}
