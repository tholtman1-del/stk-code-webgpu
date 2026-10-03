diagnostic(off, derivative_uniformity);
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

struct LightData {
    m_position_radius: vec4<f32>,
    m_color_inverse_square_range: vec4<f32>,
    m_direction_scale_offset: vec4<f32>,
}

struct GlobalLightBuffer {
    m_ambient_color: vec3<f32>,
    m_sun_scatter: f32,
    m_sun_color: vec3<f32>,
    m_sun_angle_tan_half: f32,
    m_sun_direction: vec3<f32>,
    m_fog_density: f32,
    m_fog_color: vec4<f32>,
    m_skytop_color: vec3<f32>,
    m_light_count: i32,
    m_lights: array<LightData, 32>,
}

@group(1) @binding(0) 
var<uniform> u_camera: CameraBuffer;
@group(1) @binding(3) 
var<uniform> u_global_light: GlobalLightBuffer;
@group(0) @binding(18) 
var u_depth_sampler: sampler;
@group(0) @binding(2) 
var u_depth_image: texture_2d<f32>;
var<private> gl_FragCoord_1: vec4<f32>;
var<private> o_color: vec4<f32>;
@group(0) @binding(16) 
var u_color_sampler: sampler;
@group(0) @binding(0) 
var u_color_image: texture_2d<f32>;
@group(0) @binding(17) 
var u_normal_sampler: sampler;
@group(0) @binding(1) 
var u_normal_image: texture_2d<f32>;
var<private> light_idx_1: i32;

