#include "ge_wgpu_texture.hpp"

#include "ge_main.hpp"
#include "ge_mipmap_generator.hpp"
#include "ge_texture.hpp"
#include "ge_wgpu_driver.hpp"

#include <IAttributes.h>
#include <IFileSystem.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <vector>

namespace GE
{
// ----------------------------------------------------------------------------
GEWGPUTexture::GEWGPUTexture(const std::string& name, bool single_channel)
             : video::ITexture(name.c_str()), m_image_mani(nullptr),
               m_locked_data(NULL), m_format(wgpu::TextureFormat::BGRA8Unorm),
               m_disable_reload(true), m_single_channel(single_channel),
               m_has_mipmaps(true), m_driver(getWGPUDriver()),
               m_texture_size(0)
{
}   // GEWGPUTexture

// ----------------------------------------------------------------------------
GEWGPUTexture::GEWGPUTexture(const std::string& path,
                             std::function<void(video::IImage*)> image_mani)
             : video::ITexture(path.c_str()), m_image_mani(image_mani),
               m_locked_data(NULL), m_format(wgpu::TextureFormat::BGRA8Unorm),
               m_disable_reload(false), m_single_channel(false),
               m_has_mipmaps(true), m_driver(getWGPUDriver()),
               m_texture_size(0)
{
    m_full_path = getDriver()->getFileSystem()->getAbsolutePath(NamedPath);
    if (!getDriver()->getFileSystem()->existFileOnly(m_full_path))
    {
        LoadingFailed = true;
        return;
    }
    loadFromFile();
}   // GEWGPUTexture

// ----------------------------------------------------------------------------
GEWGPUTexture::GEWGPUTexture(video::IImage* img, const std::string& name)
             : GEWGPUTexture(name, false/*single_channel*/)
{
    if (!img)
    {
        LoadingFailed = true;
        return;
    }
    upload(img);
}   // GEWGPUTexture

// ----------------------------------------------------------------------------
GEWGPUTexture::GEWGPUTexture(const std::string& name, unsigned int size,
                             bool single_channel)
             : GEWGPUTexture(name, single_channel)
{
    // Glyph pages are updated in place, mipmaps would need regenerating on
    // the GPU after every update
    m_has_mipmaps = false;
    m_size = m_orig_size = core::dimension2du(size, size);
    std::shared_ptr<std::vector<uint8_t> > data =
        std::make_shared<std::vector<uint8_t> >(size * size * 4, 0);
    if (m_single_channel)
    {
        // White with zero alpha, so glyphs blend correctly at the edges
        for (unsigned i = 0; i < size * size; i++)
            memset(&(*data)[i * 4], 255, 3);
    }
    m_driver->runOnMainThread([this, data]()
        {
            createTexture(wgpu::TextureFormat::BGRA8Unorm,
                wgpu::TextureUsage::TextureBinding |
                wgpu::TextureUsage::CopyDst);
            uploadLevels(data->data(), 4);
        }, this);
}   // GEWGPUTexture

// ----------------------------------------------------------------------------
GEWGPUTexture::~GEWGPUTexture()
{
    unlock();
    // Also cancels pending uploads that reference this texture
    m_driver->onTextureDestroyed(this);
    clearGPUData();
}   // ~GEWGPUTexture

// ----------------------------------------------------------------------------
void GEWGPUTexture::clearGPUData()
{
    if (!m_texture)
        return;
    // Release on the main thread, which owns all WebGPU objects
    auto texture = std::make_shared<wgpu::Texture>(std::move(m_texture));
    auto view = std::make_shared<wgpu::TextureView>(std::move(m_view));
    auto view_srgb =
        std::make_shared<wgpu::TextureView>(std::move(m_view_srgb));
    m_texture = nullptr;
    m_view = nullptr;
    m_view_srgb = nullptr;
    m_driver->runOnMainThread([texture, view, view_srgb]()
        {
            texture->Destroy();
        });
}   // clearGPUData

// ----------------------------------------------------------------------------
void GEWGPUTexture::loadFromFile()
{
    core::dimension2du max_size = getDriver()->getDriverAttributes()
        .getAttributeAsDimension2d("MAX_TEXTURE_SIZE");
    video::IImage* image = getResizedImageFullPath(m_full_path, max_size,
        &m_orig_size);
    if (image == NULL)
    {
        LoadingFailed = true;
        return;
    }
    if (m_image_mani)
        m_image_mani(image);
    upload(image);
}   // loadFromFile

// ----------------------------------------------------------------------------
/** Takes ownership of the image (drops it). */
void GEWGPUTexture::upload(video::IImage* image)
{
    m_size = image->getDimension();
    if (m_orig_size.Width == 0)
        m_orig_size = m_size;
    m_has_mipmaps = m_size.Width >= 4 && m_size.Height >= 4;
    // Dropped when the task runs or is cancelled
    std::shared_ptr<video::IImage> img(image,
        [](video::IImage* i) { i->drop(); });
    m_driver->runOnMainThread([this, img]()
        {
            createTexture(wgpu::TextureFormat::BGRA8Unorm,
                wgpu::TextureUsage::TextureBinding |
                wgpu::TextureUsage::CopyDst);
            uploadLevels((const uint8_t*)img->lock(), 4);
            img->unlock();
        }, this);
}   // upload

// ----------------------------------------------------------------------------
unsigned GEWGPUTexture::getMipmapLevels() const
{
    if (!m_has_mipmaps)
        return 1;
    return std::floor(std::log2(std::max(m_size.Width, m_size.Height))) + 1;
}   // getMipmapLevels

// ----------------------------------------------------------------------------
void GEWGPUTexture::createTexture(wgpu::TextureFormat format,
                                  wgpu::TextureUsage usage)
{
    m_format = format;
    wgpu::TextureFormat srgb = wgpu::TextureFormat::BGRA8UnormSrgb;
    wgpu::TextureDescriptor desc;
    desc.label = wgpu::StringView(NamedPath.getPtr());
    desc.size = { m_size.Width, m_size.Height, 1 };
    desc.mipLevelCount = getMipmapLevels();
    desc.format = format;
    desc.usage = usage;
    desc.viewFormatCount = 1;
    desc.viewFormats = &srgb;
    m_texture = m_driver->getDevice().CreateTexture(&desc);
    m_view = m_texture.CreateView();
    wgpu::TextureViewDescriptor srgb_desc;
    srgb_desc.format = srgb;
    m_view_srgb = m_texture.CreateView(&srgb_desc);

    m_texture_size = 0;
    unsigned w = m_size.Width, h = m_size.Height;
    for (unsigned i = 0; i < desc.mipLevelCount; i++)
    {
        m_texture_size += w * h * 4;
        w = std::max(w / 2, 1u);
        h = std::max(h / 2, 1u);
    }
}   // createTexture

// ----------------------------------------------------------------------------
void GEWGPUTexture::uploadLevels(const uint8_t* data, unsigned channels)
{
    const wgpu::Queue& queue = m_driver->getQueue();
    wgpu::TexelCopyTextureInfo dst;
    dst.texture = m_texture;
    if (!m_has_mipmaps)
    {
        wgpu::TexelCopyBufferLayout layout;
        layout.bytesPerRow = m_size.Width * channels;
        layout.rowsPerImage = m_size.Height;
        wgpu::Extent3D extent = { m_size.Width, m_size.Height, 1 };
        queue.WriteTexture(&dst, data, m_size.Width * m_size.Height * channels,
            &layout, &extent);
        return;
    }
    const bool normal_map = (std::string(NamedPath.getPtr()).find(
        "_Normal.") != std::string::npos);
    GEMipmapGenerator generator((uint8_t*)data, channels, m_size, normal_map);
    std::vector<GEImageLevel>& levels = generator.getAllLevels();
    for (unsigned i = 0; i < levels.size(); i++)
    {
        const GEImageLevel& level = levels[i];
        dst.mipLevel = i;
        wgpu::TexelCopyBufferLayout layout;
        layout.bytesPerRow = level.m_dim.Width * channels;
        layout.rowsPerImage = level.m_dim.Height;
        wgpu::Extent3D extent = { level.m_dim.Width, level.m_dim.Height, 1 };
        queue.WriteTexture(&dst, level.m_data, level.m_size, &layout,
            &extent);
    }
}   // uploadLevels

// ----------------------------------------------------------------------------
const wgpu::TextureView& GEWGPUTexture::getView(bool srgb) const
{
    if (!m_view)
    {
        return static_cast<GEWGPUTexture*>(m_driver->getTransparentTexture())
            ->m_view;
    }
    return srgb ? m_view_srgb : m_view;
}   // getView

// ----------------------------------------------------------------------------
/** Reading back from the GPU needs an asynchronous map, which cannot be waited
 *  for on the browser main thread, so only file backed textures (decoded
 *  again) can be locked. */
void* GEWGPUTexture::lock(video::E_TEXTURE_LOCK_MODE mode, u32 mipmap_level)
{
    if (m_full_path.empty())
        return NULL;
    video::IImage* image = getResizedImageFullPath(m_full_path,
        getDriver()->getDriverAttributes()
        .getAttributeAsDimension2d("MAX_TEXTURE_SIZE"), NULL, &m_size);
    if (image == NULL)
        return NULL;
    if (m_image_mani)
        m_image_mani(image);
    unlock();
    size_t size = m_size.Width * m_size.Height * 4;
    m_locked_data = new uint8_t[size];
    memcpy(m_locked_data, image->lock(), size);
    image->unlock();
    image->drop();
    return m_locked_data;
}   // lock

// ----------------------------------------------------------------------------
void GEWGPUTexture::unlock()
{
    delete [] m_locked_data;
    m_locked_data = NULL;
}   // unlock

// ----------------------------------------------------------------------------
void GEWGPUTexture::reload()
{
    if (m_disable_reload)
        return;
    m_driver->onTextureDestroyed(this);
    clearGPUData();
    loadFromFile();
}   // reload

// ----------------------------------------------------------------------------
void GEWGPUTexture::updateTexture(void* data, video::ECOLOR_FORMAT format,
                                  u32 w, u32 h, u32 x, u32 y)
{
    auto bgra = std::make_shared<std::vector<uint8_t> >(w * h * 4);
    const uint8_t* src = (const uint8_t*)data;
    if (format == video::ECF_R8)
    {
        for (unsigned i = 0; i < w * h; i++)
        {
            (*bgra)[i * 4] = (*bgra)[i * 4 + 1] = (*bgra)[i * 4 + 2] = 255;
            (*bgra)[i * 4 + 3] = src[i];
        }
    }
    else if (format == video::ECF_A8R8G8B8)
        memcpy(bgra->data(), src, w * h * 4);
    else
        return;

    m_driver->runOnMainThread([this, bgra, w, h, x, y]()
        {
            wgpu::TexelCopyTextureInfo dst;
            dst.texture = m_texture;
            dst.origin = { x, y, 0 };
            wgpu::TexelCopyBufferLayout layout;
            layout.bytesPerRow = w * 4;
            layout.rowsPerImage = h;
            wgpu::Extent3D extent = { w, h, 1 };
            m_driver->getQueue().WriteTexture(&dst, bgra->data(),
                bgra->size(), &layout, &extent);
        }, this);
}   // updateTexture

}
