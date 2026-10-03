#ifndef HEADER_GE_WGPU_TEXTURE_HPP
#define HEADER_GE_WGPU_TEXTURE_HPP

#include <webgpu/webgpu_cpp.h>

#include <functional>
#include <string>
#include <ITexture.h>
#include <IImage.h>

using namespace irr;

namespace GE
{
class GEWGPUDriver;

/** Sampled 2D texture for the WebGPU driver. Pixels are kept in Irrlicht's
 *  native BGRA order (so no per-pixel swizzle is needed on upload), with an
 *  additional sRGB view for the PBR pipeline. WebGPU objects may only be
 *  touched on the browser main thread, so uploads requested from other
 *  threads are deferred to it and a placeholder is sampled meanwhile. */
class GEWGPUTexture : public video::ITexture
{
protected:
    core::dimension2d<u32> m_size, m_orig_size;

    std::function<void(video::IImage*)> m_image_mani;

    uint8_t* m_locked_data;

    wgpu::Texture m_texture;

    wgpu::TextureView m_view, m_view_srgb;

    wgpu::TextureFormat m_format;

    const bool m_disable_reload;

    /** True for glyph pages: data is supplied as one channel and stored as
     *  alpha with white color, as WebGPU has no component swizzle. */
    const bool m_single_channel;

    bool m_has_mipmaps;

    io::path m_full_path;

    GEWGPUDriver* m_driver;

    unsigned m_texture_size;

    // ------------------------------------------------------------------------
    void upload(video::IImage* image);
    // ------------------------------------------------------------------------
    void createTexture(wgpu::TextureFormat format, wgpu::TextureUsage usage);
    // ------------------------------------------------------------------------
    void uploadLevels(const uint8_t* data, unsigned channels);
    // ------------------------------------------------------------------------
    void clearGPUData();
    // ------------------------------------------------------------------------
    void loadFromFile();
    // ------------------------------------------------------------------------
    GEWGPUTexture(const std::string& name, bool single_channel);
public:
    // ------------------------------------------------------------------------
    GEWGPUTexture(const std::string& path,
                  std::function<void(video::IImage*)> image_mani = nullptr);
    // ------------------------------------------------------------------------
    GEWGPUTexture(video::IImage* img, const std::string& name);
    // ------------------------------------------------------------------------
    GEWGPUTexture(const std::string& name, unsigned int size,
                  bool single_channel);
    // ------------------------------------------------------------------------
    virtual ~GEWGPUTexture();
    // ------------------------------------------------------------------------
    virtual void* lock(video::E_TEXTURE_LOCK_MODE mode =
                       video::ETLM_READ_WRITE, u32 mipmap_level = 0);
    // ------------------------------------------------------------------------
    virtual void unlock();
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getOriginalSize() const
                                                       { return m_orig_size; }
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getSize() const   { return m_size; }
    // ------------------------------------------------------------------------
    virtual video::E_DRIVER_TYPE getDriverType() const
                                                  { return video::EDT_WEBGPU; }
    // ------------------------------------------------------------------------
    virtual video::ECOLOR_FORMAT getColorFormat() const
                                                { return video::ECF_A8R8G8B8; }
    // ------------------------------------------------------------------------
    virtual u32 getPitch() const                                  { return 0; }
    // ------------------------------------------------------------------------
    virtual bool hasMipMaps() const                   { return m_has_mipmaps; }
    // ------------------------------------------------------------------------
    virtual void regenerateMipMapLevels(void* mipmap_data = NULL)            {}
    // ------------------------------------------------------------------------
    virtual u64 getTextureHandler() const
                                        { return (u64)(uintptr_t)this; }
    // ------------------------------------------------------------------------
    virtual unsigned int getTextureSize() const     { return m_texture_size; }
    // ------------------------------------------------------------------------
    virtual void reload();
    // ------------------------------------------------------------------------
    virtual void updateTexture(void* data, irr::video::ECOLOR_FORMAT format,
                               u32 w, u32 h, u32 x, u32 y);
    // ------------------------------------------------------------------------
    virtual const io::path& getFullPath() const         { return m_full_path; }
    // ------------------------------------------------------------------------
    /** Returns the view to sample, or the driver placeholder if the texture is
     *  not uploaded yet. */
    const wgpu::TextureView& getView(bool srgb = false) const;
    // ------------------------------------------------------------------------
    bool isReady() const                         { return m_view != nullptr; }
    // ------------------------------------------------------------------------
    const wgpu::Texture& getTexture() const              { return m_texture; }
    // ------------------------------------------------------------------------
    unsigned getMipmapLevels() const;
};   // GEWGPUTexture

}

#endif
