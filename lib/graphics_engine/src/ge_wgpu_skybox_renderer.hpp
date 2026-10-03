#ifndef HEADER_GE_WGPU_SKYBOX_RENDERER_HPP
#define HEADER_GE_WGPU_SKYBOX_RENDERER_HPP

#include <SColor.h>
#include <webgpu/webgpu_cpp.h>

#include <map>

namespace irr
{
    namespace scene { class ISceneNode; }
}

namespace GE
{
/** Builds a cube map from the six textures of a skybox node and draws it
 *  behind everything (fullscreen_quad.vert + skybox.frag). With PBR and IBL
 *  it also renders the diffuse irradiance and specular prefiltered
 *  environment maps with compute shaders, as GEVulkanEnvironmentMap. */
class GEWGPUSkyBoxRenderer
{
private:
    irr::scene::ISceneNode* m_skybox;

    wgpu::Texture m_cubemap, m_diffuse_env, m_specular_env;

    wgpu::BindGroup m_bind_group, m_env_bind_group;

    wgpu::BindGroupLayout m_layout;

    wgpu::PipelineLayout m_pipeline_layout;

    std::map<std::pair<wgpu::TextureFormat, bool>, wgpu::RenderPipeline>
        m_pipelines;

    irr::video::SColor m_skytop_color;

    // ------------------------------------------------------------------------
    void generateEnvironmentMaps();
public:
    // ------------------------------------------------------------------------
    GEWGPUSkyBoxRenderer();
    // ------------------------------------------------------------------------
    void addSkyBox(irr::scene::ISceneNode* node);
    // ------------------------------------------------------------------------
    /** data is the draw call's group 1 (camera), bound with its layout. */
    void render(wgpu::RenderPassEncoder& pass,
                wgpu::TextureFormat color_format,
                const wgpu::BindGroup& data,
                const wgpu::BindGroupLayout& data_layout, bool deferred);
    // ------------------------------------------------------------------------
    /** Group 2 of the PBR shaders: diffuse and specular environment maps
     *  (dummy without image based lighting), skybox and its sRGB view. */
    const wgpu::BindGroup& getEnvBindGroup(const wgpu::BindGroupLayout& layout,
                                           const wgpu::TextureView& dummy);
    // ------------------------------------------------------------------------
    bool hasEnvironmentMaps() const         { return m_diffuse_env != nullptr; }
    // ------------------------------------------------------------------------
    void reset()                                           { m_skybox = NULL; }
    // ------------------------------------------------------------------------
    irr::video::SColor getSkytopColor() const        { return m_skytop_color; }
    // ------------------------------------------------------------------------
    /** floor(log2(specular map size)), u_specular_levels_minus_one */
    static float getSpecularLevelsMinusOne();
};   // GEWGPUSkyBoxRenderer

}

#endif
