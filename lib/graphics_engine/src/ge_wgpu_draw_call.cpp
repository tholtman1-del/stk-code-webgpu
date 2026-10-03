#include "ge_wgpu_draw_call.hpp"

#include "ge_culling_tool.hpp"
#include "ge_main.hpp"
#include "ge_material_manager.hpp"
#include "ge_render_info.hpp"
#include "ge_spm.hpp"
#include "ge_spm_buffer.hpp"
#include "ge_vulkan_animated_mesh_scene_node.hpp"
#include "ge_vulkan_light_handler.hpp"
#include "ge_wgpu_camera_scene_node.hpp"
#include "ge_wgpu_driver.hpp"
#include "ge_wgpu_dynamic_spm_buffer.hpp"
#include "ge_wgpu_mesh_cache.hpp"
#include "ge_wgpu_scene_manager.hpp"
#include "ge_wgpu_shader_manager.hpp"
#include "ge_wgpu_skybox_renderer.hpp"
#include "ge_wgpu_texture.hpp"

#include "mini_glm.hpp"
#include "IBillboardSceneNode.h"
#include "ILightSceneNode.h"
#include "IMeshSceneNode.h"
#include "IParticleSystemSceneNode.h"
#include "IrrlichtDevice.h"

#include <algorithm>
#include <cstddef>