fn calculateLight_u0028_i1_u003b_vf3_u003b_vf3_u003b_vf3_u003b_vf3_u003b_f1_u003b_f1_u003b(i: ptr<function, i32>, diffuse_color: ptr<function, vec3<f32>>, normal: ptr<function, vec3<f32>>, xpos: ptr<function, vec3<f32>>, eyedir: ptr<function, vec3<f32>>, perceptual_roughness: ptr<function, f32>, metallic: ptr<function, f32>) -> vec3<f32> {
    var local: vec3<f32>;
    var local_1: f32;
    var local_2: f32;
    var local_3: f32;
    var local_4: f32;
    var local_5: f32;
    var local_6: f32;
    var local_7: f32;
    var local_8: f32;
    var local_9: f32;
    var local_10: f32;
    var local_11: f32;
    var local_12: f32;
    var local_13: f32;
    var local_14: f32;
    var local_15: f32;
    var local_16: f32;
    var local_17: f32;
    var local_18: f32;
    var local_19: f32;
    var local_20: vec4<f32>;
    var local_21: vec4<f32>;
    var local_22: vec4<f32>;
    var local_23: f32;
    var local_24: vec2<f32>;
    var local_25: f32;
    var local_26: f32;
    var local_27: vec2<f32>;
    var local_28: f32;
    var local_29: f32;
    var local_30: vec3<f32>;
    var local_31: f32;
    var local_32: f32;
    var local_33: vec3<f32>;
    var local_34: vec3<f32>;
    var local_35: f32;
    var local_36: f32;
    var local_37: f32;
    var local_38: vec3<f32>;
    var local_39: f32;
    var local_40: f32;
    var local_41: f32;
    var local_42: f32;
    var local_43: f32;
    var local_44: f32;
    var local_45: f32;
    var local_46: f32;
    var local_47: f32;
    var local_48: f32;
    var local_49: f32;
    var local_50: vec3<f32>;
    var local_51: vec3<f32>;
    var local_52: f32;
    var local_53: f32;
    var local_54: vec3<f32>;
    var local_55: vec3<f32>;
    var light_to_frag: vec3<f32>;
    var invrange: f32;
    var distance_sq: f32;
    var sattenuation: f32;
    var sscale: f32;
    var distance_: f32;
    var distance_inverse: f32;
    var L: vec3<f32>;
    var sdir: vec3<f32>;
    var diffuse_specular: vec3<f32>;
    var param: vec3<f32>;
    var param_1: vec3<f32>;
    var param_2: vec3<f32>;
    var param_3: vec3<f32>;
    var param_4: f32;
    var param_5: f32;
    var attenuation: f32;
    var radius: f32;
    var light_color: vec3<f32>;

    let _e132 = u_camera.m_view_matrix;
    let _e133 = (*i);
    let _e137 = u_global_light.m_lights[_e133].m_position_radius;
    let _e138 = _e137.xyz;
    let _e145 = (*xpos);
    light_to_frag = ((_e132 * vec4<f32>(_e138.x, _e138.y, _e138.z, 1f)).xyz - _e145);
    let _e147 = (*i);
    let _e152 = u_global_light.m_lights[_e147].m_color_inverse_square_range[3u];
    invrange = _e152;
    let _e153 = light_to_frag;
    let _e154 = light_to_frag;
    distance_sq = dot(_e153, _e154);
    let _e156 = distance_sq;
    let _e157 = invrange;
    if ((_e156 * _e157) > 1f) {
        return vec3<f32>(0f, 0f, 0f);
    }
    sattenuation = 1f;
    let _e160 = (*i);
    let _e165 = u_global_light.m_lights[_e160].m_direction_scale_offset[2u];
    sscale = _e165;
    let _e166 = distance_sq;
    distance_ = sqrt(_e166);
    let _e168 = distance_;
    distance_inverse = (1f / _e168);
    let _e170 = light_to_frag;
    let _e171 = distance_inverse;
    L = (_e170 * _e171);
    let _e173 = sscale;
    if (_e173 != 0f) {
        let _e175 = (*i);
        let _e179 = u_global_light.m_lights[_e175].m_direction_scale_offset;
        let _e180 = _e179.xy;
        sdir = vec3<f32>(_e180.x, _e180.y, 0f);
        let _e184 = sdir;
        let _e185 = sdir;
        let _e189 = sscale;
        sdir[2u] = (sqrt((1f - dot(_e184, _e185))) * sign(_e189));
        let _e194 = u_camera.m_view_matrix;
        let _e195 = sdir;
        sdir = (_e194 * vec4<f32>(_e195.x, _e195.y, _e195.z, 0f)).xyz;
        let _e202 = sdir;
        let _e204 = L;
        let _e206 = sscale;
        let _e209 = (*i);
        let _e214 = u_global_light.m_lights[_e209].m_direction_scale_offset[3u];
        sattenuation = clamp(((dot(-(_e202), _e204) * abs(_e206)) + _e214), 0f, 1f);
        let _e217 = sattenuation;
        if (_e217 == 0f) {
            return vec3<f32>(0f, 0f, 0f);
        }
    }
    let _e219 = (*normal);
    param = _e219;
    let _e220 = (*eyedir);
    param_1 = _e220;
    let _e221 = L;
    param_2 = _e221;
    let _e222 = (*diffuse_color);
    param_3 = _e222;
    let _e223 = (*perceptual_roughness);
    param_4 = _e223;
    let _e224 = (*metallic);
    param_5 = _e224;
    let _e225 = param;
    let _e226 = param_1;
    local_25 = max(dot(_e225, _e226), 0.0001f);
    let _e229 = param;
    let _e230 = param_2;
    local_26 = clamp(dot(_e229, _e230), 0f, 1f);
    let _e233 = param_4;
    local_28 = _e233;
    let _e234 = local_25;
    local_29 = _e234;
    local_20 = vec4<f32>(-1f, -0.0275f, -0.572f, 0.022f);
    local_21 = vec4<f32>(1f, 0.0425f, 1.04f, -0.04f);
    let _e235 = local_28;
    let _e236 = local_20;
    let _e238 = local_21;
    local_22 = ((_e236 * _e235) + _e238);
    let _e241 = local_22[0u];
    let _e243 = local_22[0u];
    let _e245 = local_29;
    let _e250 = local_22[0u];
    let _e253 = local_22[1u];
    local_23 = ((min((_e241 * _e243), pow(2f, (-9.28f * _e245))) * _e250) + _e253);
    let _e255 = local_23;
    let _e257 = local_22;
    local_24 = ((vec2<f32>(-1.04f, 1.04f) * _e255) + _e257.zw);
    let _e260 = local_24;
    local_27 = _e260;
    let _e261 = param_1;
    let _e262 = param_2;
    local_30 = normalize((_e261 + _e262));
    let _e265 = param;
    let _e266 = local_30;
    local_31 = clamp(dot(_e265, _e266), 0f, 1f);
    let _e269 = param_2;
    let _e270 = local_30;
    local_32 = clamp(dot(_e269, _e270), 0f, 1f);
    let _e273 = param_3;
    let _e274 = param_5;
    local_33 = (_e273 * (1f - _e274));
    let _e277 = param_3;
    let _e278 = param_5;
    local_34 = mix(vec3<f32>(0.04f, 0.04f, 0.04f), _e277, vec3(_e278));
    let _e281 = local_34;
    local_35 = clamp(dot(_e281, vec3<f32>(16.5f, 16.5f, 16.5f)), 0f, 1f);
    let _e284 = param_4;
    local_37 = _e284;
    let _e285 = local_37;
    local_18 = clamp(_e285, 0.089f, 1f);
    let _e287 = local_18;
    let _e288 = local_18;
    local_19 = (_e287 * _e288);
    let _e290 = local_19;
    local_36 = _e290;
    let _e291 = local_33;
    let _e292 = local_36;
    local_39 = _e292;
    let _e293 = local_25;
    local_40 = _e293;
    let _e294 = local_26;
    local_41 = _e294;
    let _e295 = local_31;
    local_42 = _e295;
    let _e296 = local_39;
    let _e298 = local_42;
    let _e300 = local_42;
    local_8 = (0.5f + (((2f * _e296) * _e298) * _e300));
    local_10 = 1f;
    let _e303 = local_8;
    local_11 = _e303;
    let _e304 = local_41;
    local_12 = _e304;
    let _e305 = local_10;
    let _e306 = local_11;
    let _e307 = local_12;
    local_7 = mix(_e305, _e306, pow((1f - _e307), 5f));
    let _e311 = local_7;
    local_9 = _e311;
    local_14 = 1f;
    let _e312 = local_8;
    local_15 = _e312;
    let _e313 = local_40;
    local_16 = _e313;
    let _e314 = local_14;
    let _e315 = local_15;
    let _e316 = local_16;
    local_6 = mix(_e314, _e315, pow((1f - _e316), 5f));
    let _e320 = local_6;
    local_13 = _e320;
    let _e321 = local_9;
    let _e322 = local_13;
    local_17 = (_e321 * _e322);
    let _e324 = local_17;
    local_38 = (_e291 * _e324);
    let _e326 = local_36;
    local_44 = _e326;
    let _e327 = local_31;
    local_45 = _e327;
    let _e328 = local_45;
    let _e329 = local_45;
    local_2 = (1f - (_e328 * _e329));
    let _e332 = local_45;
    let _e333 = local_44;
    local_3 = (_e332 * _e333);
    let _e335 = local_44;
    let _e336 = local_2;
    let _e337 = local_3;
    let _e338 = local_3;
    local_4 = (_e335 / (_e336 + (_e337 * _e338)));
    let _e342 = local_4;
    let _e343 = local_4;
    local_5 = ((_e342 * _e343) * 0.31830987f);
    let _e346 = local_5;
    local_43 = _e346;
    let _e347 = local_36;
    local_47 = _e347;
    let _e348 = local_25;
    local_48 = _e348;
    let _e349 = local_26;
    local_49 = _e349;
    let _e350 = local_49;
    let _e352 = local_48;
    let _e354 = local_49;
    let _e355 = local_48;
    let _e357 = local_47;
    local_1 = (0.5f / mix(((2f * _e350) * _e352), (_e354 + _e355), _e357));
    let _e360 = local_1;
    local_46 = _e360;
    let _e361 = local_34;
    local_51 = _e361;
    let _e362 = local_35;
    local_52 = _e362;
    let _e363 = local_32;
    local_53 = _e363;
    let _e364 = local_51;
    let _e365 = local_52;
    let _e366 = local_51;
    let _e369 = local_53;
    local = (_e364 + ((vec3(_e365) - _e366) * pow((1f - _e369), 5f)));
    let _e374 = local;
    local_50 = _e374;
    let _e375 = local_43;
    let _e376 = local_46;
    let _e378 = local_50;
    let _e380 = local_34;
    let _e382 = local_27[0u];
    local_54 = ((_e378 * (_e375 * _e376)) * (vec3(1f) + (_e380 * ((1f / _e382) - 1f))));
    let _e389 = local_26;
    let _e390 = local_38;
    let _e391 = local_54;
    local_55 = ((_e390 + _e391) * _e389);
    let _e394 = local_55;
    diffuse_specular = _e394;
    let _e395 = distance_sq;
    attenuation = (20f / (1f + _e395));
    let _e398 = (*i);
    let _e403 = u_global_light.m_lights[_e398].m_position_radius[3u];
    radius = _e403;
    let _e404 = radius;
    let _e405 = distance_;
    let _e407 = radius;
    let _e409 = attenuation;
    attenuation = (_e409 * ((_e404 - _e405) / _e407));
    let _e411 = sattenuation;
    let _e412 = sattenuation;
    let _e414 = attenuation;
    attenuation = (_e414 * (_e411 * _e412));
    let _e416 = (*i);
    let _e420 = u_global_light.m_lights[_e416].m_color_inverse_square_range;
    light_color = _e420.xyz;
    let _e422 = light_color;
    let _e423 = attenuation;
    let _e425 = diffuse_specular;
    return ((_e422 * _e423) * _e425);
}

