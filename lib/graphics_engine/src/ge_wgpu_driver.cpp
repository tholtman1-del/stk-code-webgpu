#include "ge_wgpu_driver.hpp"

#ifdef _IRR_COMPILE_WITH_WEBGPU_

#include "ge_main.hpp"
#include "ge_material_manager.hpp"
#include "ge_wgpu_2d_renderer.hpp"
#include "ge_wgpu_shader_manager.hpp"
#include "ge_wgpu_texture.hpp"

#include "IrrlichtDevice.h"
#include "../source/Irrlicht/os.h"

#include <emscripten.h>
#include <emscripten/threading.h>

#include <algorithm>
#include <stdexcept>

namespace GE
{
// ----------------------------------------------------------------------------
EM_JS(int, ge_wgpu_preferred_format_is_rgba, (), {
    return navigator.gpu.getPreferredCanvasFormat() == "rgba8unorm" ? 1 : 0;
});

// ----------------------------------------------------------------------------
GEWGPUDriver* getWGPUDriver()
{
    return static_cast<GEWGPUDriver*>(getDriver());
}   // getWGPUDriver

// ----------------------------------------------------------------------------
GEWGPUDriver::GEWGPUDriver(const SIrrlichtCreationParameters& params,
                           io::IFileSystem* io, SDL_Window* window,
                           IrrlichtDevice* device)
            : GEDriver(io, params.WindowSize), m_params(params),
              m_irrlicht_device(device), m_mesh_sampler(GVS_3D_MESH_MIPMAP_16),
              m_max_texture_size(8192), m_white_texture(NULL),
              m_transparent_texture(NULL)
{
    // Created by the page (asynchronously) before main() was called
    m_device = wgpu::Device::Acquire(emscripten_webgpu_get_device());
    if (!m_device)
        throw std::runtime_error("No WebGPU device was provided by the page");
    m_queue = m_device.GetQueue();
    m_instance = wgpu::CreateInstance(nullptr);

    wgpu::Limits limits;
    if (m_device.GetLimits(&limits) == wgpu::Status::Success)
        m_max_texture_size = limits.maxTextureDimension2D;

    wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector canvas;
    canvas.selector = "#canvas";
    wgpu::SurfaceDescriptor surface_desc;
    surface_desc.nextInChain = &canvas;
    m_surface = m_instance.CreateSurface(&surface_desc);
    m_surface_format = ge_wgpu_preferred_format_is_rgba() ?
        wgpu::TextureFormat::RGBA8Unorm : wgpu::TextureFormat::BGRA8Unorm;
    configureSurface();

    m_vendor_info = "WebGPU";
    m_clear_color = video::SColor(0);
    m_clip = getFullscreenClip();
    os::Printer::log("WebGPU surface format", m_surface_format ==
        wgpu::TextureFormat::RGBA8Unorm ? "rgba8unorm" : "bgra8unorm");

    // Textures created below look up the driver through GE::getDriver()
    GE::setVideoDriver(this);
    createSamplers();
    GEWGPUShaderManager::init(m_device, io);
    createUnicolorTextures();
    GEWGPU2dRenderer::init(this);
    GEMaterialManager::init();
}   // GEWGPUDriver

// ----------------------------------------------------------------------------
GEWGPUDriver::~GEWGPUDriver()
{
}   // ~GEWGPUDriver

// ----------------------------------------------------------------------------
void GEWGPUDriver::destroyDriver()
{
    if (m_white_texture)
    {
        m_white_texture->drop();
        m_white_texture = NULL;
    }
    if (m_transparent_texture)
    {
        m_transparent_texture->drop();
        m_transparent_texture = NULL;
    }
    runPendingTasks();
    GEWGPU2dRenderer::destroy();
    GEWGPUShaderManager::destroy();
    for (wgpu::Sampler& s : m_samplers)
        s = nullptr;
    if (m_surface)
        m_surface.Unconfigure();
    m_surface = nullptr;
    m_queue = nullptr;
    m_device = nullptr;
}   // destroyDriver

// ----------------------------------------------------------------------------
void GEWGPUDriver::configureSurface()
{
    if (ScreenSize.Width == 0 || ScreenSize.Height == 0)
        return;
    wgpu::SurfaceConfiguration config;
    config.device = m_device;
    config.format = m_surface_format;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.width = ScreenSize.Width;
    config.height = ScreenSize.Height;
    config.alphaMode = wgpu::CompositeAlphaMode::Opaque;
    config.presentMode = wgpu::PresentMode::Fifo;
    m_surface.Configure(&config);
}   // configureSurface

// ----------------------------------------------------------------------------
void GEWGPUDriver::createSamplers()
{
    // Same settings as GEVulkanDriver::createSamplers
    wgpu::SamplerDescriptor desc;
    desc.addressModeU = desc.addressModeV = desc.addressModeW =
        wgpu::AddressMode::Repeat;
    desc.magFilter = desc.minFilter = wgpu::FilterMode::Nearest;
    desc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
    m_samplers[GVS_NEAREST] = m_device.CreateSampler(&desc);

    desc.magFilter = desc.minFilter = wgpu::FilterMode::Linear;
    desc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    desc.addressModeU = desc.addressModeV = desc.addressModeW =
        wgpu::AddressMode::ClampToEdge;
    m_samplers[GVS_SKYBOX] = m_device.CreateSampler(&desc);

    desc.addressModeU = desc.addressModeV = desc.addressModeW =
        wgpu::AddressMode::Repeat;
    desc.maxAnisotropy = 2;
    m_samplers[GVS_3D_MESH_MIPMAP_2] = m_device.CreateSampler(&desc);
    desc.maxAnisotropy = 4;
    m_samplers[GVS_3D_MESH_MIPMAP_4] = m_device.CreateSampler(&desc);
    desc.maxAnisotropy = 16;
    m_samplers[GVS_3D_MESH_MIPMAP_16] = m_device.CreateSampler(&desc);

    // Avoid artifacts when resizing down the screen
    desc.maxAnisotropy = 1;
    desc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
    desc.lodMaxClamp = 0.25f;
    m_samplers[GVS_2D_RENDER] = m_device.CreateSampler(&desc);

    desc.lodMaxClamp = 32.0f;
    desc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    desc.addressModeU = desc.addressModeV = desc.addressModeW =
        wgpu::AddressMode::ClampToEdge;
    desc.compare = wgpu::CompareFunction::LessEqual;
    m_samplers[GVS_SHADOW] = m_device.CreateSampler(&desc);
}   // createSamplers

// ----------------------------------------------------------------------------
void GEWGPUDriver::createUnicolorTextures()
{
    constexpr unsigned size = 2;
    std::array<uint8_t, size * size * 4> data;
    data.fill(255);
    // ownForeignMemory = false copies the data
    video::IImage* img = createImageFromData(video::ECF_A8R8G8B8,
        core::dimension2d<u32>(size, size), data.data(), false);
    m_white_texture = new GEWGPUTexture(img, "unicolor_white");
    data.fill(0);
    img = createImageFromData(video::ECF_A8R8G8B8,
        core::dimension2d<u32>(size, size), data.data(), false);
    m_transparent_texture = new GEWGPUTexture(img, "unicolor_transparent");
}   // createUnicolorTextures

// ----------------------------------------------------------------------------
video::ITexture* GEWGPUDriver::createDeviceDependentTexture(IImage* surface,
                                                            const io::path& name,
                                                            void* mipmapData)
{
    // GEWGPUTexture drops the image when uploaded
    surface->grab();
    return new GEWGPUTexture(surface, name.c_str());
}   // createDeviceDependentTexture

// ----------------------------------------------------------------------------
bool GEWGPUDriver::supportsTextureCompression() const
{
    return hasFeature(wgpu::FeatureName::TextureCompressionBC) ||
        hasFeature(wgpu::FeatureName::TextureCompressionASTC);
}   // supportsTextureCompression

// ----------------------------------------------------------------------------
void GEWGPUDriver::runOnMainThread(std::function<void()> f, const void* owner)
{
    if (emscripten_is_main_browser_thread())
    {
        f();
        return;
    }
    std::lock_guard<std::mutex> lock(m_tasks_mutex);
    m_pending_tasks.push_back({ std::move(f), owner });
}   // runOnMainThread

// ----------------------------------------------------------------------------
void GEWGPUDriver::cancelTasks(const void* owner)
{
    std::vector<PendingTask> cancelled;
    {
        std::lock_guard<std::mutex> lock(m_tasks_mutex);
        auto it = std::stable_partition(m_pending_tasks.begin(),
            m_pending_tasks.end(), [owner](const PendingTask& t)
            { return t.m_owner != owner; });
        cancelled.assign(std::make_move_iterator(it),
            std::make_move_iterator(m_pending_tasks.end()));
        m_pending_tasks.erase(it, m_pending_tasks.end());
    }
    // Captured objects are released outside the lock
}   // cancelTasks

// ----------------------------------------------------------------------------
void GEWGPUDriver::runPendingTasks()
{
    std::vector<PendingTask> tasks;
    {
        std::lock_guard<std::mutex> lock(m_tasks_mutex);
        tasks.swap(m_pending_tasks);
    }
    for (PendingTask& t : tasks)
        t.m_task();
}   // runPendingTasks

// ----------------------------------------------------------------------------
void GEWGPUDriver::onTextureDestroyed(const GEWGPUTexture* texture)
{
    cancelTasks(texture);
    if (emscripten_is_main_browser_thread())
        GEWGPU2dRenderer::onTextureDestroyed(texture);
    else
    {
        runOnMainThread([texture]()
            { GEWGPU2dRenderer::onTextureDestroyed(texture); });
    }
}   // onTextureDestroyed

// ----------------------------------------------------------------------------
void GEWGPUDriver::OnResize(const core::dimension2d<u32>& size)
{
    m_params.WindowSize = size;
    CNullDriver::OnResize(size);
    m_clip = getFullscreenClip();
    configureSurface();
}   // OnResize

// ----------------------------------------------------------------------------
bool GEWGPUDriver::beginScene(bool backBuffer, bool zBuffer, SColor color,
                              const SExposedVideoData& videoData,
                              core::rect<s32>* sourceRect)
{
    runPendingTasks();
    GEMaterialManager::update();
    if (!video::CNullDriver::beginScene(backBuffer, zBuffer, color, videoData,
        sourceRect))
        return false;
    m_clear_color = color;
    return true;
}   // beginScene

// ----------------------------------------------------------------------------
bool GEWGPUDriver::endScene()
{
    wgpu::SurfaceTexture surface_texture;
    m_surface.GetCurrentTexture(&surface_texture);
    if (surface_texture.status !=
        wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
        surface_texture.status !=
        wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal)
    {
        GEWGPU2dRenderer::clear();
        configureSurface();
        video::CNullDriver::endScene();
        return false;
    }

    wgpu::CommandEncoder encoder = m_device.CreateCommandEncoder();
    wgpu::RenderPassColorAttachment color;
    color.view = surface_texture.texture.CreateView();
    color.loadOp = wgpu::LoadOp::Clear;
    color.storeOp = wgpu::StoreOp::Store;
    video::SColorf cf(m_clear_color);
    color.clearValue = { cf.r, cf.g, cf.b, cf.a };
    wgpu::RenderPassDescriptor pass_desc;
    pass_desc.colorAttachmentCount = 1;
    pass_desc.colorAttachments = &color;
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&pass_desc);
    GEWGPU2dRenderer::render(pass, m_surface_format, ScreenSize);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    m_queue.Submit(1, &commands);
    GEWGPU2dRenderer::endFrame();
    // The browser presents the canvas when control returns to it
    return video::CNullDriver::endScene();
}   // endScene

