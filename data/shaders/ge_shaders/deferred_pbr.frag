#include "utils/subpass_input.glsl"
GE_SUBPASS_INPUT(0, 0, u_color)
GE_SUBPASS_INPUT(1, 1, u_normal)
GE_SUBPASS_INPUT(2, 2, u_depth)

layout(location = 0) out vec4 o_color;

layout(push_constant) uniform Constants
{
    int m_fullscreen_light_count;
} u_push_constants;

#include "utils/unproject_position.glsl"
#include "utils/handle_pbr.glsl"
#include "../utils/decodeNormal.frag"

void main()
{
    float depth = GE_SUBPASS_LOAD(u_depth).x;
    // Through a variable: an expression of specialization constants only
    // becomes OpSpecConstantOp, which naga (WebGPU) cannot read
    bool has_skybox = u_has_skybox;
    if (!has_skybox && depth == 1.0)
        discard;
    vec3 diffuse_color = GE_SUBPASS_LOAD(u_color).xyz;
    vec3 pbr = vec3(GE_SUBPASS_LOAD(u_normal).zw, GE_SUBPASS_LOAD(u_color).w);
    vec3 world_normal = DecodeNormal(GE_SUBPASS_LOAD(u_normal).xy);
    vec3 xpos = getPosFromUVDepth(vec3(gl_FragCoord.xy, depth),
        u_camera.m_viewport, u_camera.m_inverse_projection_matrix);
    vec3 eyedir = -normalize(xpos);
    vec3 normal = (u_camera.m_view_matrix * vec4(world_normal, 0.0)).xyz;
    vec3 hdr = handlePBRDeferred(diffuse_color, pbr, world_normal, eyedir,
        normal, 1.0 - pbr.x);
    hdr += accumulateLights(u_push_constants.m_fullscreen_light_count,
        diffuse_color, normal, xpos, eyedir, 1.0 - pbr.x, pbr.y);
    o_color = vec4(hdr, 1.0);
}
