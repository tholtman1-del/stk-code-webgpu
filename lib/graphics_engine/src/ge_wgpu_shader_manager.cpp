#include "ge_wgpu_shader_manager.hpp"

#include "ge_main.hpp"

#include <IFileSystem.h>
#include <IReadFile.h>

#include <map>
#include <regex>
#include <stdexcept>
#include <vector>

namespace GE
{
namespace GEWGPUShaderManager
{
// ============================================================================
wgpu::Device g_device;
irr::io::IFileSystem* g_file_system = NULL;
std::map<std::string, wgpu::ShaderModule> g_shaders;
std::map<std::string, std::vector<unsigned> > g_override_ids;
unsigned g_mesh_texture_layer = 2;
}   // GEWGPUShaderManager

// ============================================================================
void GEWGPUShaderManager::init(const wgpu::Device& device,
                               irr::io::IFileSystem* fs)
{
    g_device = device;
    g_file_system = fs;
    reload();
}   // init

// ----------------------------------------------------------------------------
void GEWGPUShaderManager::destroy()
{
    g_shaders.clear();
    g_override_ids.clear();
    g_device = nullptr;
    g_file_system = NULL;
}   // destroy

// ----------------------------------------------------------------------------
void GEWGPUShaderManager::reload()
{
    g_shaders.clear();
    g_override_ids.clear();
    g_mesh_texture_layer = getGEConfig()->m_pbr ? 8 : 2;
}   // reload

// ----------------------------------------------------------------------------
unsigned GEWGPUShaderManager::getMeshTextureLayer()
{
    return g_mesh_texture_layer;
}   // getMeshTextureLayer

// ----------------------------------------------------------------------------
wgpu::ShaderModule GEWGPUShaderManager::getShader(const std::string& filename)
{
    auto it = g_shaders.find(filename);
    if (it != g_shaders.end())
        return it->second;

    // getShaderFolder() is ".../shaders/ge_shaders/"
    std::string folder = getShaderFolder();
    folder = folder.substr(0, folder.size() - std::string("ge_shaders/").size());
    std::string path = folder + "ge_wgsl/" +
        (getGEConfig()->m_pbr ? "pbr/" : "basic/") + filename + ".wgsl";

    irr::io::IReadFile* file = g_file_system->createAndOpenFile(path.c_str());
    if (!file)
        throw std::runtime_error(std::string("Missing WGSL shader ") + path);
    std::string code;
    code.resize(file->getSize());
    file->read(&code[0], file->getSize());
    file->drop();

    wgpu::ShaderSourceWGSL wgsl;
    wgsl.code = wgpu::StringView(code.data(), code.size());
    wgpu::ShaderModuleDescriptor desc;
    desc.nextInChain = &wgsl;
    desc.label = wgpu::StringView(filename.data(), filename.size());
    wgpu::ShaderModule module = g_device.CreateShaderModule(&desc);
    if (!module)
        throw std::runtime_error(std::string("CreateShaderModule failed for ") +
            filename);
    g_shaders[filename] = module;

    std::vector<unsigned>& ids = g_override_ids[filename];
    std::regex re("@id\\(([0-9]+)\\) override");
    for (auto it = std::sregex_iterator(code.begin(), code.end(), re);
        it != std::sregex_iterator(); it++)
        ids.push_back(std::stoul((*it)[1].str()));
    return module;
}   // getShader

// ----------------------------------------------------------------------------
std::vector<wgpu::ConstantEntry> GEWGPUShaderManager::getConstants(
                                                   const std::string& filename,
                                                   const Constants& c)
{
    getShader(filename);
    static const char* keys[] = { "0", "1", "2", "3", "4", "5" };
    const double values[] = { c.m_ibl ? 1.0 : 0.0,
        c.m_specular_levels_minus_one, c.m_deferred ? 1.0 : 0.0,
        c.m_has_skybox ? 1.0 : 0.0, c.m_ssr ? 1.0 : 0.0,
        (double)c.m_hiz_iterations };
    std::vector<wgpu::ConstantEntry> entries;
    for (unsigned id : g_override_ids[filename])
    {
        if (id >= 6)
            continue;
        wgpu::ConstantEntry e;
        e.key = keys[id];
        e.value = values[id];
        entries.push_back(e);
    }
    return entries;
}   // getConstants

}
