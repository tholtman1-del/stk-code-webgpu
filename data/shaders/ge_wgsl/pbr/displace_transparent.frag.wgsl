diagnostic(off, derivative_uniformity);
struct Constants {
    m_displace_direction: vec4<f32>,
}

struct CameraBuffer {
    m_view_matrix: mat4x4<f32>,
    m_projection_matrix: mat4x4<f32>,
    m_inverse_view_matrix: mat4x4<f32>,
    m_inverse_projection_matrix: mat4x4<f32>,
    m_projection_view_matrix: mat4x4<f32>,
    m_inverse_projection_view_matrix: mat4x4<f32>,
    m_viewport: vec4<f32>,
    m_screensize: vec2<f32>,
    m_padding: vec2<f32>,
}

@id(0) override u_ibl: bool = true;
@id(4) override u_ssr: bool = false;

@group(0) @binding(16) 
var f_mesh_texture_0_sampler: sampler;
@group(0) @binding(0) 
var f_mesh_texture_0_image: texture_2d<f32>;
@group(0) @binding(18) 
var f_mesh_texture_2_sampler: sampler;
@group(0) @binding(2) 
var f_mesh_texture_2_image: texture_2d<f32>;
var<private> gl_FragCoord_1: vec4<f32>;
var<private> f_uv_1: vec2<f32>;
var<private> f_vertex_color_1: vec4<f32>;
var<private> o_color: vec4<f32>;
@group(1) @binding(4)
var<uniform> u_push_constants: Constants;
@group(1) @binding(0) 
var<uniform> u_camera: CameraBuffer;
@group(3) @binding(16) 
var u_displace_mask_sampler: sampler;
@group(3) @binding(0) 
var u_displace_mask_image: texture_2d<f32>;
@group(3) @binding(17) 
var u_displace_ssr_sampler: sampler;
@group(3) @binding(1) 
var u_displace_ssr_image: texture_2d<f32>;

fn convertColor_u0028_vf3_u003b(input_color: ptr<function, vec3<f32>>) -> vec3<f32> {
    if u_ibl {
        let _e38 = (*input_color);
        let _e39 = (*input_color);
        let _e44 = (*input_color);
        let _e45 = (*input_color);
        return ((_e38 * ((_e39 * 6.5f) + vec3(0.45f))) / ((_e44 * ((_e45 * 5f) + vec3(1.75f))) + vec3(0.05f)));
    } else {
        let _e53 = (*input_color);
        let _e54 = (*input_color);
        let _e59 = (*input_color);
        let _e60 = (*input_color);
        return ((_e53 * ((_e54 * 7f) + vec3(0.75f))) / ((_e59 * ((_e60 * 5f) + vec3(1.75f))) + vec3(0.05f)));
    }
}

