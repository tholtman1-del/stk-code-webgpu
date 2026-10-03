#include "ge_object_data.hpp"

#include "ge_main.hpp"
#include "ge_render_info.hpp"

#include "mini_glm.hpp"
#include "IBillboardSceneNode.h"
#include "IParticleSystemSceneNode.h"
#include "ISceneNode.h"
#include "quaternion.h"
#include "../source/Irrlicht/os.h"

#include <cmath>
#include <cstring>
#include <vector>

namespace GE
{
// ============================================================================
void ObjectData::init(irr::scene::ISceneNode* node, int material_id,
                      int skinning_offset, int irrlicht_material_id)
{
    using namespace MiniGLM;
    const irr::core::matrix4& model_mat = node->getAbsoluteTransformation();
    irr::core::quaternion rotation(0.0f, 0.0f, 0.0f, 1.0f);
    irr::core::vector3df scale = model_mat.getScale();
    if (scale.X != 0.0f && scale.Y != 0.0f && scale.Z != 0.0f)
    {
        irr::core::matrix4 local_mat = model_mat;
        local_mat[0] = local_mat[0] / scale.X / local_mat[15];
        local_mat[1] = local_mat[1] / scale.X / local_mat[15];
        local_mat[2] = local_mat[2] / scale.X / local_mat[15];
        local_mat[4] = local_mat[4] / scale.Y / local_mat[15];
        local_mat[5] = local_mat[5] / scale.Y / local_mat[15];
        local_mat[6] = local_mat[6] / scale.Y / local_mat[15];
        local_mat[8] = local_mat[8] / scale.Z / local_mat[15];
        local_mat[9] = local_mat[9] / scale.Z / local_mat[15];
        local_mat[10] = local_mat[10] / scale.Z / local_mat[15];
        rotation = getQuaternion(local_mat);
        // Conjugated quaternion in glsl
        rotation.W = -rotation.W;
    }
    memcpy(&m_translation_x, &node->getAbsoluteTransformation()[12],
        sizeof(float) * 3);
    memcpy(m_rotation, &rotation, sizeof(irr::core::quaternion));
    memcpy(&m_scale_x, &scale, sizeof(irr::core::vector3df));
    m_skinning_offset = skinning_offset;
    m_material_id = material_id;
    const irr::core::matrix4& texture_matrix =
        node->getMaterial(irrlicht_material_id).getTextureMatrix(0);
    m_texture_trans[0] = texture_matrix[8];
    m_texture_trans[1] = texture_matrix[9];
    auto& ri = node->getMaterial(irrlicht_material_id).getRenderInfo();
    if (ri && ri->getHue() > 0.0f)
        m_hue_change = ri->getHue();
    else
        m_hue_change = 0.0f;
    if (ri)
    {
        if (getGEConfig()->m_pbr)
        {
            m_custom_vertex_color =
                srgb255ToLinearFromSColor(ri->getVertexColor()).color;
        }
        else
        {
            m_custom_vertex_color = ri->getVertexColor();
        }
    }
    else
    {
        m_custom_vertex_color = irr::video::SColor((uint32_t)-1);
    }
}   // init

// ============================================================================
void ObjectData::init(irr::scene::IBillboardSceneNode* node, int material_id,
                      const btQuaternion& rotation)
{
    memcpy(&m_translation_x, &node->getAbsoluteTransformation()[12],
        sizeof(float) * 3);
    memcpy(m_rotation, &rotation[0], sizeof(btQuaternion));
    irr::core::vector2df billboard_size = node->getSize();
    m_scale_x = billboard_size.X / 2.0f;
    m_scale_y = billboard_size.Y / 2.0f;
    m_scale_z = 0.0f;
    m_skinning_offset = 0;
    m_material_id = material_id;
    m_texture_trans[0] = 0.0f;
    m_texture_trans[1] = 0.0f;
    m_hue_change = 0.0f;
    // Only support average of them at the moment
    irr::video::SColor top, bottom, output;
    node->getColor(top, bottom);
    output.setAlpha((top.getAlpha() + bottom.getAlpha()) / 2);
    output.setRed((top.getRed() + bottom.getRed()) / 2);
    output.setGreen((top.getGreen() + bottom.getGreen()) / 2);
    output.setBlue((top.getBlue() + bottom.getBlue()) / 2);
    if (getGEConfig()->m_pbr)
        output = srgb255ToLinearFromSColor(output).color;
    m_custom_vertex_color = output;
}   // init

// ============================================================================
std::vector<float> g_flips_data;
// ============================================================================
void ObjectData::init(const irr::scene::SParticle& particle, int material_id,
                      const btQuaternion& rotation,
                      const irr::core::vector3df& view_position, bool flips,
                      bool sky_particle, bool backface_culling)
{
    memcpy(&m_translation_x, &particle.pos, sizeof(float) * 3);
    float scale_x = particle.size.Width / 2.0f;
    if (flips)
    {
        // Following stk_particle.cpp
        const unsigned particle_index = particle.startTime;
        const float lifetime = particle.startSize.Width;
        const float pi = 3.14159265358979323846f;
        while (particle_index + 1 > g_flips_data.size())
        {
            // Maximum 3 rotation around axis (0, 1, 0) during lifetime
            g_flips_data.push_back(pi * 2.0f * 3.0f * os::Randomizer::frand() *
                (g_flips_data.size() % 2 == 0 ? 1.0f : -1.0f));
        }
        float angle = fmodf(lifetime * g_flips_data[particle_index],
            pi * 2.0f);
        btQuaternion rotated(btVector3(0.0f, 1.0f, 0.0f), angle);
        rotated = btQuaternion(rotation[0], rotation[1], rotation[2],
            -rotation[3]) * rotated;
        rotated.normalize();
        // Conjugated quaternion in glsl
        rotated[3] = -rotated[3];
        memcpy(m_rotation, &rotated[0], sizeof(btQuaternion));
        if (backface_culling)
        {
            irr::core::quaternion q(rotated[0], rotated[1], rotated[2],
                -rotated[3]);
            irr::core::matrix4 m;
            q.getMatrix(m, particle.pos);
            irr::core::vector3df tri[3] =
            {
                irr::core::vector3df( 1.0f, -1.0f, 0.0f),
                irr::core::vector3df( 1.0f,  1.0f, 0.0f),
                irr::core::vector3df(-1.0f,  1.0f, 0.0f)
            };
            m.transformVect(tri[0]);
            m.transformVect(tri[1]);
            m.transformVect(tri[2]);
            irr::core::vector3df normal = (tri[1] - tri[0])
                .crossProduct(tri[2] - tri[0]);
            float dot_product = (tri[0] - view_position).dotProduct(normal);
            if (dot_product < 0.0f)
                scale_x = -scale_x;
        }
    }
    else if (sky_particle)
    {
        irr::core::vector3df diff = particle.pos - view_position;
        float angle = atan2f(diff.X, diff.Z);
        btQuaternion rotated(btVector3(0.0f, 1.0f, 0.0f), angle);
        rotated.normalize();
        // Conjugated quaternion in glsl
        rotated[3] = -rotated[3];
        memcpy(m_rotation, &rotated[0], sizeof(btQuaternion));
    }
    else
        memcpy(m_rotation, &rotation[0], sizeof(btQuaternion));
    m_scale_x = scale_x;
    m_scale_y = particle.size.Height / 2.0f;
    m_scale_z = 0.0f;
    m_skinning_offset = 0;
    m_material_id = material_id;
    m_texture_trans[0] = 0.0f;
    m_texture_trans[1] = 0.0f;
    m_hue_change = 0.0f;
    if (getGEConfig()->m_pbr)
        m_custom_vertex_color = srgb255ToLinearFromSColor(particle.color).color;
    else
        m_custom_vertex_color = particle.color;
}   // init

}