fn main_1() {
    var local_56: vec2<f32>;
    var local_57: vec4<f32>;
    var local_58: vec4<f32>;
    var local_59: vec3<f32>;
    var local_60: vec3<f32>;
    var local_61: f32;
    var local_62: f32;
    var local_63: f32;
    var local_64: vec3<f32>;
    var depth: f32;
    var diffuse_color_1: vec3<f32>;
    var pbr: vec3<f32>;
    var world_normal: vec3<f32>;
    var param_6: vec2<f32>;
    var xpos_1: vec3<f32>;
    var param_7: vec3<f32>;
    var param_8: vec4<f32>;
    var param_9: mat4x4<f32>;
    var eyedir_1: vec3<f32>;
    var normal_1: vec3<f32>;
    var light: vec3<f32>;
    var param_10: i32;
    var param_11: vec3<f32>;
    var param_12: vec3<f32>;
    var param_13: vec3<f32>;
    var param_14: vec3<f32>;
    var param_15: f32;
    var param_16: f32;

    let _e77 = gl_FragCoord_1;
    let _e80 = textureLoad(u_depth_image, vec2<i32>(_e77.xy), 0i);
    depth = _e80.x;
    let _e82 = depth;
    if (_e82 == 1f) {
        o_color = vec4<f32>(0f, 0f, 0f, 1f);
        return;
    }
    let _e84 = gl_FragCoord_1;
    let _e87 = textureLoad(u_color_image, vec2<i32>(_e84.xy), 0i);
    diffuse_color_1 = _e87.xyz;
    let _e89 = gl_FragCoord_1;
    let _e92 = textureLoad(u_normal_image, vec2<i32>(_e89.xy), 0i);
    let _e93 = _e92.zw;
    let _e94 = gl_FragCoord_1;
    let _e97 = textureLoad(u_color_image, vec2<i32>(_e94.xy), 0i);
    pbr = vec3<f32>(_e93.x, _e93.y, _e97.w);
    let _e102 = gl_FragCoord_1;
    let _e105 = textureLoad(u_normal_image, vec2<i32>(_e102.xy), 0i);
    param_6 = _e105.xy;
    let _e107 = param_6;
    param_6 = ((_e107 * 2f) - vec2(1f));
    let _e112 = param_6[0u];
    let _e114 = param_6[1u];
    let _e116 = param_6[0u];
    let _e120 = param_6[1u];
    local_60 = vec3<f32>(_e112, _e114, ((1f - abs(_e116)) - abs(_e120)));
    let _e125 = local_60[2u];
    local_61 = max(-(_e125), 0f);
    let _e129 = local_60[0u];
    if (_e129 >= 0f) {
        let _e131 = local_61;
        local_62 = -(_e131);
    } else {
        let _e133 = local_61;
        local_62 = _e133;
    }
    let _e134 = local_62;
    let _e136 = local_60[0u];
    local_60[0u] = (_e136 + _e134);
    let _e140 = local_60[1u];
    if (_e140 >= 0f) {
        let _e142 = local_61;
        local_63 = -(_e142);
    } else {
        let _e144 = local_61;
        local_63 = _e144;
    }
    let _e145 = local_63;
    let _e147 = local_60[1u];
    local_60[1u] = (_e147 + _e145);
    let _e150 = local_60;
    local_64 = normalize(_e150);
    let _e152 = local_64;
    world_normal = _e152;
    let _e153 = gl_FragCoord_1;
    let _e154 = _e153.xy;
    let _e155 = depth;
    param_7 = vec3<f32>(_e154.x, _e154.y, _e155);
    let _e160 = u_camera.m_viewport;
    param_8 = _e160;
    let _e162 = u_camera.m_inverse_projection_matrix;
    param_9 = _e162;
    let _e164 = param_7[0u];
    let _e166 = param_8[0u];
    let _e169 = param_8[2u];
    let _e174 = param_7[1u];
    let _e176 = param_8[1u];
    let _e179 = param_8[3u];
    local_56 = vec2<f32>(((((_e164 - _e166) / _e169) * 2f) - 1f), ((((_e174 - _e176) / _e179) * 2f) - 1f));
    let _e184 = local_56;
    let _e186 = param_7[2u];
    local_57 = vec4<f32>(_e184.x, _e184.y, _e186, 1f);
    let _e190 = param_9;
    let _e191 = local_57;
    local_58 = (_e190 * _e191);
    let _e193 = local_58;
    let _e196 = local_58[3u];
    local_59 = (_e193.xyz / vec3(_e196));
    let _e199 = local_59;
    xpos_1 = _e199;
    let _e200 = xpos_1;
    eyedir_1 = -(normalize(_e200));
    let _e204 = u_camera.m_view_matrix;
    let _e205 = world_normal;
    normal_1 = (_e204 * vec4<f32>(_e205.x, _e205.y, _e205.z, 0f)).xyz;
    let _e213 = pbr[0u];
    let _e215 = light_idx_1;
    param_10 = _e215;
    let _e216 = diffuse_color_1;
    param_11 = _e216;
    let _e217 = normal_1;
    param_12 = _e217;
    let _e218 = xpos_1;
    param_13 = _e218;
    let _e219 = eyedir_1;
    param_14 = _e219;
    param_15 = (1f - _e213);
    let _e221 = pbr[1u];
    param_16 = _e221;
    let _e222 = calculateLight_u0028_i1_u003b_vf3_u003b_vf3_u003b_vf3_u003b_vf3_u003b_f1_u003b_f1_u003b((&param_10), (&param_11), (&param_12), (&param_13), (&param_14), (&param_15), (&param_16));
    light = _e222;
    let _e223 = light;
    o_color = vec4<f32>(_e223.x, _e223.y, _e223.z, 1f);
    return;
}

@fragment 
fn main(@builtin(position) gl_FragCoord: vec4<f32>, @location(0) @interpolate(flat) light_idx: i32) -> @location(0) vec4<f32> {
    gl_FragCoord_1 = gl_FragCoord;
    light_idx_1 = light_idx;
    main_1();
    let _e5 = o_color;
    return _e5;
}
