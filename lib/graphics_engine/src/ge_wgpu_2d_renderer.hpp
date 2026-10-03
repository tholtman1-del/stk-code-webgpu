#ifndef HEADER_GE_WGPU_2D_RENDERER_HPP
#define HEADER_GE_WGPU_2D_RENDERER_HPP

#include <webgpu/webgpu_cpp.h>

#include "ITexture.h"
#include "S3DVertex.h"

namespace GE
{
class GEWGPUDriver;
class GEWGPUTexture;
/** Batches all 2D draws (GUI, fonts, the 3D scene's final image) of a frame
 *  and renders them in submission order into the current output. */
namespace GEWGPU2dRenderer
{
// ----------------------------------------------------------------------------
void init(GEWGPUDriver* driver);
// ----------------------------------------------------------------------------
void destroy();
// ----------------------------------------------------------------------------
/** Uploads the queued triangles and records them into a render pass. */
void render(wgpu::RenderPassEncoder& pass, wgpu::TextureFormat format,
            const irr::core::dimension2du& target_size);
// ----------------------------------------------------------------------------
/** Called after the frame was submitted, buffers may be reused. */
void endFrame();
// ----------------------------------------------------------------------------
void clear();
// ----------------------------------------------------------------------------
bool empty();
// ----------------------------------------------------------------------------
void onTextureDestroyed(const GEWGPUTexture* t);
// ----------------------------------------------------------------------------
void addVerticesIndices(irr::video::S3DVertex* vertices,
                        unsigned vertices_count, uint16_t* indices,
                        unsigned indices_count,
                        const irr::video::ITexture* t);
};   // GEWGPU2dRenderer

}

#endif
