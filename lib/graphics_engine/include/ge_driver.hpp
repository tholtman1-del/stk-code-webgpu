#ifndef HEADER_GE_DRIVER_HPP
#define HEADER_GE_DRIVER_HPP

#include "../source/Irrlicht/CNullDriver.h"

struct SDL_Window;

namespace GE
{
enum GEVulkanSampler : unsigned
{
    GVS_MIN = 0,
    GVS_NEAREST = GVS_MIN,
    GVS_SKYBOX,
    GVS_3D_MESH_MIPMAP_2,
    GVS_3D_MESH_MIPMAP_4,
    GVS_3D_MESH_MIPMAP_16,
    GVS_2D_RENDER,
    GVS_SHADOW,
    GVS_COUNT,
};

/** Interface shared by the GE renderers (Vulkan and WebGPU), so game code can
 *  talk to whichever one is active without depending on a graphics API. */
class GEDriver : public irr::video::CNullDriver
{
public:
    // ------------------------------------------------------------------------
    GEDriver(irr::io::IFileSystem* io,
             const irr::core::dimension2d<irr::u32>& screen_size)
        : CNullDriver(io, screen_size)                                     {}
    // ------------------------------------------------------------------------
    virtual void updateDriver(bool scale_changed = true,
                              bool pbr_changed = false,
                              bool ibl_changed = false) = 0;
    // ------------------------------------------------------------------------
    virtual void updateSwapInterval(int value) = 0;
    // ------------------------------------------------------------------------
    virtual void reloadShaders() = 0;
    // ------------------------------------------------------------------------
    virtual void clearDrawCallsCache() = 0;
    // ------------------------------------------------------------------------
    /** Blocks until the GPU finished all submitted work. */
    virtual void waitIdle(bool flush_command_loader = false) = 0;
    // ------------------------------------------------------------------------
    virtual void setDisableWaitIdle(bool val) = 0;
    // ------------------------------------------------------------------------
    virtual SDL_Window* getSDLWindow() const = 0;
    // ------------------------------------------------------------------------
    /** Selects the sampler (anisotropy level) used for mesh textures. */
    virtual void setMeshSamplerUse(GEVulkanSampler sampler) = 0;
    // ------------------------------------------------------------------------
    /** True if any block-compressed texture format can be uploaded. */
    virtual bool supportsTextureCompression() const = 0;
    // ------------------------------------------------------------------------
    /** Releases all GPU objects before the driver is dropped. */
    virtual void destroyDriver() = 0;
};   // GEDriver

}

#endif
