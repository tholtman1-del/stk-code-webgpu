#ifndef HEADER_GE_WGPU_TEXTURE_LOADER_HPP
#define HEADER_GE_WGPU_TEXTURE_LOADER_HPP

#include <functional>

namespace GE
{
/** Threads decoding textures (image decoding, resizing and mipmaps), as the
 *  loader threads of GEVulkanCommandLoader. Uploads still happen on the main
 *  thread. */
namespace GEWGPUTextureLoader
{
// ----------------------------------------------------------------------------
void init();
// ----------------------------------------------------------------------------
void destroy();
// ----------------------------------------------------------------------------
/** Queues task, or runs it now without loader threads. */
void add(const void* owner, std::function<void()> task);
// ----------------------------------------------------------------------------
/** Runs the queued task of owner on the calling thread if no loader thread
 *  took it yet, so a thread waiting for it does not depend on the loaders
 *  being started. Returns false if there was none. */
bool runNow(const void* owner);
}

}

#endif
