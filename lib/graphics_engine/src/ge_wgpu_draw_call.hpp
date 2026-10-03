#ifndef HEADER_GE_WGPU_DRAW_CALL_HPP
#define HEADER_GE_WGPU_DRAW_CALL_HPP

#include "ge_object_data.hpp"

#include "ESceneNodeTypes.h"
#include "IrrCompileConfig.h"
#include "SMaterial.h"
#include "matrix4.h"
#include "vector3d.h"

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace irr
{
    namespace scene { class ISceneNode; class IMesh; }
    namespace video { class ITexture; }
}

namespace GE
{
class GECullingTool;
class GESPMBuffer;
class GEVulkanAnimatedMeshSceneNode;
class GEWGPUCameraSceneNode;
class GEWGPUDynamicSPMBuffer;
class GEWGPUSkyBoxRenderer;

enum GEWGPUPassType : unsigned
{
    GWPT_SOLID,
    // Depth of ghost (transparent) karts, so only their front faces show
    GWPT_GHOST_DEPTH,
    GWPT_TRANSPARENT,
    GWPT_COUNT
};

/** Collects the visible nodes of one camera and renders them, following
 *  GEVulkanDrawCall in its non-bindless mode: instanced draws from the
 *  shared mesh buffer, per instance data in a storage buffer indexed by
 *  instance_index (with firstInstance), and one bind group of textures per
 *  material. */
class GEWGPUDrawCall
{
public:
    typedef std::array<const irr::video::ITexture*,
        _IRR_MATERIAL_MAX_TEXTURES_> TexturesList;
private:
    struct DrawCmd
    {
        std::string m_shader;
        std::string m_sorting_key;
        TexturesList m_textures;
        bool m_skinning;
        // NULL for buffers in the mesh cache
        GEWGPUDynamicSPMBuffer* m_dynamic;
        uint32_t m_index_count;
        uint32_t m_instance_count;
        uint32_t m_first_index;
        int32_t m_base_vertex;
        uint32_t m_first_instance;
    };

    static constexpr int BILLBOARD_NODE = -1;
    static constexpr int PARTICLE_NODE = -2;

    std::map<std::pair<GESPMBuffer*, TexturesList>, std::unordered_map<
        std::string, std::vector<std::pair<irr::scene::ISceneNode*, int> > > >
        m_visible_nodes;

    std::vector<std::pair<GEWGPUDynamicSPMBuffer*, irr::scene::ISceneNode*> >
        m_dynamic_spm_buffers;

    std::unordered_set<GEVulkanAnimatedMeshSceneNode*> m_skinning_nodes;

    std::map<TexturesList, GESPMBuffer*> m_billboard_buffers;

    std::unique_ptr<GECullingTool> m_culling_tool;

    GEWGPUSkyBoxRenderer* m_skybox_renderer;

    irr::core::vector3df m_view_position;

    btQuaternion m_billboard_rotation;

    GEWGPUCameraSceneNode* m_camera;

    std::vector<DrawCmd> m_cmds;

    std::vector<ObjectData> m_objects;

    std::vector<irr::core::matrix4> m_skinning;

    // Offsets into m_push_constants_buffer per shader
    std::unordered_map<std::string, uint32_t> m_push_constants_offsets;

    wgpu::Buffer m_camera_buffer, m_object_buffer, m_skinning_buffer,
        m_push_constants_buffer;

    uint64_t m_object_buffer_size, m_skinning_buffer_size,
        m_push_constants_buffer_size;

    wgpu::BindGroup m_data_bind_group;

    // ------------------------------------------------------------------------
    std::string getShader(const irr::video::SMaterial& m) const;
    // ------------------------------------------------------------------------
    bool ensureBuffer(wgpu::Buffer& buffer, uint64_t& size, uint64_t needed,
                      wgpu::BufferUsage usage, const char* label);
    // ------------------------------------------------------------------------
    void renderPass(wgpu::RenderPassEncoder& pass, GEWGPUPassType pt,
                    wgpu::TextureFormat color_format);
public:
    // ------------------------------------------------------------------------
    GEWGPUDrawCall();
    // ------------------------------------------------------------------------
    ~GEWGPUDrawCall();
    // ------------------------------------------------------------------------
    void prepare(GEWGPUCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    void addNode(irr::scene::ISceneNode* node);
    // ------------------------------------------------------------------------
    void addBillboardNode(irr::scene::ISceneNode* node,
                          irr::scene::ESCENE_NODE_TYPE node_type);
    // ------------------------------------------------------------------------
    void addSkyBox(irr::scene::ISceneNode* node);
    // ------------------------------------------------------------------------
    void generate();
    // ------------------------------------------------------------------------
    /** Queues the buffer writes of this frame, before the encoder is
     *  submitted. */
    void upload();
    // ------------------------------------------------------------------------
    void render(wgpu::RenderPassEncoder& pass,
                wgpu::TextureFormat color_format);
    // ------------------------------------------------------------------------
    GEWGPUCameraSceneNode* getCamera() const               { return m_camera; }
    // ------------------------------------------------------------------------
    unsigned getPolyCount() const
    {
        unsigned result = 0;
        for (const DrawCmd& cmd : m_cmds)
            result += (cmd.m_index_count / 3) * cmd.m_instance_count;
        return result;
    }
    // ------------------------------------------------------------------------
    void reset();
    // ------------------------------------------------------------------------
    /** Pipelines and material bind groups are shared by all draw calls. */
    static void destroyShared();
    // ------------------------------------------------------------------------
    static void onTextureDestroyed();
};   // GEWGPUDrawCall

}

#endif