// ----------------------------------------------------------------------------
bool GEWGPUDriver::setRenderTarget(video::ITexture* texture,
                                   bool clearBackBuffer, bool clearZBuffer,
                                   SColor color)
{
    // Render to texture is not implemented yet, draw into the screen
    return texture == NULL;
}   // setRenderTarget

// ----------------------------------------------------------------------------
void GEWGPUDriver::setViewPort(const core::rect<s32>& area)
{
    core::rect<s32> vp = area;
    core::rect<s32> rendert(0, 0, getCurrentRenderTargetSize().Width,
        getCurrentRenderTargetSize().Height);
    vp.clipAgainst(rendert);
    if (vp.getHeight() > 0 && vp.getWidth() > 0)
        ViewPort = vp;
}   // setViewPort

// ----------------------------------------------------------------------------
void GEWGPUDriver::updateDriver(bool scale_changed, bool pbr_changed,
                                bool ibl_changed)
{
    if (pbr_changed)
        reloadShaders();
}   // updateDriver

// ----------------------------------------------------------------------------
void GEWGPUDriver::reloadShaders()
{
    GEWGPU2dRenderer::destroy();
    GEWGPUShaderManager::reload();
    GEWGPU2dRenderer::init(this);
}   // reloadShaders

// ----------------------------------------------------------------------------
void GEWGPUDriver::draw2DVertexPrimitiveList(const void* vertices,
                                               u32 vertexCount,
                                               const void* indexList,
                                               u32 primitiveCount,
                                               E_VERTEX_TYPE vType,
                                               scene::E_PRIMITIVE_TYPE pType,
                                               E_INDEX_TYPE iType)
{
    const GEWGPUTexture* texture =
        dynamic_cast<const GEWGPUTexture*>(Material.getTexture(0));
    if (!texture)
        return;
    if (vType != EVT_STANDARD || iType != EIT_16BIT)
        return;
    if (pType == irr::scene::EPT_TRIANGLES)
    {
        S3DVertex* v = (S3DVertex*)vertices;
        u16* i = (u16*)indexList;
        GEWGPU2dRenderer::addVerticesIndices(v, vertexCount, i,
            primitiveCount, texture);
    }
    else if (pType == irr::scene::EPT_TRIANGLE_FAN)
    {
        std::vector<uint16_t> new_idx;
        uint16_t* idx = (uint16_t*)indexList;
        for (unsigned i = 0; i < primitiveCount; i++)
        {
            new_idx.push_back(idx[0]);
            new_idx.push_back(idx[i + 1]);
            new_idx.push_back(idx[i + 2]);
        }
        S3DVertex* v = (S3DVertex*)vertices;
        GEWGPU2dRenderer::addVerticesIndices(v, vertexCount, new_idx.data(),
            primitiveCount, texture);
    }
}   // draw2DVertexPrimitiveList

