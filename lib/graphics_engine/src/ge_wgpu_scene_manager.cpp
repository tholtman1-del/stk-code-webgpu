#include "ge_wgpu_scene_manager.hpp"

#include "../source/Irrlicht/os.h"

#include "ge_main.hpp"
#include "ge_material_manager.hpp"
#include "ge_vulkan_animated_mesh_scene_node.hpp"
#include "ge_vulkan_mesh_scene_node.hpp"
#include "ge_wgpu_camera_scene_node.hpp"
#include "ge_wgpu_draw_call.hpp"
#include "ge_wgpu_driver.hpp"
#include "ge_wgpu_fbo_texture.hpp"
#include "ge_wgpu_mesh_cache.hpp"

#include "ILightSceneNode.h"
#include <sstream>

namespace GE
{
// ----------------------------------------------------------------------------
GEWGPUSceneManager::GEWGPUSceneManager(irr::video::IVideoDriver* driver,
                                       irr::io::IFileSystem* fs,
                                       irr::gui::ICursorControl* cursor_control,
                                       irr::gui::IGUIEnvironment* gui_environment)
                  : CSceneManager(driver, fs, cursor_control,
                                  new GEWGPUMeshCache(), gui_environment)
{
    // CSceneManager grabbed it
    getMeshCache()->drop();
}   // GEWGPUSceneManager

// ----------------------------------------------------------------------------
GEWGPUSceneManager::~GEWGPUSceneManager()
{
}   // ~GEWGPUSceneManager

// ----------------------------------------------------------------------------
irr::scene::ICameraSceneNode* GEWGPUSceneManager::addCameraSceneNode(
                                                irr::scene::ISceneNode* parent,
                                          const irr::core::vector3df& position,
                                            const irr::core::vector3df& lookat,
                                                 irr::s32 id, bool make_active)
{
    if (!parent)
        parent = this;

    irr::scene::ICameraSceneNode* node = new GEWGPUCameraSceneNode(parent,
        this, id, position, lookat);

    if (make_active)
        setActiveCamera(node);
    node->drop();

    return node;
}   // addCameraSceneNode

// ----------------------------------------------------------------------------
irr::scene::IAnimatedMeshSceneNode* GEWGPUSceneManager::addAnimatedMeshSceneNode(
    irr::scene::IAnimatedMesh* mesh, irr::scene::ISceneNode* parent,
    irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale,
    bool alsoAddIfMeshPointerZero)
{
    if (!alsoAddIfMeshPointerZero && (!mesh ||
        mesh->getMeshType() != irr::scene::EAMT_SPM))
        return NULL;

    if (!parent)
        parent = this;

    irr::scene::IAnimatedMeshSceneNode* node =
        new GEVulkanAnimatedMeshSceneNode(mesh, parent, this, id, position,
        rotation, scale);
    node->drop();
    node->setMesh(mesh);
    return node;
}   // addAnimatedMeshSceneNode

// ----------------------------------------------------------------------------
irr::scene::IMeshSceneNode* GEWGPUSceneManager::addMeshSceneNode(
    irr::scene::IMesh* mesh,
    irr::scene::ISceneNode* parent, irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale,
    bool alsoAddIfMeshPointerZero)
{
    if (!alsoAddIfMeshPointerZero && !mesh)
        return NULL;

    bool convert_irrlicht_mesh = false;
    if (mesh)
    {
        for (unsigned i = 0; i < mesh->getMeshBufferCount(); i++)
        {
            irr::scene::IMeshBuffer* b = mesh->getMeshBuffer(i);
            if (b->getVertexType() != irr::video::EVT_SKINNED_MESH)
            {
                if (!getGEConfig()->m_convert_irrlicht_mesh)
                {
                    return irr::scene::CSceneManager::addMeshSceneNode(
                        mesh, parent, id, position, rotation, scale,
                        alsoAddIfMeshPointerZero);
                }
                convert_irrlicht_mesh = true;
                break;
            }
        }
    }

    if (!parent)
        parent = this;

    if (convert_irrlicht_mesh)
    {
        irr::scene::IAnimatedMesh* spm = convertIrrlichtMeshToSPM(mesh);
        std::stringstream oss;
        oss << (uint64_t)spm;
        getMeshCache()->addMesh(oss.str().c_str(), spm);
        mesh = spm;
    }

    GEVulkanMeshSceneNode* ge_node = new GEVulkanMeshSceneNode(mesh, parent,
        this, id, position, rotation, scale);
    irr::scene::IMeshSceneNode* node = ge_node;
    node->drop();

    if (convert_irrlicht_mesh)
    {
        ge_node->setRemoveFromMeshCache(true);
        mesh->drop();
    }
    return node;
}   // addMeshSceneNode

// ----------------------------------------------------------------------------
void GEWGPUSceneManager::drawAll(irr::u32 flags)
{
    static_cast<GEWGPUMeshCache*>(getMeshCache())->updateCache();
    OnAnimate(irr::os::Timer::getTime());
    GEWGPUCameraSceneNode* cam =
        static_cast<GEWGPUCameraSceneNode*>(getActiveCamera());
    if (!cam)
        return;
    GEWGPUDriver* driver = static_cast<GEWGPUDriver*>(getVideoDriver());
    GEWGPUFBOTexture* rtt = driver->getRenderTargetTexture();
    if (rtt)
    {
        cam->setViewPort(irr::core::recti(0, 0, rtt->getSize().Width,
            rtt->getSize().Height));
    }
    cam->render();
    auto it = m_draw_calls.find(cam);
    if (it == m_draw_calls.end())
        return;
    it->second->prepare(cam);
    OnRegisterSceneNode();
    it->second->generate();
    if (rtt)
        driver->renderToTexture(it->second.get());
}   // drawAll

// ----------------------------------------------------------------------------
irr::u32 GEWGPUSceneManager::registerNodeForRendering(
    irr::scene::ISceneNode* node,
    irr::scene::E_SCENE_NODE_RENDER_PASS pass)
{
    if (!getActiveCamera())
        return 0;

    auto it = m_draw_calls.find(
        static_cast<GEWGPUCameraSceneNode*>(getActiveCamera()));
    if (it == m_draw_calls.end())
        return 0;
    GEWGPUDrawCall* dc = it->second.get();

    switch (node->getType())
    {
    case irr::scene::ESNT_SKY_BOX:
        dc->addSkyBox(node);
        return 1;
    case irr::scene::ESNT_LIGHT:
        dc->addLightNode(static_cast<irr::scene::ILightSceneNode*>(node));
        return 1;
    case irr::scene::ESNT_BILLBOARD:
    case irr::scene::ESNT_PARTICLE_SYSTEM:
        dc->addBillboardNode(node, node->getType());
        return 1;
    case irr::scene::ESNT_ANIMATED_MESH:
    case irr::scene::ESNT_MESH:
        if (pass != irr::scene::ESNRP_SOLID)
            return 0;
        dc->addNode(node);
        return 1;
    default:
        return 0;
    }
}   // registerNodeForRendering

// ----------------------------------------------------------------------------
namespace
{
// ----------------------------------------------------------------------------
/** Counts visible meshes with displace materials, as
 *  GEVulkanSceneManager::detectDeferred. */
unsigned countDisplace(irr::scene::ISceneNode* node)
{
    unsigned count = 0;
    if (node->isVisible() &&
        (node->getType() == irr::scene::ESNT_ANIMATED_MESH ||
        node->getType() == irr::scene::ESNT_MESH))
    {
        for (unsigned i = 0; i < node->getMaterialCount(); i++)
        {
            if (GEMaterialManager::getShader(node->getMaterial(i)
                .MaterialType) == "displace")
                count++;
        }
    }
    const irr::core::array<irr::scene::ISceneNode*>& children =
        node->getChildren();
    for (unsigned i = 0; i < children.size(); i++)
        count += countDisplace(children[i]);
    return count;
}   // countDisplace

}   // anonymous namespace

// ----------------------------------------------------------------------------
void GEWGPUSceneManager::addDrawCall(GEWGPUCameraSceneNode* cam)
{
    // Deferred rendering for displace (water), decided when a camera is
    // added after the track is loaded, as in the Vulkan renderer
    if (!static_cast<GEWGPUDriver*>(getVideoDriver())->getRenderTargetTexture())
    {
        getGEConfig()->m_auto_deferred_type =
            countDisplace(this) > 0 ? GADT_DISPLACE : GADT_DISABLED;
    }
    m_draw_calls[cam] = std::unique_ptr<GEWGPUDrawCall>(new GEWGPUDrawCall);
}   // addDrawCall

// ----------------------------------------------------------------------------
void GEWGPUSceneManager::removeDrawCall(GEWGPUCameraSceneNode* cam)
{
    m_draw_calls.erase(cam);
}   // removeDrawCall

}