fn main_1() {
    var local: vec2<i32>;
    var local_1: vec2<i32>;
    var local_2: vec2<i32>;
    var local_3: vec2<i32>;
    var local_4: vec2<f32>;
    var local_5: vec2<i32>;
    var local_6: vec2<f32>;
    var local_7: vec4<f32>;
    var local_8: vec2<f32>;
    var local_9: vec2<f32>;
    var local_10: vec4<f32>;
    var local_11: vec4<f32>;
    var local_12: vec4<f32>;
    var local_13: vec4<f32>;
    var color: vec4<f32>;
    var param: vec2<f32>;
    var mixed_color: vec3<f32>;
    var alpha: f32;
    var param_1: vec3<f32>;
    var alpha_1: f32;
    var param_2: vec2<f32>;
    var horiz: f32;
    var param_3: vec2<f32>;
    var vert: f32;
    var param_4: vec2<f32>;
    var shift: vec2<f32>;
    var param_5: f32;
    var param_6: f32;
    var uv: vec2<i32>;
    var param_7: vec2<f32>;
    var param_8: vec4<f32>;
    var reflection: vec3<f32>;
    var phi_487_: bool;

    let _e69 = f_uv_1;
    param = _e69;
    let _e70 = param;
    let _e71 = textureSample(f_mesh_texture_0_image, f_mesh_texture_0_sampler, _e70);
    local_13 = _e71;
    let _e72 = local_13;
    let _e73 = f_vertex_color_1;
    color = (_e72 * _e73);
    let _e75 = color;
    mixed_color = _e75.xyz;
    let _e78 = color[3u];
    alpha = _e78;
    let _e79 = mixed_color;
    param_1 = _e79;
    let _e80 = convertColor_u0028_vf3_u003b((&param_1));
    mixed_color = _e80;
    if u_ssr {
        let _e81 = f_uv_1;
        param_2 = _e81;
        let _e82 = param_2;
        let _e83 = textureSample(f_mesh_texture_0_image, f_mesh_texture_0_sampler, _e82);
        local_12 = _e83;
        let _e84 = local_12;
        alpha_1 = _e84.w;
        let _e86 = alpha_1;
        if (_e86 == 0f) {
            let _e88 = mixed_color;
            let _e89 = alpha_1;
            let _e90 = (_e88 * _e89);
            let _e91 = alpha_1;
            o_color = vec4<f32>(_e90.x, _e90.y, _e90.z, _e91);
            return;
        }
        let _e96 = f_uv_1;
        let _e98 = u_push_constants.m_displace_direction;
        param_3 = (_e96 + (_e98.xy * 150f));
        let _e102 = param_3;
        let _e103 = textureSample(f_mesh_texture_2_image, f_mesh_texture_2_sampler, _e102);
        local_11 = _e103;
        let _e104 = local_11;
        horiz = _e104.x;
        let _e106 = f_uv_1;
        let _e109 = u_push_constants.m_displace_direction;
        param_4 = ((_e106.yx + (_e109.zw * 150f)) * vec2<f32>(0.9f, 0.9f));
        let _e114 = param_4;
        let _e115 = textureSample(f_mesh_texture_2_image, f_mesh_texture_2_sampler, _e114);
        local_10 = _e115;
        let _e116 = local_10;
        vert = _e116.x;
        let _e118 = horiz;
        param_5 = _e118;
        let _e119 = vert;
        param_6 = _e119;
        let _e120 = param_5;
        let _e121 = param_6;
        local_6 = vec2<f32>(_e120, _e121);
        let _e123 = local_6;
        local_6 = ((_e123 * 2f) - vec2(1f));
        let _e128 = local_6[0u];
        let _e131 = local_6[0u];
        local_7[0u] = (step(_e128, 0f) * -(_e131));
        let _e136 = local_6[0u];
        let _e139 = local_6[0u];
        local_7[1u] = (step(0f, _e136) * _e139);
        let _e143 = local_6[1u];
        let _e146 = local_6[1u];
        local_7[2u] = (step(_e143, 0f) * -(_e146));
        let _e151 = local_6[1u];
        let _e154 = local_6[1u];
        local_7[3u] = (step(0f, _e151) * _e154);
        let _e158 = local_7[0u];
        let _e161 = local_7[1u];
        local_8[0u] = (-(_e158) + _e161);
        let _e165 = local_7[2u];
        let _e168 = local_7[3u];
        local_8[1u] = (-(_e165) + _e168);
        let _e171 = local_8;
        local_9 = _e171;
        let _e172 = local_9;
        shift = _e172;
        let _e173 = shift;
        param_7 = _e173;
        let _e175 = u_camera.m_viewport;
        param_8 = _e175;
        let _e176 = gl_FragCoord_1;
        local = vec2<i32>(_e176.xy);
        let _e179 = param_8;
        let _e182 = param_7;
        param_7 = (_e182 * (_e179.zw * 0.02f));
        let _e184 = param_8;
        local_1 = vec2<i32>(_e184.xy);
        let _e187 = param_8;
        let _e189 = param_8;
        local_2 = vec2<i32>((_e187.xy + _e189.zw));
        let _e193 = gl_FragCoord_1;
        let _e196 = param_7;
        let _e199 = local_1;
        let _e200 = local_2;
        local_3 = clamp((vec2<i32>(_e193.xy) + vec2<i32>(_e196)), _e199, _e200);
        let _e202 = local_3;
        let _e203 = textureLoad(u_displace_mask_image, _e202, 0i);
        local_4 = _e203.xy;
        let _e206 = local_4[0u];
        let _e207 = (_e206 == 0f);
        phi_487_ = _e207;
        if _e207 {
            let _e209 = local_4[1u];
            phi_487_ = (_e209 == 0f);
        }
        let _e212 = phi_487_;
        if !(_e212) {
            let _e214 = local_3;
            local = _e214;
        }
        let _e215 = local;
        local_5 = _e215;
        let _e216 = local_5;
        uv = _e216;
        let _e217 = uv;
        let _e218 = textureLoad(u_displace_ssr_image, _e217, 0i);
        reflection = _e218.xyz;
        let _e220 = mixed_color;
        let _e221 = alpha_1;
        let _e224 = reflection;
        let _e225 = alpha_1;
        let _e228 = (((_e220 * _e221) * 0.5f) + ((_e224 * _e225) * 0.5f));
        let _e229 = alpha_1;
        o_color = vec4<f32>(_e228.x, _e228.y, _e228.z, _e229);
    } else {
        let _e234 = mixed_color;
        let _e235 = alpha;
        let _e236 = (_e234 * _e235);
        let _e237 = alpha;
        o_color = vec4<f32>(_e236.x, _e236.y, _e236.z, _e237);
    }
    return;
}

@fragment 
fn main(@builtin(position) gl_FragCoord: vec4<f32>, @location(1) f_uv: vec2<f32>, @location(0) f_vertex_color: vec4<f32>) -> @location(0) vec4<f32> {
    gl_FragCoord_1 = gl_FragCoord;
    f_uv_1 = f_uv;
    f_vertex_color_1 = f_vertex_color;
    main_1();
    let _e7 = o_color;
    return _e7;
}