// ----------------------------------------------------------------------------
void GEWGPUDriver::draw2DImage(const video::ITexture* tex,
                                 const core::position2d<s32>& destPos,
                                 const core::rect<s32>& sourceRect,
                                 const core::rect<s32>* clipRect,
                                 SColor color, bool useAlphaChannelOfTexture)
{
    const GEWGPUTexture* texture = dynamic_cast<const GEWGPUTexture*>(tex);
    if (!texture)
        return;

    if (!sourceRect.isValid())
        return;

    core::position2d<s32> targetPos = destPos;
    core::position2d<s32> sourcePos = sourceRect.UpperLeftCorner;
    // This needs to be signed as it may go negative.
    core::dimension2d<s32> sourceSize(sourceRect.getSize());

    if (clipRect)
    {
        if (targetPos.X < clipRect->UpperLeftCorner.X)
        {
            sourceSize.Width += targetPos.X - clipRect->UpperLeftCorner.X;
            if (sourceSize.Width <= 0)
                return;

            sourcePos.X -= targetPos.X - clipRect->UpperLeftCorner.X;
            targetPos.X = clipRect->UpperLeftCorner.X;
        }

        if (targetPos.X + (s32)sourceSize.Width > clipRect->LowerRightCorner.X)
        {
            sourceSize.Width -= (targetPos.X + sourceSize.Width) - clipRect->LowerRightCorner.X;
            if (sourceSize.Width <= 0)
                return;
        }

        if (targetPos.Y < clipRect->UpperLeftCorner.Y)
        {
            sourceSize.Height += targetPos.Y - clipRect->UpperLeftCorner.Y;
            if (sourceSize.Height <= 0)
                return;

            sourcePos.Y -= targetPos.Y - clipRect->UpperLeftCorner.Y;
            targetPos.Y = clipRect->UpperLeftCorner.Y;
        }

        if (targetPos.Y + (s32)sourceSize.Height > clipRect->LowerRightCorner.Y)
        {
            sourceSize.Height -= (targetPos.Y + sourceSize.Height) - clipRect->LowerRightCorner.Y;
            if (sourceSize.Height <= 0)
                return;
        }
    }

    // clip these coordinates

    if (targetPos.X<0)
    {
        sourceSize.Width += targetPos.X;
        if (sourceSize.Width <= 0)
            return;

        sourcePos.X -= targetPos.X;
        targetPos.X = 0;
    }

    const core::dimension2d<u32>& renderTargetSize = getCurrentRenderTargetSize();

    if (targetPos.X + sourceSize.Width > (s32)renderTargetSize.Width)
    {
        sourceSize.Width -= (targetPos.X + sourceSize.Width) - renderTargetSize.Width;
        if (sourceSize.Width <= 0)
            return;
    }

    if (targetPos.Y<0)
    {
        sourceSize.Height += targetPos.Y;
        if (sourceSize.Height <= 0)
            return;

        sourcePos.Y -= targetPos.Y;
        targetPos.Y = 0;
    }

    if (targetPos.Y + sourceSize.Height > (s32)renderTargetSize.Height)
    {
        sourceSize.Height -= (targetPos.Y + sourceSize.Height) - renderTargetSize.Height;
        if (sourceSize.Height <= 0)
            return;
    }

    // ok, we've clipped everything.
    // now draw it.

    core::rect<f32> tcoords;
    tcoords.UpperLeftCorner.X = (((f32)sourcePos.X)) / texture->getSize().Width ;
    tcoords.UpperLeftCorner.Y = (((f32)sourcePos.Y)) / texture->getSize().Height;
    tcoords.LowerRightCorner.X = tcoords.UpperLeftCorner.X + ((f32)(sourceSize.Width) / texture->getSize().Width);
    tcoords.LowerRightCorner.Y = tcoords.UpperLeftCorner.Y + ((f32)(sourceSize.Height) / texture->getSize().Height);

    const core::rect<s32> poss(targetPos, sourceSize);

    S3DVertex vtx[4];
    vtx[0] = S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.UpperLeftCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, color,
            tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y);
    vtx[1] = S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.UpperLeftCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, color,
            tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y);
    vtx[2] = S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.LowerRightCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, color,
            tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y);
    vtx[3] = S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.LowerRightCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, color,
            tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y);

    u16 indices[6] = {0,1,2,0,2,3};

    if (clipRect)
        enableScissorTest(*clipRect);
    GEWGPU2dRenderer::addVerticesIndices(&vtx[0], 4, &indices[0], 2, texture);
    if (clipRect)
        disableScissorTest();
}   // draw2DImage

