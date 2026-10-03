#ifndef HEADER_GE_WGPU_DRIVER_HPP
#define HEADER_GE_WGPU_DRIVER_HPP

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_WEBGPU_

#include "ge_driver.hpp"
#include "SIrrCreationParameters.h"

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

using namespace irr;
using namespace video;

namespace GE
{
class GEWGPUDeferredFBO;
class GEWGPUDrawCall;
class GEWGPUFBOTexture;
class GEWGPUSkyBoxRenderer;
class GEWGPUTexture;

/** GE renderer on top of WebGPU, used by the browser (Emscripten) build.
 *  The device is created by the page before main() runs (WebGPU device
 *  creation is asynchronous) and handed over through
 *  Module.preinitializedWebGPUDevice. All WebGPU calls must happen on the
 *  browser main thread, work from other threads goes through
 *  runOnMainThread(). */
class GEWGPUDriver : public GEDriver
{
public:
    // ------------------------------------------------------------------------
    GEWGPUDriver(const SIrrlichtCreationParameters& params,
                 io::IFileSystem* io, SDL_Window* window,
                 IrrlichtDevice* device);
    // ------------------------------------------------------------------------
    virtual ~GEWGPUDriver();
    // ------------------------------------------------------------------------
    virtual bool beginScene(bool backBuffer = true, bool zBuffer = true,
                            SColor color = SColor(255, 0, 0, 0),
                            const SExposedVideoData& videoData =
                            SExposedVideoData(),
                            core::rect<s32>* sourceRect = 0);
    // ------------------------------------------------------------------------
    virtual bool endScene();
    // ------------------------------------------------------------------------
    virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const
                                                               { return true; }
    // ------------------------------------------------------------------------
    virtual void setTransform(E_TRANSFORMATION_STATE state,
                              const core::matrix4& mat)                     {}
    // ------------------------------------------------------------------------
    virtual void setMaterial(const SMaterial& material)
                                                     { Material = material; }
    // ------------------------------------------------------------------------
    virtual bool setRenderTarget(video::ITexture* texture,
                                 bool clearBackBuffer = true,
                                 bool clearZBuffer = true,
                                 SColor color = video::SColor(0, 0, 0, 0));
    // ------------------------------------------------------------------------
    virtual bool setRenderTarget(const core::array<video::IRenderTarget>& t,
                                 bool clearBackBuffer = true,
                                 bool clearZBuffer = true,
                                 SColor color = video::SColor(0, 0, 0, 0))
                                                               { return true; }
    // ------------------------------------------------------------------------
    virtual void setViewPort(const core::rect<s32>& area);
    // ------------------------------------------------------------------------
    virtual bool updateHardwareBuffer(SHWBufferLink* HWBuffer) { return false; }
    // ------------------------------------------------------------------------
    virtual SHWBufferLink* createHardwareBuffer(const scene::IMeshBuffer* mb)
                                                               { return NULL; }
    // ------------------------------------------------------------------------
    virtual void deleteHardwareBuffer(SHWBufferLink* HWBuffer)              {}
    // ------------------------------------------------------------------------
    virtual void drawHardwareBuffer(SHWBufferLink* HWBuffer)                {}
    // ------------------------------------------------------------------------
    virtual void addOcclusionQuery(scene::ISceneNode* node,
                                   const scene::IMesh* mesh = 0)            {}
    // ------------------------------------------------------------------------
    virtual void removeOcclusionQuery(scene::ISceneNode* node)              {}
    // ------------------------------------------------------------------------
    virtual void runOcclusionQuery(scene::ISceneNode* node,
                                   bool visible = false)                    {}
    // ------------------------------------------------------------------------
    virtual void updateOcclusionQuery(scene::ISceneNode* node,
                                      bool block = true)                    {}
    // ------------------------------------------------------------------------
    virtual u32 getOcclusionQueryResult(scene::ISceneNode* node) const
                                                                  { return 0; }
    // ------------------------------------------------------------------------
    virtual void drawVertexPrimitiveList(const void* vertices,
                                         u32 vertexCount,
                                         const void* indexList,
                                         u32 primitiveCount,
                                         E_VERTEX_TYPE vType,
                                         scene::E_PRIMITIVE_TYPE pType,
                                         E_INDEX_TYPE iType)                {}
    // ------------------------------------------------------------------------
    virtual void draw2DVertexPrimitiveList(const void* vertices,
                                           u32 vertexCount,
                                           const void* indexList,
                                           u32 primitiveCount,
                                           E_VERTEX_TYPE vType,
                                           scene::E_PRIMITIVE_TYPE pType,
                                           E_INDEX_TYPE iType);
    // ------------------------------------------------------------------------
    virtual void draw2DImage(const video::ITexture* texture,
                             const core::position2d<s32>& destPos,
                             const core::rect<s32>& sourceRect,
                             const core::rect<s32>* clipRect = 0,
                             SColor color = SColor(255, 255, 255, 255),
                             bool useAlphaChannelOfTexture = false);
    // ------------------------------------------------------------------------
    virtual void draw2DImage(const video::ITexture* texture,
                             const core::rect<s32>& destRect,
                             const core::rect<s32>& sourceRect,
                             const core::rect<s32>* clipRect = 0,
                             const video::SColor* const colors = 0,
                             bool useAlphaChannelOfTexture = false);
    // ------------------------------------------------------------------------
    virtual void draw2DImageBatch(const video::ITexture* texture,
                       const core::array<core::position2d<s32> >& positions,
                       const core::array<core::rect<s32> >& sourceRects,
                       const core::rect<s32>* clipRect = 0,
                       SColor color = SColor(255, 255, 255, 255),
                       bool useAlphaChannelOfTexture = false);
    // ------------------------------------------------------------------------
    virtual void draw2DRectangle(const core::rect<s32>& pos,
                                 SColor colorLeftUp, SColor colorRightUp,
                                 SColor colorLeftDown, SColor colorRightDown,
                                 const core::rect<s32>* clip)
    {
        SColor color[4] = { colorLeftUp, colorLeftDown, colorRightDown,
            colorRightUp };
        draw2DImage(m_white_texture, pos, core::recti(0, 0, 2, 2), clip,
            color, true);
    }
    // ------------------------------------------------------------------------
    virtual void draw2DLine(const core::position2d<s32>& start,
                            const core::position2d<s32>& end,
                            SColor color = SColor(255, 255, 255, 255))     {}
    // ------------------------------------------------------------------------
    virtual void drawPixel(u32 x, u32 y, const SColor& color)               {}
    // ------------------------------------------------------------------------
    virtual void draw3DLine(const core::vector3df& start,
                            const core::vector3df& end,
                            SColor color = SColor(255, 255, 255, 255))     {}
    // ------------------------------------------------------------------------
    virtual const wchar_t* getName() const               { return L"WebGPU"; }
    // ------------------------------------------------------------------------
    virtual void deleteAllDynamicLights()                                   {}
    // ------------------------------------------------------------------------
    virtual s32 addDynamicLight(const SLight& light)          { return -1; }
    // ------------------------------------------------------------------------
    virtual void turnLightOn(s32 lightIndex, bool turnOn)                   {}
    // ------------------------------------------------------------------------
    virtual u32 getMaximalDynamicLightAmount() const     { return (u32)-1; }
    // ------------------------------------------------------------------------
    virtual void setAmbientLight(const SColorf& color)
                                       { CNullDriver::setAmbientLight(color); }
    // ------------------------------------------------------------------------
    virtual void drawStencilShadowVolume(
        const core::array<core::vector3df>& triangles, bool zfail = true,
        u32 debugDataVisible = 0)                                           {}
    // ------------------------------------------------------------------------
    virtual void drawStencilShadow(bool clearStencilBuffer = false,
        video::SColor leftUpEdge = video::SColor(0, 0, 0, 0),
        video::SColor rightUpEdge = video::SColor(0, 0, 0, 0),
        video::SColor leftDownEdge = video::SColor(0, 0, 0, 0),
        video::SColor rightDownEdge = video::SColor(0, 0, 0, 0))           {}
    // ------------------------------------------------------------------------
    virtual u32 getMaximalPrimitiveCount() const         { return (u32)-1; }
    // ------------------------------------------------------------------------
    virtual void setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag,
                                        bool enabled)                       {}
    // ------------------------------------------------------------------------
    virtual void setFog(SColor color, E_FOG_TYPE fogType, f32 start,
                        f32 end, f32 density, bool pixelFog,
                        bool rangeFog)                                      {}
    // ------------------------------------------------------------------------
    virtual void OnResize(const core::dimension2d<u32>& size);
    // ------------------------------------------------------------------------
    virtual E_DRIVER_TYPE getDriverType() const  { return video::EDT_WEBGPU; }
    // ------------------------------------------------------------------------
    virtual const core::matrix4& getTransform(E_TRANSFORMATION_STATE state)
                                                                         const
    {
        static core::matrix4 unused;
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual ITexture* addRenderTargetTexture(const core::dimension2d<u32>& size,
                                             const io::path& name,
                                             const ECOLOR_FORMAT format =
                                             ECF_UNKNOWN,
                                             const bool useStencil = false);
    // ------------------------------------------------------------------------
    virtual void clearZBuffer()                                             {}
    // ------------------------------------------------------------------------
    virtual IImage* createScreenShot(video::ECOLOR_FORMAT format =
                                     video::ECF_UNKNOWN,
                                     video::E_RENDER_TARGET target =
                                     video::ERT_FRAME_BUFFER)  { return NULL; }
    // ------------------------------------------------------------------------
    virtual bool setClipPlane(u32 index, const core::plane3df& plane,
                              bool enable = false)             { return true; }
    // ------------------------------------------------------------------------
    virtual void enableClipPlane(u32 index, bool enable)                    {}
    // ------------------------------------------------------------------------
    virtual core::stringc getVendorInfo()            { return m_vendor_info; }
    // ------------------------------------------------------------------------
    virtual void enableMaterial2D(bool enable = true)                       {}
    // ------------------------------------------------------------------------
    virtual bool checkDriverReset()                           { return false; }
    // ------------------------------------------------------------------------
    virtual ECOLOR_FORMAT getColorFormat() const      { return ECF_A8R8G8B8; }
    // ------------------------------------------------------------------------
    virtual core::dimension2du getMaxTextureSize() const
                         { return core::dimension2du(m_max_texture_size,
                                                     m_max_texture_size); }
    // ------------------------------------------------------------------------
    virtual void enableScissorTest(const core::rect<s32>& r)   { m_clip = r; }
    // ------------------------------------------------------------------------
    core::rect<s32> getFullscreenClip() const
    {
        return core::rect<s32>(0, 0, ScreenSize.Width, ScreenSize.Height);
    }
    // ------------------------------------------------------------------------
    virtual void disableScissorTest()        { m_clip = getFullscreenClip(); }
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getCurrentRenderTargetSize() const;
    // GEDriver interface
    // ------------------------------------------------------------------------
    virtual void updateDriver(bool scale_changed = true,
                              bool pbr_changed = false,
                              bool ibl_changed = false);
    // ------------------------------------------------------------------------
    /** Presentation is paced by requestAnimationFrame, nothing to change. */
    virtual void updateSwapInterval(int value)                              {}
    // ------------------------------------------------------------------------
    virtual void reloadShaders();
    // ------------------------------------------------------------------------
    virtual void clearDrawCallsCache()                                      {}
    // ------------------------------------------------------------------------
    /** The browser main thread cannot block on the GPU. WebGPU keeps every
     *  object alive until the work using it finished, so nothing waits. */
    virtual void waitIdle(bool flush_command_loader = false)                {}
    // ------------------------------------------------------------------------
    virtual void setDisableWaitIdle(bool val)                               {}
    // ------------------------------------------------------------------------
    /** Textures drop their cached bind groups when destroyed or reloaded. */
    virtual void handleDeletedTextures()                                    {}
    // ------------------------------------------------------------------------
    virtual SDL_Window* getSDLWindow() const   { return m_params.m_sdl_window; }
    // ------------------------------------------------------------------------
    virtual void setMeshSamplerUse(GEVulkanSampler sampler)
                                                { m_mesh_sampler = sampler; }
    // ------------------------------------------------------------------------
    virtual bool supportsTextureCompression() const;
    // ------------------------------------------------------------------------
    virtual void destroyDriver();
    // WebGPU specific
    // ------------------------------------------------------------------------
    const wgpu::Device& getDevice() const                  { return m_device; }
    // ------------------------------------------------------------------------
    const wgpu::Queue& getQueue() const                     { return m_queue; }
    // ------------------------------------------------------------------------
    const wgpu::Sampler& getSampler(GEVulkanSampler s) const
                                                     { return m_samplers[s]; }
    // ------------------------------------------------------------------------
    GEVulkanSampler getMeshSampler() const           { return m_mesh_sampler; }
    // ------------------------------------------------------------------------
    wgpu::TextureFormat getSurfaceFormat() const   { return m_surface_format; }
    // ------------------------------------------------------------------------
    video::ITexture* getWhiteTexture() const        { return m_white_texture; }
    // ------------------------------------------------------------------------
    video::ITexture* getTransparentTexture() const
                                              { return m_transparent_texture; }
    // ------------------------------------------------------------------------
    const core::rect<s32>& getCurrentClip() const            { return m_clip; }
    // ------------------------------------------------------------------------
    video::SColor getClearColor() const               { return m_clear_color; }
    // ------------------------------------------------------------------------
    IrrlichtDevice* getIrrlichtDevice() const    { return m_irrlicht_device; }
    // ------------------------------------------------------------------------
    /** Runs f now when called from the main thread, otherwise queues it for
     *  the start of the next frame. owner allows cancelling queued tasks
     *  (cancelTasks) when the object they refer to is destroyed. */
    void runOnMainThread(std::function<void()> f, const void* owner = NULL);
    // ------------------------------------------------------------------------
    void cancelTasks(const void* owner);
    // ------------------------------------------------------------------------
    void onTextureDestroyed(const GEWGPUTexture* texture);
    // ------------------------------------------------------------------------
    bool hasFeature(wgpu::FeatureName feature) const
                                       { return m_device.HasFeature(feature); }
    // ------------------------------------------------------------------------
    /** Quad in the mesh cache used by billboards and particles. */
    scene::IMesh* getBillboardQuad() const          { return m_billboard_quad; }
    // ------------------------------------------------------------------------
    GEWGPUSkyBoxRenderer* getSkyBoxRenderer() const
                                           { return m_skybox_renderer.get(); }
    // ------------------------------------------------------------------------
    /** The texture set by setRenderTarget, NULL when drawing to the screen. */
    GEWGPUFBOTexture* getRenderTargetTexture() const   { return m_rtt_texture; }
    // ------------------------------------------------------------------------
    /** Renders a draw call into the current render target texture now, in a
     *  submit of its own (as GEVulkanSceneManager::drawAll does). */
    void renderToTexture(GEWGPUDrawCall* dc);
private:
    // ------------------------------------------------------------------------
    virtual video::ITexture* createDeviceDependentTexture(IImage* surface,
        const io::path& name, void* mipmapData = 0);
    // ------------------------------------------------------------------------
    virtual s32 addHighLevelShaderMaterial(
        const c8* vertexShaderProgram,
        const c8* vertexShaderEntryPointName,
        E_VERTEX_SHADER_TYPE vsCompileTarget,
        const c8* pixelShaderProgram,
        const c8* pixelShaderEntryPointName,
        E_PIXEL_SHADER_TYPE psCompileTarget,
        const c8* geometryShaderProgram,
        const c8* geometryShaderEntryPointName = "main",
        E_GEOMETRY_SHADER_TYPE gsCompileTarget = EGST_GS_4_0,
        scene::E_PRIMITIVE_TYPE inType = scene::EPT_TRIANGLES,
        scene::E_PRIMITIVE_TYPE outType = scene::EPT_TRIANGLE_STRIP,
        u32 verticesOut = 0,
        IShaderConstantSetCallBack* callback = 0,
        E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
        s32 userData = 0,
        E_GPU_SHADING_LANGUAGE shadingLang = EGSL_DEFAULT)        { return 0; }
    // ------------------------------------------------------------------------
    void createSamplers();
    // ------------------------------------------------------------------------
    void createUnicolorTextures();
    // ------------------------------------------------------------------------
    void configureSurface();
    // ------------------------------------------------------------------------
    void runPendingTasks();
    // ------------------------------------------------------------------------
    void createBillboardQuad();
    // ------------------------------------------------------------------------
    const wgpu::TextureView& getDepthView(const core::dimension2du& size);
    // ------------------------------------------------------------------------
    void renderDeferred(wgpu::CommandEncoder& encoder,
                        const std::vector<GEWGPUDrawCall*>& draw_calls,
                        const wgpu::TextureView& output);

    SIrrlichtCreationParameters m_params;
    SMaterial Material;
    IrrlichtDevice* m_irrlicht_device;

    wgpu::Instance m_instance;
    wgpu::Device m_device;
    wgpu::Queue m_queue;
    wgpu::Surface m_surface;
    wgpu::TextureFormat m_surface_format;
    std::array<wgpu::Sampler, GVS_COUNT> m_samplers;
    GEVulkanSampler m_mesh_sampler;
    uint32_t m_max_texture_size;
    core::stringc m_vendor_info;

    video::SColor m_clear_color;
    core::rect<s32> m_clip;
    video::ITexture* m_white_texture;
    video::ITexture* m_transparent_texture;
    scene::IMesh* m_billboard_quad;
    std::unique_ptr<GEWGPUSkyBoxRenderer> m_skybox_renderer;
    GEWGPUFBOTexture* m_rtt_texture;
    video::SColor m_rtt_clear_color;
    std::unique_ptr<GEWGPUDeferredFBO> m_deferred_fbo;
    wgpu::Texture m_depth_texture;
    wgpu::TextureView m_depth_view;

    struct PendingTask
    {
        std::function<void()> m_task;
        const void* m_owner;
    };
    std::mutex m_tasks_mutex;
    std::vector<PendingTask> m_pending_tasks;
};   // GEWGPUDriver

// ----------------------------------------------------------------------------
GEWGPUDriver* getWGPUDriver();

}

namespace irr
{
namespace video
{
    IVideoDriver* createWebGPUDriver(const SIrrlichtCreationParameters& params,
                                     io::IFileSystem* io, SDL_Window* window,
                                     IrrlichtDevice* device);
}
}

#endif // _IRR_COMPILE_WITH_WEBGPU_
#endif // HEADER_GE_WGPU_DRIVER_HPP