namespace GE
{
namespace
{
// Binding of the push constants uniform in group 1, see compile_shaders.py
const uint32_t PUSH_CONSTANTS_BINDING = 4;
// Dynamic uniform offsets must be multiples of 256
const uint32_t PUSH_CONSTANTS_SLOT = 256;
const uint64_t PUSH_CONSTANTS_SIZE = 64;
// Binding of u_global_light in group 1 (global_light_data.glsl)
const uint32_t GLOBAL_LIGHT_BINDING = 3;
const wgpu::TextureFormat DEPTH_FORMAT = wgpu::TextureFormat::Depth32Float;

wgpu::BindGroupLayout g_material_layout;
wgpu::BindGroupLayout g_data_layout;
// Group 2 of the PBR shaders: diffuse and specular environment cube maps
wgpu::BindGroupLayout g_env_layout;
wgpu::BindGroup g_env_bind_group;
wgpu::PipelineLayout g_pipeline_layout;
std::unordered_map<std::string, wgpu::RenderPipeline> g_pipelines;
// Key is the texture views of every layer, then the sampler
std::map<std::vector<WGPUTextureView>, wgpu::BindGroup> g_material_bind_groups;
std::unordered_map<std::string, char> g_drawing_priority;

// ----------------------------------------------------------------------------
/** Billboards and particles share one quad in the mesh cache, a buffer per
 *  texture set only carries the material. */
class GEWGPUBillboardBuffer : public GESPMBuffer
{
private:
    GESPMBuffer* m_quad;
public:
    GEWGPUBillboardBuffer(GESPMBuffer* quad,
                          const irr::video::SMaterial& material)
        : m_quad(quad)                               { m_material = material; }
    virtual irr::u32 getIndexCount() const    { return m_quad->getIndexCount(); }
    virtual size_t getVBOOffset() const        { return m_quad->getVBOOffset(); }
    virtual size_t getIBOOffset() const        { return m_quad->getIBOOffset(); }
};

// ----------------------------------------------------------------------------
GEWGPUDrawCall::TexturesList getTexturesList(const irr::video::SMaterial& m)
{
    GEWGPUDrawCall::TexturesList textures;
    for (unsigned i = 0; i < textures.size(); i++)
        textures[i] = m.TextureLayer[i].Texture;
    return textures;
}   // getTexturesList

// ----------------------------------------------------------------------------
void createLayouts()
{
    if (g_pipeline_layout)
        return;
    const wgpu::Device& device = getWGPUDriver()->getDevice();
    const unsigned layers = GEWGPUShaderManager::getMeshTextureLayer();
    std::vector<wgpu::BindGroupLayoutEntry> entries;
    for (unsigned i = 0; i < layers; i++)
    {
        wgpu::BindGroupLayoutEntry texture;
        texture.binding = i;
        texture.visibility = wgpu::ShaderStage::Fragment;
        texture.texture.sampleType = wgpu::TextureSampleType::Float;
        texture.texture.viewDimension = wgpu::TextureViewDimension::e2D;
        entries.push_back(texture);
        wgpu::BindGroupLayoutEntry sampler;
        sampler.binding = i + GEWGPUShaderManager::getSamplerBindingOffset();
        sampler.visibility = wgpu::ShaderStage::Fragment;
        sampler.sampler.type = wgpu::SamplerBindingType::Filtering;
        entries.push_back(sampler);
    }
    wgpu::BindGroupLayoutDescriptor desc;
    desc.label = "mesh textures";
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    g_material_layout = device.CreateBindGroupLayout(&desc);

    const wgpu::ShaderStage vs_fs =
        wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries.clear();
    entries.resize(5);
    entries[0].binding = 0;
    entries[0].visibility = vs_fs;
    entries[0].buffer.type = wgpu::BufferBindingType::Uniform;
    entries[0].buffer.minBindingSize = sizeof(GEWGPUCameraUBO);
    entries[1].binding = 1;
    entries[1].visibility = vs_fs;
    entries[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[2].binding = 2;
    entries[2].visibility = wgpu::ShaderStage::Vertex;
    entries[2].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    entries[3].binding = PUSH_CONSTANTS_BINDING;
    entries[3].visibility = vs_fs;
    entries[3].buffer.type = wgpu::BufferBindingType::Uniform;
    entries[3].buffer.hasDynamicOffset = true;
    entries[3].buffer.minBindingSize = PUSH_CONSTANTS_SIZE;
    entries[4].binding = GLOBAL_LIGHT_BINDING;
    entries[4].visibility = vs_fs;
    entries[4].buffer.type = wgpu::BufferBindingType::Uniform;
    entries[4].buffer.minBindingSize = sizeof(GEGlobalLightBuffer);
    desc.label = "mesh data";
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    g_data_layout = device.CreateBindGroupLayout(&desc);

    entries.clear();
    for (unsigned i = 0; i < 2; i++)
    {
        wgpu::BindGroupLayoutEntry cube;
        cube.binding = i;
        cube.visibility = wgpu::ShaderStage::Fragment;
        cube.texture.sampleType = wgpu::TextureSampleType::Float;
        cube.texture.viewDimension = wgpu::TextureViewDimension::Cube;
        entries.push_back(cube);
        wgpu::BindGroupLayoutEntry sampler;
        sampler.binding = i + GEWGPUShaderManager::getSamplerBindingOffset();
        sampler.visibility = wgpu::ShaderStage::Fragment;
        sampler.sampler.type = wgpu::SamplerBindingType::Filtering;
        entries.push_back(sampler);
    }
    desc.label = "environment maps";
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    g_env_layout = device.CreateBindGroupLayout(&desc);

    // Black environment until image based lighting is implemented
    wgpu::TextureDescriptor tex_desc;
    tex_desc.label = "dummy environment map";
    tex_desc.size = { 1, 1, 6 };
    tex_desc.format = wgpu::TextureFormat::RGBA8Unorm;
    tex_desc.usage = wgpu::TextureUsage::TextureBinding |
        wgpu::TextureUsage::CopyDst;
    wgpu::Texture dummy = device.CreateTexture(&tex_desc);
    const uint32_t black[6] = {};
    wgpu::TexelCopyTextureInfo dst;
    dst.texture = dummy;
    wgpu::TexelCopyBufferLayout data_layout;
    data_layout.bytesPerRow = 4;
    data_layout.rowsPerImage = 1;
    wgpu::Extent3D extent = { 1, 1, 6 };
    getWGPUDriver()->getQueue().WriteTexture(&dst, black, sizeof(black),
        &data_layout, &extent);
    wgpu::TextureViewDescriptor view_desc;
    view_desc.dimension = wgpu::TextureViewDimension::Cube;
    wgpu::TextureView dummy_view = dummy.CreateView(&view_desc);
    std::array<wgpu::BindGroupEntry, 4> env_entries = {};
    for (unsigned i = 0; i < 2; i++)
    {
        env_entries[i * 2].binding = i;
        env_entries[i * 2].textureView = dummy_view;
        env_entries[i * 2 + 1].binding =
            i + GEWGPUShaderManager::getSamplerBindingOffset();
        env_entries[i * 2 + 1].sampler =
            getWGPUDriver()->getSampler(GVS_SKYBOX);
    }
    wgpu::BindGroupDescriptor bg_desc;
    bg_desc.label = "environment maps";
    bg_desc.layout = g_env_layout;
    bg_desc.entryCount = env_entries.size();
    bg_desc.entries = env_entries.data();
    g_env_bind_group = device.CreateBindGroup(&bg_desc);

    std::array<wgpu::BindGroupLayout, 3> layouts =
        {{ g_material_layout, g_data_layout, g_env_layout }};
    wgpu::PipelineLayoutDescriptor pl_desc;
    pl_desc.bindGroupLayoutCount = layouts.size();
    pl_desc.bindGroupLayouts = layouts.data();
    g_pipeline_layout = device.CreatePipelineLayout(&pl_desc);

    // Same drawing order as GEVulkanDrawCall::createAllPipelines: opaque
    // materials, ghost, then transparent materials
    char order = 1;
    for (auto& p : GEMaterialManager::g_materials)
    {
        if (!p.second->isTransparent())
            g_drawing_priority[p.first] = order++;
    }
    g_drawing_priority["ghost"] = order++;
    for (auto& p : GEMaterialManager::g_materials)
    {
        if (p.second->isTransparent())
            g_drawing_priority[p.first] = order++;
    }
}   // createLayouts

// ----------------------------------------------------------------------------
/** Returns a null pipeline if the shader is not drawn in pass pt. */
wgpu::RenderPipeline getPipeline(const std::string& shader, bool skinning,
                                 GEWGPUPassType pt,
                                 wgpu::TextureFormat color_format,
                                 const GEWGPUShaderManager::Constants& c)
{
    std::string key = std::to_string(pt) + shader +
        (skinning ? "_skinning" : "") + std::to_string((int)color_format) +
        (c.m_ibl ? "i" : "") + (c.m_has_skybox ? "s" : "");
    auto it = g_pipelines.find(key);
    if (it != g_pipelines.end())
        return it->second;

    const bool ghost = shader == "ghost";
    auto material =
        GEMaterialManager::getMaterial(ghost ? "solid" : shader);
    std::string fragment_shader = material->m_fragment_shader;
    bool depth_write = material->m_depth_write;
    bool write_color = true;
    bool alphablend = material->m_alphablend;
    bool additive = material->m_additive;
    wgpu::CompareFunction depth_compare = wgpu::CompareFunction::Less;
    bool valid = false;
    switch (pt)
    {
    case GWPT_SOLID:
        valid = !ghost && !material->isTransparent();
        break;
    case GWPT_GHOST_DEPTH:
        valid = ghost;
        fragment_shader = "depth_only.frag";
        write_color = false;
        depth_write = true;
        break;
    case GWPT_TRANSPARENT:
        if (ghost)
        {
            valid = true;
            fragment_shader = "ghost.frag";
            depth_write = false;
            depth_compare = wgpu::CompareFunction::Equal;
            alphablend = true;
            additive = false;
        }
        else
            valid = material->isTransparent();
        break;
    default:
        break;
    }
    const std::string& vertex_shader = skinning ?
        material->m_skinning_vertex_shader : material->m_vertex_shader;
    if (!valid || vertex_shader.empty())
    {
        g_pipelines[key] = nullptr;
        return nullptr;
    }

    // Matches the layout written by copyToMappedBuffer and GEWGPUMeshCache
    const size_t bone_pitch = sizeof(int16_t) * 8;
    const size_t static_pitch =
        sizeof(irr::video::S3DVertexSkinnedMesh) - bone_pitch;
    std::array<wgpu::VertexAttribute, 6> attrs = {};
    attrs[0] = { nullptr, wgpu::VertexFormat::Float32x3, 0, 0 };
    attrs[1] = { nullptr, wgpu::VertexFormat::Uint32, 12, 1 };
    attrs[2] = { nullptr, wgpu::VertexFormat::Unorm8x4, 16, 2 };
    attrs[3] = { nullptr, wgpu::VertexFormat::Float16x2, 20, 3 };
    attrs[4] = { nullptr, wgpu::VertexFormat::Float16x2, 24, 4 };
    attrs[5] = { nullptr, wgpu::VertexFormat::Uint32, 28, 5 };
    std::array<wgpu::VertexAttribute, 2> bone_attrs = {};
    bone_attrs[0] = { nullptr, wgpu::VertexFormat::Sint16x4, 0, 6 };
    bone_attrs[1] = { nullptr, wgpu::VertexFormat::Float16x4, 8, 7 };
    std::array<wgpu::VertexBufferLayout, 2> buffers = {};
    buffers[0].arrayStride = static_pitch;
    buffers[0].attributeCount = attrs.size();
    buffers[0].attributes = attrs.data();
    buffers[1].arrayStride = bone_pitch;
    buffers[1].attributeCount = bone_attrs.size();
    buffers[1].attributes = bone_attrs.data();

    wgpu::BlendState blend;
    if (additive)
    {
        blend.color = { wgpu::BlendOperation::Add, wgpu::BlendFactor::One,
            wgpu::BlendFactor::One };
    }
    else
    {
        // Premultiplied alpha, as in GEVulkanDrawCall
        blend.color = { wgpu::BlendOperation::Add, wgpu::BlendFactor::One,
            wgpu::BlendFactor::OneMinusSrcAlpha };
    }
    blend.alpha = blend.color;
    wgpu::ColorTargetState target;
    target.format = color_format;
    if (write_color && (alphablend || additive))
        target.blend = &blend;
    if (!write_color)
        target.writeMask = wgpu::ColorWriteMask::None;

    wgpu::FragmentState fragment;
    fragment.module = GEWGPUShaderManager::getShader(fragment_shader);
    fragment.entryPoint = "main";
    fragment.targetCount = 1;
    fragment.targets = &target;
    std::vector<wgpu::ConstantEntry> fs_constants =
        GEWGPUShaderManager::getConstants(fragment_shader, c);
    fragment.constantCount = fs_constants.size();
    fragment.constants = fs_constants.data();

    wgpu::DepthStencilState depth;
    depth.format = DEPTH_FORMAT;
    depth.depthWriteEnabled =
        depth_write ? wgpu::OptionalBool::True : wgpu::OptionalBool::False;
    depth.depthCompare = material->m_depth_test ?
        depth_compare : wgpu::CompareFunction::Always;

    wgpu::RenderPipelineDescriptor desc;
    desc.label = key.c_str();
    desc.layout = g_pipeline_layout;
    desc.vertex.module = GEWGPUShaderManager::getShader(vertex_shader);
    desc.vertex.entryPoint = "main";
    std::vector<wgpu::ConstantEntry> vs_constants =
        GEWGPUShaderManager::getConstants(vertex_shader, c);
    desc.vertex.constantCount = vs_constants.size();
    desc.vertex.constants = vs_constants.data();
    desc.vertex.bufferCount = skinning ? 2 : 1;
    desc.vertex.buffers = buffers.data();
    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    // Same winding as the Vulkan renderer: its y flip in the projection and
    // naga's y negation cancel out on screen
    desc.primitive.frontFace = wgpu::FrontFace::CW;
    desc.primitive.cullMode = material->m_backface_culling ?
        wgpu::CullMode::Back : wgpu::CullMode::None;
    desc.depthStencil = &depth;
    desc.fragment = &fragment;
    wgpu::RenderPipeline pipeline =
        getWGPUDriver()->getDevice().CreateRenderPipeline(&desc);
    g_pipelines[key] = pipeline;
    return pipeline;
}   // getPipeline

// ----------------------------------------------------------------------------
const wgpu::BindGroup& getMaterialBindGroup(
                                const GEWGPUDrawCall::TexturesList& textures,
                                const std::string& shader)
{
    const bool pbr = getGEConfig()->m_pbr;
    auto material = GEMaterialManager::getMaterial(
        shader == "ghost" ? "solid" : shader);
    GEWGPUDriver* driver = getWGPUDriver();
    const unsigned layers = GEWGPUShaderManager::getMeshTextureLayer();
    const wgpu::Sampler& sampler =
        driver->getSampler(driver->getMeshSampler());
    std::vector<WGPUTextureView> key(layers + 1);
    std::vector<wgpu::TextureView> views(layers);
    for (unsigned i = 0; i < layers; i++)
    {
        // Same defaults as GEVulkanTextureDescriptor
        const GEWGPUTexture* t = dynamic_cast<const GEWGPUTexture*>(
            textures[i]);
        if (!t)
        {
            t = static_cast<const GEWGPUTexture*>(i == 0 ?
                driver->getWhiteTexture() : driver->getTransparentTexture());
        }
        views[i] = t->getView(pbr && i < material->m_srgb_settings.size() &&
            material->m_srgb_settings[i]);
        key[i] = views[i].Get();
    }
    key[layers] = (WGPUTextureView)sampler.Get();
    auto it = g_material_bind_groups.find(key);
    if (it != g_material_bind_groups.end())
        return it->second;

    std::vector<wgpu::BindGroupEntry> entries;
    for (unsigned i = 0; i < layers; i++)
    {
        wgpu::BindGroupEntry texture;
        texture.binding = i;
        texture.textureView = views[i];
        entries.push_back(texture);
        wgpu::BindGroupEntry s;
        s.binding = i + GEWGPUShaderManager::getSamplerBindingOffset();
        s.sampler = sampler;
        entries.push_back(s);
    }
    wgpu::BindGroupDescriptor desc;
    desc.layout = g_material_layout;
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    return g_material_bind_groups[key] =
        driver->getDevice().CreateBindGroup(&desc);
}   // getMaterialBindGroup

// ----------------------------------------------------------------------------
GESPMBuffer* getBillboardQuadBuffer()
{
    irr::scene::IMesh* quad = getWGPUDriver()->getBillboardQuad();
    return quad ? static_cast<GESPMBuffer*>(quad->getMeshBuffer(0)) : NULL;
}   // getBillboardQuadBuffer

}   // anonymous namespace

// ============================================================================
GEWGPUDrawCall::GEWGPUDrawCall()
              : m_culling_tool(new GECullingTool)
{
    m_skybox_renderer = NULL;
    m_camera = NULL;
    m_object_buffer_size = m_skinning_buffer_size =
        m_push_constants_buffer_size = 0;
}   // GEWGPUDrawCall

// ----------------------------------------------------------------------------
GEWGPUDrawCall::~GEWGPUDrawCall()
{
    for (auto& p : m_billboard_buffers)
        p.second->drop();
}   // ~GEWGPUDrawCall

// ----------------------------------------------------------------------------
void GEWGPUDrawCall::reset()
{
    m_visible_nodes.clear();
    m_dynamic_spm_buffers.clear();
    m_skinning_nodes.clear();
    m_cmds.clear();
    m_objects.clear();
    m_skinning.clear();
    m_push_constants_offsets.clear();
    m_skybox_renderer = NULL;
    m_camera = NULL;
}   // reset

// ----------------------------------------------------------------------------
std::string GEWGPUDrawCall::getShader(const irr::video::SMaterial& m) const
{
    std::string shader = GEMaterialManager::getShader(m.MaterialType);
    auto material = GEMaterialManager::getMaterial(shader);
    // displace needs deferred rendering, which is not implemented yet
    if (!material->m_nonpbr_fallback.empty() &&
        (!getGEConfig()->m_pbr || shader == "displace"))
    {
        shader = material->m_nonpbr_fallback;
        material = GEMaterialManager::getMaterial(shader);
    }
    auto& ri = m.getRenderInfo();
    // Use real transparent shader first
    if (!material->isTransparent() && ri && ri->isTransparent())
        return "ghost";
    return shader;
}   // getShader

// ----------------------------------------------------------------------------
void GEWGPUDrawCall::prepare(GEWGPUCameraSceneNode* cam)
{
    reset();
    m_camera = cam;
    m_culling_tool->init(cam);
    m_view_position = cam->getAbsolutePosition();
    m_billboard_rotation = MiniGLM::getBulletQuaternion(cam->getViewMatrix());
    if (getGEConfig()->m_pbr)
    {
        if (!m_light_handler)
        {
            m_light_handler.reset(new GEVulkanLightHandler(getWGPUDriver()
                ->getIrrlichtDevice()->getSceneManager()));
        }
        m_light_handler->prepare();
    }
    else
        m_light_handler.reset();
}   // prepare

// ----------------------------------------------------------------------------
void GEWGPUDrawCall::addLightNode(irr::scene::ILightSceneNode* node)
{
    if (!m_light_handler)
        return;
    if (node->getLightType() != irr::video::ELT_DIRECTIONAL)
    {
        const irr::video::SLight& l = node->getLightData();
        if (m_culling_tool->isCulled(l.Position, l.Radius))
            return;
    }
    m_light_handler->addLightNode(node);
}   // addLightNode

// ----------------------------------------------------------------------------
void GEWGPUDrawCall::addNode(irr::scene::ISceneNode* node)
{
    irr::scene::IMesh* mesh;
    GEVulkanAnimatedMeshSceneNode* anode = NULL;
    if (node->getType() == irr::scene::ESNT_ANIMATED_MESH)
    {
        anode = static_cast<GEVulkanAnimatedMeshSceneNode*>(node);
        mesh = anode->getMesh();
    }
    else if (node->getType() == irr::scene::ESNT_MESH)
    {
        mesh = static_cast<irr::scene::IMeshSceneNode*>(node)->getMesh();
        for (unsigned i = 0; i < mesh->getMeshBufferCount(); i++)
        {
            irr::scene::IMeshBuffer* b = mesh->getMeshBuffer(i);
            if (b->getVertexType() != irr::video::EVT_SKINNED_MESH)
                return;
        }
    }
    else
        return;

    for (unsigned i = 0; i < mesh->getMeshBufferCount(); i++)
    {
        GESPMBuffer* buffer = static_cast<GESPMBuffer*>(
            mesh->getMeshBuffer(i));
        if (m_culling_tool->isCulled(buffer, node))
            continue;
        if (buffer->getHardwareMappingHint_Vertex() == irr::scene::EHM_STREAM ||
            buffer->getHardwareMappingHint_Index() == irr::scene::EHM_STREAM)
        {
            m_dynamic_spm_buffers.emplace_back(
                static_cast<GEWGPUDynamicSPMBuffer*>(buffer), node);
            continue;
        }
        const irr::video::SMaterial& m = node->getMaterial(i);
        auto k = std::make_pair(buffer, getTexturesList(m));
        m_visible_nodes[k][getShader(m)].emplace_back(node, i);
        if (anode && !anode->getSkinningMatrices().empty())
            m_skinning_nodes.insert(anode);
    }
}   // addNode

// ----------------------------------------------------------------------------
void GEWGPUDrawCall::addBillboardNode(irr::scene::ISceneNode* node,
                                      irr::scene::ESCENE_NODE_TYPE node_type)
{
    GESPMBuffer* quad = getBillboardQuadBuffer();
    if (!quad)
        return;
    irr::core::aabbox3df bb = node->getTransformedBoundingBox();
    if (m_culling_tool->isCulled(bb))
        return;
    const irr::video::SMaterial& m = node->getMaterial(0);
    TexturesList textures = getTexturesList(m);
    auto it = m_billboard_buffers.find(textures);
    if (it == m_billboard_buffers.end())
    {
        it = m_billboard_buffers.emplace(textures,
            new GEWGPUBillboardBuffer(quad, m)).first;
    }
    auto k = std::make_pair(it->second, textures);
    m_visible_nodes[k][getShader(m)].emplace_back(node,
        node_type == irr::scene::ESNT_BILLBOARD ? BILLBOARD_NODE :
        PARTICLE_NODE);
}   // addBillboardNode

// ----------------------------------------------------------------------------
void GEWGPUDrawCall::addSkyBox(irr::scene::ISceneNode* node)
{
    m_skybox_renderer = getWGPUDriver()->getSkyBoxRenderer();
    m_skybox_renderer->addSkyBox(node);
}   // addSkyBox

// ----------------------------------------------------------------------------
void GEWGPUDrawCall::generate()
{
    createLayouts();
    if (m_light_handler)
    {
        irr::video::SColor skytop;
        if (m_skybox_renderer)
            skytop = m_skybox_renderer->getSkytopColor();
        m_light_handler->generate(m_view_position,
            m_skybox_renderer ? &skytop : NULL, false/*deferred*/);
    }
    // Index 0 is the identity matrix, for nodes without skinning data
    m_skinning.emplace_back();
    std::unordered_map<irr::scene::ISceneNode*, int> skinning_offsets;
    for (GEVulkanAnimatedMeshSceneNode* node : m_skinning_nodes)
    {
        skinning_offsets[node] = m_skinning.size();
        const auto& matrices = node->getSkinningMatrices();
        m_skinning.insert(m_skinning.end(), matrices.begin(), matrices.end());
    }

    auto sorting_key = [](const std::string& shader)
    {
        auto it = g_drawing_priority.find(shader);
        char priority = it == g_drawing_priority.end() ? 1 : it->second;
        return std::string(1, priority) + shader;
    };

    for (auto& p : m_dynamic_spm_buffers)
    {
        GEWGPUDynamicSPMBuffer* buffer = p.first;
        irr::scene::ISceneNode* node = p.second;
        if (buffer->getIndexCount() == 0)
            continue;
        const irr::video::SMaterial& m = node->getMaterial(0);
        DrawCmd cmd = {};
        cmd.m_shader = getShader(m);
        cmd.m_sorting_key = sorting_key(cmd.m_shader);
        cmd.m_textures = getTexturesList(m);
        cmd.m_dynamic = buffer;
        cmd.m_index_count = buffer->getIndexCount();
        cmd.m_instance_count = 1;
        cmd.m_first_instance = m_objects.size();
        m_objects.emplace_back();
        m_objects.back().init(node, 0, -1, 0);
        m_cmds.push_back(cmd);
    }

    for (auto& p : m_visible_nodes)
    {
        GESPMBuffer* mb = p.first.first;
        const TexturesList& textures = p.first.second;
        for (auto& q : p.second)
        {
            if (q.second.empty())
                continue;
            DrawCmd cmd = {};
            cmd.m_shader = q.first;
            cmd.m_sorting_key = sorting_key(cmd.m_shader);
            cmd.m_textures = textures;
            cmd.m_skinning = mb->hasSkinning();
            cmd.m_index_count = mb->getIndexCount();
            cmd.m_first_index = mb->getIBOOffset();
            cmd.m_base_vertex = mb->getVBOOffset();
            cmd.m_first_instance = m_objects.size();
            const bool backface_culling =
                GEMaterialManager::getMaterial(cmd.m_shader)
                ->m_backface_culling;
            for (auto& r : q.second)
            {
                irr::scene::ISceneNode* node = r.first;
                if (r.second == BILLBOARD_NODE)
                {
                    m_objects.emplace_back();
                    m_objects.back().init(
                        static_cast<irr::scene::IBillboardSceneNode*>(node),
                        0, m_billboard_rotation);
                }
                else if (r.second == PARTICLE_NODE)
                {
                    auto* pn =
                        static_cast<irr::scene::IParticleSystemSceneNode*>(
                        node);
                    const irr::core::array<irr::scene::SParticle>& particles =
                        pn->getParticles();
                    for (unsigned i = 0; i < particles.size(); i++)
                    {
                        m_objects.emplace_back();
                        m_objects.back().init(particles[i], 0,
                            m_billboard_rotation, m_view_position,
                            pn->getFlips(), pn->isSkyParticle(),
                            backface_culling);
                    }
                }
                else
                {
                    auto it = skinning_offsets.find(node);
                    int skinning_offset =
                        it == skinning_offsets.end() ? -1000 : it->second;
                    m_objects.emplace_back();
                    m_objects.back().init(node, 0, skinning_offset, r.second);
                }
            }
            cmd.m_instance_count = m_objects.size() - cmd.m_first_instance;
            if (cmd.m_instance_count > 0)
                m_cmds.push_back(cmd);
        }
    }

    // Fewer bind group changes, then the drawing order of the materials
    std::stable_sort(m_cmds.begin(), m_cmds.end(),
        [](const DrawCmd& a, const DrawCmd& b)
        { return a.m_textures < b.m_textures; });
    std::stable_sort(m_cmds.begin(), m_cmds.end(),
        [](const DrawCmd& a, const DrawCmd& b)
        { return a.m_sorting_key < b.m_sorting_key; });
}   // generate

// ----------------------------------------------------------------------------
bool GEWGPUDrawCall::ensureBuffer(wgpu::Buffer& buffer, uint64_t& size,
                                  uint64_t needed, wgpu::BufferUsage usage,
                                  const char* label)
{
    if (buffer && size >= needed)
        return false;
    uint64_t new_size = 1024;
    while (new_size < needed)
        new_size *= 2;
    wgpu::BufferDescriptor desc;
    desc.label = label;
    desc.size = new_size;
    desc.usage = usage | wgpu::BufferUsage::CopyDst;
    buffer = getWGPUDriver()->getDevice().CreateBuffer(&desc);
    size = new_size;
    return true;
}   // ensureBuffer

// ----------------------------------------------------------------------------
void GEWGPUDrawCall::upload()
{
    if (!m_camera || (m_cmds.empty() && !m_skybox_renderer))
        return;
    createLayouts();
    GEWGPUDriver* driver = getWGPUDriver();
    const wgpu::Queue& queue = driver->getQueue();

    bool recreate = false;
    if (!m_camera_buffer)
    {
        uint64_t size = 0;
        ensureBuffer(m_camera_buffer, size, sizeof(GEWGPUCameraUBO),
            wgpu::BufferUsage::Uniform, "camera");
        recreate = true;
    }
    queue.WriteBuffer(m_camera_buffer, 0, m_camera->getUBOData(),
        sizeof(GEWGPUCameraUBO));

    const uint64_t object_size =
        std::max<size_t>(m_objects.size(), 1) * sizeof(ObjectData);
    recreate |= ensureBuffer(m_object_buffer, m_object_buffer_size,
        object_size, wgpu::BufferUsage::Storage, "objects");
    if (!m_objects.empty())
    {
        queue.WriteBuffer(m_object_buffer, 0, m_objects.data(),
            m_objects.size() * sizeof(ObjectData));
    }

    const uint64_t skinning_size =
        std::max<size_t>(m_skinning.size(), 1) * sizeof(irr::core::matrix4);
    recreate |= ensureBuffer(m_skinning_buffer, m_skinning_buffer_size,
        skinning_size, wgpu::BufferUsage::Storage, "skinning");
    if (!m_skinning.empty())
    {
        queue.WriteBuffer(m_skinning_buffer, 0, m_skinning.data(),
            m_skinning.size() * sizeof(irr::core::matrix4));
    }

    if (!m_light_buffer)
    {
        uint64_t size = 0;
        ensureBuffer(m_light_buffer, size, sizeof(GEGlobalLightBuffer),
            wgpu::BufferUsage::Uniform, "global light");
        recreate = true;
    }
    if (m_light_handler)
    {
        queue.WriteBuffer(m_light_buffer, 0, m_light_handler->getData(),
            (m_light_handler->getSize() + 3) & ~(size_t)3);
    }

    // Push constants of each material that has them, in its own slot
    std::vector<uint8_t> push_data(PUSH_CONSTANTS_SLOT, 0);
    for (const DrawCmd& cmd : m_cmds)
    {
        if (m_push_constants_offsets.count(cmd.m_shader))
            continue;
        auto material = GEMaterialManager::getMaterial(cmd.m_shader);
        if (!material->m_push_constants)
        {
            m_push_constants_offsets[cmd.m_shader] = 0;
            continue;
        }
        uint32_t size = 0;
        void* data = NULL;
        material->m_push_constants(&size, &data);
        uint32_t offset = push_data.size();
        push_data.resize(offset + PUSH_CONSTANTS_SLOT, 0);
        memcpy(&push_data[offset], data,
            std::min<uint32_t>(size, PUSH_CONSTANTS_SIZE));
        m_push_constants_offsets[cmd.m_shader] = offset;
    }
    recreate |= ensureBuffer(m_push_constants_buffer,
        m_push_constants_buffer_size, push_data.size(),
        wgpu::BufferUsage::Uniform, "push constants");
    queue.WriteBuffer(m_push_constants_buffer, 0, push_data.data(),
        push_data.size());

    if (recreate || !m_data_bind_group)
    {
        std::array<wgpu::BindGroupEntry, 5> entries = {};
        entries[0].binding = 0;
        entries[0].buffer = m_camera_buffer;
        entries[0].size = sizeof(GEWGPUCameraUBO);
        entries[1].binding = 1;
        entries[1].buffer = m_object_buffer;
        entries[1].size = m_object_buffer_size;
        entries[2].binding = 2;
        entries[2].buffer = m_skinning_buffer;
        entries[2].size = m_skinning_buffer_size;
        entries[3].binding = PUSH_CONSTANTS_BINDING;
        entries[3].buffer = m_push_constants_buffer;
        entries[3].size = PUSH_CONSTANTS_SIZE;
        entries[4].binding = GLOBAL_LIGHT_BINDING;
        entries[4].buffer = m_light_buffer;
        entries[4].size = sizeof(GEGlobalLightBuffer);
        wgpu::BindGroupDescriptor desc;
        desc.label = "mesh data";
        desc.layout = g_data_layout;
        desc.entryCount = entries.size();
        desc.entries = entries.data();
        m_data_bind_group = driver->getDevice().CreateBindGroup(&desc);
    }

    for (const DrawCmd& cmd : m_cmds)
    {
        if (cmd.m_dynamic)
            cmd.m_dynamic->update();
    }
    if (m_skybox_renderer)
        m_skybox_renderer->upload();
}   // upload

// ----------------------------------------------------------------------------
void GEWGPUDrawCall::render(wgpu::RenderPassEncoder& pass,
                            wgpu::TextureFormat color_format)
{
    if (!m_camera || !m_data_bind_group)
        return;
    GEWGPUDriver* driver = getWGPUDriver();
    const irr::core::dimension2du& size = driver->getCurrentRenderTargetSize();
    irr::core::recti vp = m_camera->getViewPort();
    vp.clipAgainst(irr::core::recti(0, 0, size.Width, size.Height));
    if (vp.getWidth() <= 0 || vp.getHeight() <= 0)
        return;
    pass.SetViewport(vp.UpperLeftCorner.X, vp.UpperLeftCorner.Y,
        vp.getWidth(), vp.getHeight(), 0.0f, 1.0f);
    pass.SetScissorRect(vp.UpperLeftCorner.X, vp.UpperLeftCorner.Y,
        vp.getWidth(), vp.getHeight());

    pass.SetBindGroup(2, g_env_bind_group);
    renderPass(pass, GWPT_SOLID, color_format);
    if (m_skybox_renderer)
        m_skybox_renderer->render(pass, color_format, m_data_bind_group,
            g_data_layout);
    renderPass(pass, GWPT_GHOST_DEPTH, color_format);
    renderPass(pass, GWPT_TRANSPARENT, color_format);
}   // render

// ----------------------------------------------------------------------------
void GEWGPUDrawCall::renderPass(wgpu::RenderPassEncoder& pass,
                                GEWGPUPassType pt,
                                wgpu::TextureFormat color_format)
{
    GEWGPUMeshCache* mc = static_cast<GEWGPUMeshCache*>(getWGPUDriver()
        ->getIrrlichtDevice()->getSceneManager()->getMeshCache());
    GEWGPUShaderManager::Constants constants;
    constants.m_has_skybox = m_skybox_renderer != NULL;
    WGPURenderPipeline cur_pipeline = NULL;
    const TexturesList* cur_textures = NULL;
    // 0: nothing bound, 1: mesh cache, 2: mesh cache with bones, 3: dynamic
    int cur_vertex_binding = 0;
    for (const DrawCmd& cmd : m_cmds)
    {
        wgpu::RenderPipeline pipeline = getPipeline(cmd.m_shader,
            cmd.m_skinning, pt, color_format, constants);
        if (!pipeline)
            continue;
        if (cmd.m_dynamic ? !cmd.m_dynamic->getVertexBuffer() :
            !mc->getBuffer())
            continue;
        const bool pipeline_changed = pipeline.Get() != cur_pipeline;
        if (pipeline_changed)
        {
            cur_pipeline = pipeline.Get();
            pass.SetPipeline(pipeline);
            uint32_t offset = m_push_constants_offsets[cmd.m_shader];
            pass.SetBindGroup(1, m_data_bind_group, 1, &offset);
        }
        if (!cur_textures || *cur_textures != cmd.m_textures ||
            pipeline_changed)
        {
            cur_textures = &cmd.m_textures;
            pass.SetBindGroup(0, getMaterialBindGroup(cmd.m_textures,
                cmd.m_shader));
        }
        if (cmd.m_dynamic)
        {
            pass.SetVertexBuffer(0, cmd.m_dynamic->getVertexBuffer());
            pass.SetIndexBuffer(cmd.m_dynamic->getIndexBuffer(),
                wgpu::IndexFormat::Uint16);
            cur_vertex_binding = 3;
        }
        else
        {
            int binding = cmd.m_skinning ? 2 : 1;
            if (cur_vertex_binding != binding &&
                !(cur_vertex_binding == 2 && binding == 1))
            {
                pass.SetVertexBuffer(0, mc->getBuffer());
                if (cmd.m_skinning)
                {
                    pass.SetVertexBuffer(1, mc->getBuffer(),
                        mc->getSkinningVBOOffset());
                }
                pass.SetIndexBuffer(mc->getBuffer(), wgpu::IndexFormat::Uint16,
                    mc->getIBOOffset());
                cur_vertex_binding = binding;
            }
        }
        pass.DrawIndexed(cmd.m_index_count, cmd.m_instance_count,
            cmd.m_first_index, cmd.m_base_vertex, cmd.m_first_instance);
    }
}   // renderPass

// ----------------------------------------------------------------------------
void GEWGPUDrawCall::destroyShared()
{
    g_pipelines.clear();
    g_material_bind_groups.clear();
    g_drawing_priority.clear();
    g_pipeline_layout = nullptr;
    g_material_layout = nullptr;
    g_data_layout = nullptr;
    g_env_layout = nullptr;
    g_env_bind_group = nullptr;
}   // destroyShared

// ----------------------------------------------------------------------------
void GEWGPUDrawCall::onTextureDestroyed()
{
    // Bind groups keep their textures alive, drop them all so the memory of
    // deleted textures is freed (textures are deleted rarely, mostly when
    // loading a track)
    g_material_bind_groups.clear();
}   // onTextureDestroyed

}