// ----------------------------------------------------------------------------
void GEWGPUDriver::draw2DImage(const video::ITexture* tex,
                                 const core::rect<s32>& destRect,
                                 const core::rect<s32>& sourceRect,
                                 const core::rect<s32>* clipRect,
                                 const video::SColor* const colors,
                                 bool useAlphaChannelOfTexture)
{
    const GEWGPUTexture* texture = dynamic_cast<const GEWGPUTexture*>(tex);
    if (!texture)
        return;

    const core::dimension2d<u32>& ss = texture->getSize();
    core::rect<f32> tcoords;
    tcoords.UpperLeftCorner.X = (f32)sourceRect.UpperLeftCorner.X / (f32)ss.Width;
    tcoords.UpperLeftCorner.Y = (f32)sourceRect.UpperLeftCorner.Y / (f32)ss.Height;
    tcoords.LowerRightCorner.X = (f32)sourceRect.LowerRightCorner.X / (f32)ss.Width;
    tcoords.LowerRightCorner.Y = (f32)sourceRect.LowerRightCorner.Y / (f32)ss.Height;

    const core::dimension2d<u32>& renderTargetSize = getCurrentRenderTargetSize();

    const video::SColor temp[4] =
    {
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF
    };

    const video::SColor* const useColor = colors ? colors : temp;

    S3DVertex vtx[4];
    vtx[0] = S3DVertex((f32)destRect.UpperLeftCorner.X, (f32)destRect.UpperLeftCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, useColor[0],
            tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y);
    vtx[1] = S3DVertex((f32)destRect.LowerRightCorner.X, (f32)destRect.UpperLeftCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, useColor[3],
            tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y);
    vtx[2] = S3DVertex((f32)destRect.LowerRightCorner.X, (f32)destRect.LowerRightCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, useColor[2],
            tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y);
    vtx[3] = S3DVertex((f32)destRect.UpperLeftCorner.X, (f32)destRect.LowerRightCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, useColor[1],
            tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y);

    u16 indices[6] = {0,1,2,0,2,3};

    if (clipRect)
        enableScissorTest(*clipRect);
    GEWGPU2dRenderer::addVerticesIndices(&vtx[0], 4, &indices[0], 2, texture);
    if (clipRect)
        disableScissorTest();
}   // draw2DImage

