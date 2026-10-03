#ifndef HEADER_GE_WGPU_SCENE_MANAGER_HPP
#define HEADER_GE_WGPU_SCENE_MANAGER_HPP

#include "../source/Irrlicht/CSceneManager.h"

#include <map>
#include <memory>

namespace GE
{
class GEWGPUCameraSceneNode;
class GEWGPUDrawCall;

/** Scene manager of the WebGPU renderer: GE mesh and camera nodes, and one
 *  GEWGPUDrawCall per camera which collects the visible nodes in drawAll()
 *  and is rendered by GEWGPUDriver::endScene(). */
class GEWGPUSceneManager : public irr::scene::CSceneManager
{
private:
    std::map<GEWGPUCameraSceneNode*, std::unique_ptr<GEWGPUDrawCall> >
        m_draw_calls;
public:
    // ------------------------------------------------------------------------
    GEWGPUSceneManager(irr::video::IVideoDriver* driver,
                       irr::io::IFileSystem* fs,
                       irr::gui::ICursorControl* cursor_control,
                       irr::gui::IGUIEnvironment* gui_environment);
    // ------------------------------------------------------------------------
    ~GEWGPUSceneManager();
    // ------------------------------------------------------------------------
    virtual irr::scene::ICameraSceneNode* addCameraSceneNode(
        irr::scene::ISceneNode* parent = 0,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& lookat = irr::core::vector3df(0, 0, 100),
        irr::s32 id = -1, bool make_active = true);
    // ------------------------------------------------------------------------
    virtual irr::scene::IAnimatedMeshSceneNode* addAnimatedMeshSceneNode(
        irr::scene::IAnimatedMesh* mesh, irr::scene::ISceneNode* parent = NULL,
        irr::s32 id = -1,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& rotation = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f),
        bool alsoAddIfMeshPointerZero = false);
    // ------------------------------------------------------------------------
    virtual irr::scene::IMeshSceneNode* addMeshSceneNode(irr::scene::IMesh* mesh,
        irr::scene::ISceneNode* parent = NULL, irr::s32 id = -1,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& rotation = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f),
        bool alsoAddIfMeshPointerZero = false);
    // ------------------------------------------------------------------------
    virtual void drawAll(irr::u32 flags = 0xFFFFFFFF);
    // ------------------------------------------------------------------------
    virtual irr::u32 registerNodeForRendering(irr::scene::ISceneNode* node,
        irr::scene::E_SCENE_NODE_RENDER_PASS pass = irr::scene::ESNRP_AUTOMATIC);
    // ------------------------------------------------------------------------
    void addDrawCall(GEWGPUCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    void removeDrawCall(GEWGPUCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    std::map<GEWGPUCameraSceneNode*, std::unique_ptr<GEWGPUDrawCall> >&
                                        getDrawCalls() { return m_draw_calls; }
};   // GEWGPUSceneManager

}

#endif