// ----------------------------------------------------------------------------
void GEWGPUDriver::draw2DImageBatch(const video::ITexture* tex,
                          const core::array<core::position2d<s32> >& positions,
                          const core::array<core::rect<s32> >& sourceRects,
                          const core::rect<s32>* clipRect, SColor color,
                          bool useAlphaChannelOfTexture)
{
    const GEWGPUTexture* texture = dynamic_cast<const GEWGPUTexture*>(tex);
    if (!texture)
        return;

    const irr::u32 drawCount = core::min_<u32>(positions.size(), sourceRects.size());

    core::array<S3DVertex> vtx(drawCount * 4);
    core::array<u16> indices(drawCount * 6);

    for(u32 i = 0;i < drawCount;i++)
    {
        core::position2d<s32> targetPos = positions[i];
        core::position2d<s32> sourcePos = sourceRects[i].UpperLeftCorner;
        // This needs to be signed as it may go negative.
        core::dimension2d<s32> sourceSize(sourceRects[i].getSize());

        if (clipRect)
        {
            if (targetPos.X < clipRect->UpperLeftCorner.X)
            {
                sourceSize.Width += targetPos.X - clipRect->UpperLeftCorner.X;
                if (sourceSize.Width <= 0)
                    continue;

                sourcePos.X -= targetPos.X - clipRect->UpperLeftCorner.X;
                targetPos.X = clipRect->UpperLeftCorner.X;
            }

            if (targetPos.X + (s32)sourceSize.Width > clipRect->LowerRightCorner.X)
            {
                sourceSize.Width -= (targetPos.X + sourceSize.Width) - clipRect->LowerRightCorner.X;
                if (sourceSize.Width <= 0)
                    continue;
            }

            if (targetPos.Y < clipRect->UpperLeftCorner.Y)
            {
                sourceSize.Height += targetPos.Y - clipRect->UpperLeftCorner.Y;
                if (sourceSize.Height <= 0)
                    continue;

                sourcePos.Y -= targetPos.Y - clipRect->UpperLeftCorner.Y;
                targetPos.Y = clipRect->UpperLeftCorner.Y;
            }

            if (targetPos.Y + (s32)sourceSize.Height > clipRect->LowerRightCorner.Y)
            {
                sourceSize.Height -= (targetPos.Y + sourceSize.Height) - clipRect->LowerRightCorner.Y;
                if (sourceSize.Height <= 0)
                    continue;
            }
        }

        // clip these coordinates

        if (targetPos.X<0)
        {
            sourceSize.Width += targetPos.X;
            if (sourceSize.Width <= 0)
                continue;

            sourcePos.X -= targetPos.X;
            targetPos.X = 0;
        }

        const core::dimension2d<u32>& renderTargetSize = getCurrentRenderTargetSize();

        if (targetPos.X + sourceSize.Width > (s32)renderTargetSize.Width)
        {
            sourceSize.Width -= (targetPos.X + sourceSize.Width) - renderTargetSize.Width;
            if (sourceSize.Width <= 0)
                continue;
        }

        if (targetPos.Y<0)
        {
            sourceSize.Height += targetPos.Y;
            if (sourceSize.Height <= 0)
                continue;

            sourcePos.Y -= targetPos.Y;
            targetPos.Y = 0;
        }

        if (targetPos.Y + sourceSize.Height > (s32)renderTargetSize.Height)
        {
            sourceSize.Height -= (targetPos.Y + sourceSize.Height) - renderTargetSize.Height;
            if (sourceSize.Height <= 0)
                continue;
        }

        // ok, we've clipped everything.
        // now draw it.

        core::rect<f32> tcoords;
        tcoords.UpperLeftCorner.X = (((f32)sourcePos.X)) / texture->getSize().Width ;
        tcoords.UpperLeftCorner.Y = (((f32)sourcePos.Y)) / texture->getSize().Height;
        tcoords.LowerRightCorner.X = tcoords.UpperLeftCorner.X + ((f32)(sourceSize.Width) / texture->getSize().Width);
        tcoords.LowerRightCorner.Y = tcoords.UpperLeftCorner.Y + ((f32)(sourceSize.Height) / texture->getSize().Height);

        const core::rect<s32> poss(targetPos, sourceSize);

        vtx.push_back(S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.UpperLeftCorner.Y, 0.0f,
                0.0f, 0.0f, 0.0f, color,
                tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y));
        vtx.push_back(S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.UpperLeftCorner.Y, 0.0f,
                0.0f, 0.0f, 0.0f, color,
                tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y));
        vtx.push_back(S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.LowerRightCorner.Y, 0.0f,
                0.0f, 0.0f, 0.0f, color,
                tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y));
        vtx.push_back(S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.LowerRightCorner.Y, 0.0f,
                0.0f, 0.0f, 0.0f, color,
                tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y));

        const u32 curPos = vtx.size()-4;
        indices.push_back(0+curPos);
        indices.push_back(1+curPos);
        indices.push_back(2+curPos);

        indices.push_back(0+curPos);
        indices.push_back(2+curPos);
        indices.push_back(3+curPos);
    }

    if (vtx.size())
    {
        if (clipRect)
            enableScissorTest(*clipRect);
        GEWGPU2dRenderer::addVerticesIndices(vtx.pointer(), vtx.size(),
            indices.pointer(), indices.size() / 3, texture);
        if (clipRect)
            disableScissorTest();
    }
}   // draw2DImageBatch


}

namespace irr
{
namespace video
{
    IVideoDriver* createWebGPUDriver(const SIrrlichtCreationParameters& params,
                                     io::IFileSystem* io, SDL_Window* window,
                                     IrrlichtDevice* device)
    {
        return new GE::GEWGPUDriver(params, io, window, device);
    }   // createWebGPUDriver
}
}

#endif // _IRR_COMPILE_WITH_WEBGPU_
