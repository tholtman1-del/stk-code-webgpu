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

struct Constants {
    m_displace_direction: vec4<f32>,
}

struct FragmentOutput {
    @location(0) member: vec2<f32>,
    @location(1) member_1: vec4<f32>,
}

@id(5) override u_hiz_iterations: u32 = 0u;
@id(4) override u_ssr: bool = false;

@group(0) @binding(16) 
var f_mesh_texture_0_sampler: sampler;
@group(0) @binding(0) 
var f_mesh_texture_0_image: texture_2d<f32>;
@group(0) @binding(18) 
var f_mesh_texture_2_sampler: sampler;
@group(0) @binding(2) 
var f_mesh_texture_2_image: texture_2d<f32>;
@group(3) @binding(18) 
var u_hiz_depth_sampler: sampler;
@group(3) @binding(2) 
var u_hiz_depth_image: texture_2d<f32>;
@group(1) @binding(0) 
var<uniform> u_camera: CameraBuffer;
var<private> f_uv_1: vec2<f32>;
@group(1) @binding(4)
var<uniform> u_push_constants: Constants;
var<private> o_displace_mask: vec2<f32>;
var<private> o_displace_ssr: vec4<f32>;
var<private> f_world_position_1: vec4<f32>;
var<private> f_normal_1: vec3<f32>;
@group(2) @binding(18) 
var u_skybox_texture_sampler: sampler;
@group(2) @binding(2) 
var u_skybox_texture_image: texture_cube<f32>;
@group(3) @binding(17) 
var u_depth_sampler: sampler_comparison;
@group(3) @binding(1) 
var u_depth_image: texture_depth_2d;
@group(3) @binding(16) 
var u_displace_color_sampler: sampler;
@group(3) @binding(0) 
var u_displace_color_image: texture_2d<f32>;

fn main_1() {
    var local: vec2<f32>;
    var local_1: f32;
    var local_2: f32;
    var local_3: f32;
    var local_4: f32;
    var local_5: f32;
    var local_6: vec3<f32>;
    var local_7: vec3<f32>;
    var local_8: vec2<f32>;
    var local_9: vec2<f32>;
    var local_10: vec2<f32>;
    var local_11: f32;
    var local_12: vec3<f32>;
    var local_13: vec3<f32>;
    var local_14: f32;
    var local_15: vec3<f32>;
    var local_16: bool;
    var local_17: vec2<i32>;
    var local_18: vec3<f32>;
    var local_19: f32;
    var local_20: vec2<i32>;
    var local_21: vec2<i32>;
    var local_22: vec3<f32>;
    var local_23: vec3<f32>;
    var local_24: vec2<f32>;
    var local_25: vec2<f32>;
    var local_26: vec2<f32>;
    var local_27: f32;
    var local_28: vec3<f32>;
    var local_29: vec3<f32>;
    var local_30: f32;
    var local_31: vec3<f32>;
    var local_32: vec2<i32>;
    var local_33: vec2<i32>;
    var local_34: vec3<f32>;
    var local_35: f32;
    var local_36: i32;
    var local_37: f32;
    var local_38: vec3<f32>;
    var local_39: vec3<f32>;
    var local_40: vec2<f32>;
    var local_41: vec2<f32>;
    var local_42: vec3<f32>;
    var local_43: f32;
    var local_44: f32;
    var local_45: f32;
    var local_46: vec3<f32>;
    var local_47: vec3<f32>;
    var local_48: i32;
    var local_49: u32;
    var local_50: bool;
    var local_51: f32;
    var local_52: vec2<i32>;
    var local_53: i32;
    var local_54: vec2<i32>;
    var local_55: vec2<f32>;
    var local_56: vec2<i32>;
    var local_57: vec3<f32>;
    var local_58: vec3<f32>;
    var local_59: vec2<i32>;
    var local_60: vec2<i32>;
    var local_61: vec2<f32>;
    var local_62: vec2<f32>;
    var local_63: vec2<i32>;
    var local_64: i32;
    var local_65: vec2<i32>;
    var local_66: vec2<f32>;
    var local_67: vec2<i32>;
    var local_68: f32;
    var local_69: vec2<i32>;
    var local_70: i32;
    var local_71: vec3<f32>;
    var local_72: vec3<f32>;
    var local_73: vec3<f32>;
    var local_74: f32;
    var local_75: vec2<i32>;
    var local_76: vec2<f32>;
    var local_77: vec2<i32>;
    var local_78: f32;
    var local_79: f32;
    var local_80: bool;
    var local_81: vec2<i32>;
    var local_82: vec2<i32>;
    var local_83: vec3<f32>;
    var local_84: vec3<f32>;
    var local_85: vec2<i32>;
    var local_86: vec2<i32>;
    var local_87: vec2<f32>;
    var local_88: vec2<f32>;
    var local_89: bool;
    var local_90: vec4<f32>;
    var local_91: vec3<f32>;
    var local_92: vec4<f32>;
    var local_93: vec3<f32>;
    var local_94: vec4<f32>;
    var local_95: vec3<f32>;
    var local_96: vec3<f32>;
    var local_97: vec3<f32>;
    var local_98: mat4x4<f32>;
    var local_99: vec2<f32>;
    var local_100: vec2<f32>;
    var local_101: f32;
    var local_102: i32;
    var local_103: f32;
    var local_104: vec3<f32>;
    var local_105: mat4x4<f32>;
    var local_106: vec2<f32>;
    var local_107: vec2<f32>;
    var local_108: vec2<f32>;
    var local_109: vec4<f32>;
    var local_110: vec2<f32>;
    var local_111: vec4<f32>;
    var local_112: vec2<f32>;
    var local_113: vec2<f32>;
    var local_114: vec4<f32>;
    var local_115: vec4<f32>;
    var horiz: f32;
    var param: vec2<f32>;
    var vert: f32;
    var param_1: vec2<f32>;
    var mask: vec2<f32>;
    var param_2: f32;
    var param_3: f32;
    var alpha: f32;
    var param_4: vec2<f32>;
    var xpos: vec3<f32>;
    var eyedir: vec3<f32>;
    var normal: vec3<f32>;
    var NdotV: f32;
    var reflected: vec3<f32>;
    var world_reflection: vec3<f32>;
    var fallback: vec4<f32>;
    var viewport_scale: vec2<f32>;
    var viewport_offset: vec2<f32>;
    var hit: bool;
    var hiz_iterations: u32;
    var coords: vec2<f32>;
    var param_5: vec3<f32>;
    var param_6: vec3<f32>;
    var param_7: mat4x4<f32>;
    var param_8: vec2<f32>;
    var param_9: vec2<f32>;
    var positionSS: vec3<f32>;
    var param_10: vec3<f32>;
    var param_11: mat4x4<f32>;
    var param_12: vec2<f32>;
    var param_13: vec2<f32>;
    var position2VS: vec3<f32>;
    var position2CS: vec4<f32>;
    var position2SS: vec3<f32>;
    var reflectionDirSS: vec3<f32>;
    var param_14: vec3<f32>;
    var param_15: vec3<f32>;
    var param_16: vec2<f32>;
    var viewport_coords: vec2<f32>;
    var result: vec4<f32>;
    var edge: f32;
    var param_17: vec2<f32>;
    var param_18: vec2<f32>;
    var param_19: vec2<f32>;
    var fresnel: f32;
    var blend_weight: f32;
    var phi_1440_: bool;
    var phi_1494_: bool;
    var phi_1504_: bool;
    var phi_978_: bool;
    var phi_985_: bool;
    var phi_992_: bool;
    var phi_999_: bool;

    let _e215 = f_uv_1;
    let _e217 = u_push_constants.m_displace_direction;
    param = (_e215 + (_e217.xy * 150f));
    let _e221 = param;
    let _e222 = textureSample(f_mesh_texture_2_image, f_mesh_texture_2_sampler, _e221);
    local_115 = _e222;
    let _e223 = local_115;
    horiz = _e223.x;
    let _e225 = f_uv_1;
    let _e228 = u_push_constants.m_displace_direction;
    param_1 = ((_e225.yx + (_e228.zw * 150f)) * vec2<f32>(0.9f, 0.9f));
    let _e233 = param_1;
    let _e234 = textureSample(f_mesh_texture_2_image, f_mesh_texture_2_sampler, _e233);
    local_114 = _e234;
    let _e235 = local_114;
    vert = _e235.x;
    let _e237 = horiz;
    param_2 = _e237;
    let _e238 = vert;
    param_3 = _e238;
    let _e239 = param_2;
    let _e240 = param_3;
    local_110 = vec2<f32>(_e239, _e240);
    let _e242 = local_110;
    local_110 = ((_e242 * 2f) - vec2(1f));
    let _e247 = local_110[0u];
    let _e250 = local_110[0u];
    local_111[0u] = (step(_e247, 0f) * -(_e250));
    let _e255 = local_110[0u];
    let _e258 = local_110[0u];
    local_111[1u] = (step(0f, _e255) * _e258);
    let _e262 = local_110[1u];
    let _e265 = local_110[1u];
    local_111[2u] = (step(_e262, 0f) * -(_e265));
    let _e270 = local_110[1u];
    let _e273 = local_110[1u];
    local_111[3u] = (step(0f, _e270) * _e273);
    let _e277 = local_111[0u];
    let _e280 = local_111[1u];
    local_112[0u] = (-(_e277) + _e280);
    let _e284 = local_111[2u];
    let _e287 = local_111[3u];
    local_112[1u] = (-(_e284) + _e287);
    let _e290 = local_112;
    local_113 = _e290;
    let _e291 = local_113;
    mask = _e291;
    let _e292 = mask;
    mask = ((_e292 + vec2(1f)) * 0.5f);
    let _e296 = mask;
    o_displace_mask = _e296;
    if u_ssr {
        let _e297 = f_uv_1;
        param_4 = _e297;
        let _e298 = param_4;
        let _e299 = textureSample(f_mesh_texture_0_image, f_mesh_texture_0_sampler, _e298);
        local_109 = _e299;
        let _e300 = local_109;
        alpha = _e300.w;
        let _e302 = alpha;
        if (_e302 == 0f) {
            o_displace_ssr = vec4<f32>(0f, 0f, 0f, 0f);
            return;
        }
        let _e305 = u_camera.m_view_matrix;
        let _e306 = f_world_position_1;
        xpos = (_e305 * _e306).xyz;
        let _e309 = xpos;
        eyedir = -(normalize(_e309));
        let _e313 = u_camera.m_view_matrix;
        let _e314 = f_normal_1;
        let _e315 = normalize(_e314);
        normal = (_e313 * vec4<f32>(_e315.x, _e315.y, _e315.z, 0f)).xyz;
        let _e322 = normal;
        let _e323 = eyedir;
        NdotV = dot(_e322, _e323);
        let _e325 = NdotV;
        if (_e325 <= 0f) {
            o_displace_ssr = vec4<f32>(0f, 0f, 0f, 0f);
            return;
        }
        let _e327 = eyedir;
        let _e329 = normal;
        reflected = reflect(-(_e327), _e329);
        let _e332 = u_camera.m_inverse_view_matrix;
        let _e333 = reflected;
        world_reflection = (_e332 * vec4<f32>(_e333.x, _e333.y, _e333.z, 0f)).xyz;
        let _e340 = world_reflection;
        let _e341 = textureSample(u_skybox_texture_image, u_skybox_texture_sampler, _e340);
        fallback = _e341;
        let _e343 = normal[2u];
        if (_e343 < -0.75f) {
            let _e345 = fallback;
            o_displace_ssr = _e345;
            return;
        }
        let _e347 = u_camera.m_viewport;
        let _e350 = u_camera.m_screensize;
        viewport_scale = (_e347.zw / _e350);
        let _e353 = u_camera.m_viewport;
        let _e356 = u_camera.m_screensize;
        viewport_offset = (_e353.xy / _e356);
        hit = true;
        hiz_iterations = u_hiz_iterations;
        let _e358 = hiz_iterations;
        if (_e358 == 0u) {
            let _e360 = reflected;
            param_5 = _e360;
            let _e361 = xpos;
            param_6 = _e361;
            let _e363 = u_camera.m_projection_matrix;
            param_7 = _e363;
            let _e364 = viewport_scale;
            param_8 = _e364;
            let _e365 = viewport_offset;
            param_9 = _e365;
            let _e366 = param_5;
            param_5 = (_e366 * 0.5f);
            let _e368 = param_5;
            let _e369 = param_6;
            param_6 = (_e369 + _e368);
            let _e371 = param_6;
            local_97 = _e371;
            let _e372 = param_7;
            local_98 = _e372;
            let _e373 = param_8;
            local_99 = _e373;
            let _e374 = param_9;
            local_100 = _e374;
            let _e375 = local_98;
            let _e376 = local_97;
            local_94 = (_e375 * vec4<f32>(_e376.x, _e376.y, _e376.z, 1f));
            let _e383 = local_94[3u];
            let _e384 = local_94;
            let _e387 = (_e384.xyz / vec3(_e383));
            local_94[0u] = _e387.x;
            local_94[1u] = _e387.y;
            local_94[2u] = _e387.z;
            let _e394 = local_94;
            let _e398 = ((_e394.xy * 0.5f) + vec2(0.5f));
            local_94[0u] = _e398.x;
            local_94[1u] = _e398.y;
            let _e403 = local_94;
            let _e405 = local_99;
            let _e407 = local_100;
            let _e408 = ((_e403.xy * _e405) + _e407);
            local_94[0u] = _e408.x;
            local_94[1u] = _e408.y;
            let _e413 = local_94;
            local_95 = _e413.xyz;
            let _e415 = local_95;
            local_96 = _e415;
            local_101 = 1f;
            local_102 = 0i;
            loop {
                let _e416 = local_102;
                if (_e416 < 32i) {
                    let _e418 = local_96;
                    let _e421 = textureSampleCompare(u_depth_image, u_depth_sampler, _e418.xy, _e418.z);
                    local_103 = _e421;
                    let _e422 = local_103;
                    let _e423 = local_101;
                    local_101 = (_e423 * _e422);
                    let _e425 = param_5;
                    let _e426 = local_101;
                    param_5 = (_e425 * (0.5f + (0.5f * _e426)));
                    let _e430 = param_5;
                    let _e431 = local_103;
                    let _e435 = param_6;
                    param_6 = (_e435 + (_e430 * ((2f * _e431) - 1f)));
                    let _e437 = param_6;
                    local_104 = _e437;
                    let _e438 = param_7;
                    local_105 = _e438;
                    let _e439 = param_8;
                    local_106 = _e439;
                    let _e440 = param_9;
                    local_107 = _e440;
                    let _e441 = local_105;
                    let _e442 = local_104;
                    local_92 = (_e441 * vec4<f32>(_e442.x, _e442.y, _e442.z, 1f));
                    let _e449 = local_92[3u];
                    let _e450 = local_92;
                    let _e453 = (_e450.xyz / vec3(_e449));
                    local_92[0u] = _e453.x;
                    local_92[1u] = _e453.y;
                    local_92[2u] = _e453.z;
                    let _e460 = local_92;
                    let _e464 = ((_e460.xy * 0.5f) + vec2(0.5f));
                    local_92[0u] = _e464.x;
                    local_92[1u] = _e464.y;
                    let _e469 = local_92;
                    let _e471 = local_106;
                    let _e473 = local_107;
                    let _e474 = ((_e469.xy * _e471) + _e473);
                    local_92[0u] = _e474.x;
                    local_92[1u] = _e474.y;
                    let _e479 = local_92;
                    local_93 = _e479.xyz;
                    let _e481 = local_93;
                    local_96 = _e481;
                    continue;
                } else {
                    break;
                }
                continuing {
                    let _e482 = local_102;
                    local_102 = (_e482 + 1i);
                }
            }
            let _e484 = local_96;
            local_108 = _e484.xy;
            let _e486 = local_108;
            coords = _e486;
        } else {
            let _e487 = xpos;
            param_10 = _e487;
            let _e489 = u_camera.m_projection_matrix;
            param_11 = _e489;
            param_12 = vec2<f32>(1f, 1f);
            param_13 = vec2<f32>(0f, 0f);
            let _e490 = param_11;
            let _e491 = param_10;
            local_90 = (_e490 * vec4<f32>(_e491.x, _e491.y, _e491.z, 1f));
            let _e498 = local_90[3u];
            let _e499 = local_90;
            let _e502 = (_e499.xyz / vec3(_e498));
            local_90[0u] = _e502.x;
            local_90[1u] = _e502.y;
            local_90[2u] = _e502.z;
            let _e509 = local_90;
            let _e513 = ((_e509.xy * 0.5f) + vec2(0.5f));
            local_90[0u] = _e513.x;
            local_90[1u] = _e513.y;
            let _e518 = local_90;
            let _e520 = param_12;
            let _e522 = param_13;
            let _e523 = ((_e518.xy * _e520) + _e522);
            local_90[0u] = _e523.x;
            local_90[1u] = _e523.y;
            let _e528 = local_90;
            local_91 = _e528.xyz;
            let _e530 = local_91;
            positionSS = _e530;
            let _e531 = xpos;
            let _e532 = reflected;
            position2VS = (_e531 + (_e532 * 1000f));
            let _e536 = u_camera.m_projection_matrix;
            let _e537 = position2VS;
            position2CS = (_e536 * vec4<f32>(_e537.x, _e537.y, _e537.z, 1f));
            let _e544 = position2CS[3u];
            let _e545 = position2CS;
            position2CS = (_e545 / vec4(_e544));
            let _e548 = position2CS;
            position2SS = _e548.xyz;
            let _e550 = position2SS;
            let _e553 = (vec2<f32>(0.5f, 0.5f) + (_e550.xy * 0.5f));
            position2SS[0u] = _e553.x;
            position2SS[1u] = _e553.y;
            let _e558 = position2SS;
            let _e559 = positionSS;
            reflectionDirSS = normalize((_e558 - _e559));
            let _e562 = positionSS;
            param_14 = _e562;
            let _e563 = reflectionDirSS;
            param_15 = _e563;
            let _e564 = textureNumLevels(u_hiz_depth_image);
            local_36 = min(6i, (i32(_e564) - 1i));
            let _e568 = param_14;
            local_38 = _e568;
            let _e569 = param_15;
            local_39 = _e569;
            let _e571 = local_39[0u];
            if (_e571 < 0f) {
                let _e574 = local_38[0u];
                let _e576 = local_39[0u];
                local_34[0u] = (_e574 / -(_e576));
            } else {
                let _e581 = local_38[0u];
                let _e584 = local_39[0u];
                local_34[0u] = ((1f - _e581) / _e584);
            }
            let _e588 = local_39[1u];
            if (_e588 < 0f) {
                let _e591 = local_38[1u];
                let _e593 = local_39[1u];
                local_34[1u] = (_e591 / -(_e593));
            } else {
                let _e598 = local_38[1u];
                let _e601 = local_39[1u];
                local_34[1u] = ((1f - _e598) / _e601);
            }
            let _e605 = local_39[2u];
            if (_e605 < 0f) {
                let _e608 = local_38[2u];
                let _e610 = local_39[2u];
                local_34[2u] = (_e608 / -(_e610));
            } else {
                let _e615 = local_38[2u];
                let _e618 = local_39[2u];
                local_34[2u] = ((1f - _e615) / _e618);
            }
            let _e622 = local_34[0u];
            let _e624 = local_34[1u];
            let _e626 = local_34[2u];
            local_35 = min(_e622, min(_e624, _e626));
            let _e629 = local_35;
            local_37 = _e629;
            let _e631 = param_15[0u];
            let _e636 = param_15[1u];
            local_40 = vec2<f32>(f32(select(-1i, 1i, (_e631 >= 0f))), f32(select(-1i, 1i, (_e636 >= 0f))));
            let _e641 = local_40;
            let _e643 = u_camera.m_viewport;
            local_41 = ((_e641 / _e643.zw) / vec2(128f));
            let _e648 = local_40;
            local_40 = clamp(_e648, vec2(0f), vec2(1f));
            let _e652 = param_14;
            local_42 = _e652;
            let _e654 = local_42[2u];
            local_43 = _e654;
            let _e656 = local_42[2u];
            let _e658 = param_15[2u];
            let _e659 = local_37;
            local_44 = (_e656 + (_e658 * _e659));
            let _e662 = local_44;
            let _e663 = local_43;
            local_45 = (_e662 - _e663);
            let _e665 = local_42;
            local_46 = _e665;
            let _e666 = param_15;
            let _e667 = local_37;
            local_47 = (_e666 * _e667);
            local_48 = 0i;
            local_49 = 0u;
            let _e670 = param_15[2u];
            local_50 = (_e670 < 0f);
            let _e672 = local_50;
            local_51 = select(1f, -1f, _e672);
            let _e674 = local_48;
            local_53 = _e674;
            let _e675 = local_53;
            let _e676 = textureDimensions(u_hiz_depth_image, _e675);
            local_33 = vec2<i32>(_e676);
            let _e678 = local_33;
            local_52 = _e678;
            let _e679 = local_42;
            local_55 = _e679.xy;
            let _e681 = local_52;
            local_56 = _e681;
            let _e682 = local_55;
            let _e683 = local_56;
            local_32 = vec2<i32>((_e682 * vec2<f32>(_e683)));
            let _e687 = local_32;
            local_54 = _e687;
            let _e688 = local_41;
            let _e690 = local_46;
            local_57 = _e690;
            let _e691 = local_47;
            local_58 = _e691;
            let _e692 = local_54;
            local_59 = _e692;
            let _e693 = local_52;
            local_60 = _e693;
            let _e694 = local_40;
            local_61 = _e694;
            local_62 = (_e688 * 64f);
            local_23 = vec3<f32>(0f, 0f, 0f);
            let _e695 = local_59;
            let _e697 = local_61;
            local_24 = (vec2<f32>(_e695) + _e697);
            let _e699 = local_24;
            let _e700 = local_60;
            local_25 = (_e699 / vec2<f32>(_e700));
            let _e703 = local_62;
            let _e704 = local_25;
            local_25 = (_e704 + _e703);
            let _e706 = local_25;
            let _e707 = local_57;
            local_26 = (_e706 - _e707.xy);
            let _e710 = local_58;
            let _e712 = local_26;
            local_26 = (_e712 / _e710.xy);
            let _e715 = local_26[0u];
            let _e717 = local_26[1u];
            local_27 = min(_e715, _e717);
            let _e719 = local_57;
            local_28 = _e719;
            let _e720 = local_58;
            local_29 = _e720;
            let _e721 = local_27;
            local_30 = _e721;
            let _e722 = local_28;
            let _e723 = local_29;
            let _e724 = local_30;
            local_22 = (_e722 + (_e723 * _e724));
            let _e727 = local_22;
            local_23 = _e727;
            let _e728 = local_23;
            local_31 = _e728;
            let _e729 = local_31;
            local_42 = _e729;
            loop {
                let _e730 = local_48;
                let _e731 = (_e730 >= 0i);
                phi_1440_ = _e731;
                if _e731 {
                    let _e733 = local_42[2u];
                    let _e734 = local_51;
                    let _e736 = local_44;
                    let _e737 = local_51;
                    phi_1440_ = ((_e733 * _e734) <= (_e736 * _e737));
                }
                let _e741 = phi_1440_;
                let _e742 = local_49;
                if (_e741 && (_e742 < u_hiz_iterations)) {
                    let _e745 = local_48;
                    local_64 = _e745;
                    let _e746 = local_64;
                    let _e747 = textureDimensions(u_hiz_depth_image, _e746);
                    local_21 = vec2<i32>(_e747);
                    let _e749 = local_21;
                    local_63 = _e749;
                    let _e750 = local_42;
                    local_66 = _e750.xy;
                    let _e752 = local_63;
                    local_67 = _e752;
                    let _e753 = local_66;
                    let _e754 = local_67;
                    local_20 = vec2<i32>((_e753 * vec2<f32>(_e754)));
                    let _e758 = local_20;
                    local_65 = _e758;
                    let _e759 = local_65;
                    local_69 = _e759;
                    let _e760 = local_48;
                    local_70 = _e760;
                    let _e761 = local_69;
                    let _e762 = local_70;
                    let _e763 = textureLoad(u_hiz_depth_image, _e761, _e762);
                    local_19 = _e763.x;
                    let _e765 = local_19;
                    local_68 = _e765;
                    let _e766 = local_68;
                    let _e768 = local_42[2u];
                    let _e770 = local_50;
                    if ((_e766 > _e768) && !(_e770)) {
                        let _e773 = local_68;
                        let _e774 = local_43;
                        let _e776 = local_45;
                        let _e778 = local_46;
                        local_72 = _e778;
                        let _e779 = local_47;
                        local_73 = _e779;
                        local_74 = ((_e773 - _e774) / _e776);
                        let _e780 = local_72;
                        let _e781 = local_73;
                        let _e782 = local_74;
                        local_18 = (_e780 + (_e781 * _e782));
                        let _e785 = local_18;
                        local_71 = _e785;
                    } else {
                        let _e786 = local_42;
                        local_71 = _e786;
                    }
                    let _e787 = local_71;
                    local_76 = _e787.xy;
                    let _e789 = local_63;
                    local_77 = _e789;
                    let _e790 = local_76;
                    let _e791 = local_77;
                    local_17 = vec2<i32>((_e790 * vec2<f32>(_e791)));
                    let _e795 = local_17;
                    local_75 = _e795;
                    let _e796 = local_48;
                    if (_e796 == 0i) {
                        let _e799 = local_42[2u];
                        let _e800 = local_68;
                        local_79 = (_e799 - _e800);
                    } else {
                        local_79 = 0f;
                    }
                    let _e802 = local_79;
                    local_78 = _e802;
                    let _e803 = local_50;
                    phi_1494_ = _e803;
                    if _e803 {
                        let _e804 = local_68;
                        let _e806 = local_42[2u];
                        phi_1494_ = (_e804 > _e806);
                    }
                    let _e809 = phi_1494_;
                    let _e810 = local_78;
                    let _e812 = (_e809 || (_e810 > 0.001f));
                    phi_1504_ = _e812;
                    if !(_e812) {
                        let _e814 = local_65;
                        local_81 = _e814;
                        let _e815 = local_75;
                        local_82 = _e815;
                        let _e816 = local_81;
                        let _e817 = local_82;
                        local_16 = any((_e816 != _e817));
                        let _e820 = local_16;
                        phi_1504_ = _e820;
                    }
                    let _e822 = phi_1504_;
                    local_80 = _e822;
                    let _e823 = local_80;
                    if _e823 {
                        let _e824 = local_46;
                        local_83 = _e824;
                        let _e825 = local_47;
                        local_84 = _e825;
                        let _e826 = local_65;
                        local_85 = _e826;
                        let _e827 = local_63;
                        local_86 = _e827;
                        let _e828 = local_40;
                        local_87 = _e828;
                        let _e829 = local_41;
                        local_88 = _e829;
                        local_7 = vec3<f32>(0f, 0f, 0f);
                        let _e830 = local_85;
                        let _e832 = local_87;
                        local_8 = (vec2<f32>(_e830) + _e832);
                        let _e834 = local_8;
                        let _e835 = local_86;
                        local_9 = (_e834 / vec2<f32>(_e835));
                        let _e838 = local_88;
                        let _e839 = local_9;
                        local_9 = (_e839 + _e838);
                        let _e841 = local_9;
                        let _e842 = local_83;
                        local_10 = (_e841 - _e842.xy);
                        let _e845 = local_84;
                        let _e847 = local_10;
                        local_10 = (_e847 / _e845.xy);
                        let _e850 = local_10[0u];
                        let _e852 = local_10[1u];
                        local_11 = min(_e850, _e852);
                        let _e854 = local_83;
                        local_12 = _e854;
                        let _e855 = local_84;
                        local_13 = _e855;
                        let _e856 = local_11;
                        local_14 = _e856;
                        let _e857 = local_12;
                        let _e858 = local_13;
                        let _e859 = local_14;
                        local_6 = (_e857 + (_e858 * _e859));
                        let _e862 = local_6;
                        local_7 = _e862;
                        let _e863 = local_7;
                        local_15 = _e863;
                        let _e864 = local_15;
                        local_42 = _e864;
                        let _e865 = local_36;
                        let _e866 = local_48;
                        local_48 = min(_e865, (_e866 + 1i));
                    } else {
                        let _e869 = local_71;
                        local_42 = _e869;
                        let _e870 = local_48;
                        local_48 = (_e870 - 1i);
                    }
                    let _e872 = local_49;
                    local_49 = (_e872 + 1u);
                    continue;
                } else {
                    break;
                }
            }
            let _e874 = local_42;
            param_16 = _e874.xy;
            let _e876 = local_48;
            let _e878 = local_49;
            local_89 = ((_e876 < 0i) && (_e878 < u_hiz_iterations));
            let _e881 = local_89;
            let _e882 = param_16;
            coords = _e882;
            hit = _e881;
            let _e883 = coords;
            let _e884 = viewport_scale;
            let _e886 = viewport_offset;
            coords = ((_e883 * _e884) + _e886);
        }
        let _e888 = coords;
        let _e889 = viewport_offset;
        let _e891 = viewport_scale;
        viewport_coords = ((_e888 - _e889) / _e891);
        let _e893 = hit;
        let _e894 = !(_e893);
        phi_978_ = _e894;
        if !(_e894) {
            let _e897 = viewport_coords[0u];
            phi_978_ = (_e897 < 0f);
        }
        let _e900 = phi_978_;
        phi_985_ = _e900;
        if !(_e900) {
            let _e903 = viewport_coords[0u];
            phi_985_ = (_e903 > 1f);
        }
        let _e906 = phi_985_;
        phi_992_ = _e906;
        if !(_e906) {
            let _e909 = viewport_coords[1u];
            phi_992_ = (_e909 < 0f);
        }
        let _e912 = phi_992_;
        phi_999_ = _e912;
        if !(_e912) {
            let _e915 = viewport_coords[1u];
            phi_999_ = (_e915 > 1f);
        }
        let _e918 = phi_999_;
        if _e918 {
            let _e919 = fallback;
            result = _e919;
        } else {
            let _e920 = coords;
            let _e921 = textureSample(u_displace_color_image, u_displace_color_sampler, _e920);
            result = _e921;
            let _e922 = coords;
            param_17 = _e922;
            let _e923 = viewport_scale;
            param_18 = _e923;
            let _e924 = viewport_offset;
            param_19 = _e924;
            let _e925 = param_17;
            let _e926 = param_19;
            let _e928 = param_18;
            local = ((_e925 - _e926) / _e928);
            let _e931 = local[0u];
            local_1 = smoothstep(0f, 0.4f, _e931);
            let _e934 = local[0u];
            local_2 = (1f - smoothstep(0.6f, 1f, _e934));
            let _e938 = local[1u];
            local_3 = smoothstep(0f, 0.4f, _e938);
            let _e941 = local[1u];
            local_4 = (1f - smoothstep(0.6f, 1f, _e941));
            let _e944 = local_1;
            let _e945 = local_2;
            let _e947 = local_3;
            let _e948 = local_4;
            local_5 = min(min(_e944, _e945), min(_e947, _e948));
            let _e951 = local_5;
            edge = _e951;
            let _e952 = NdotV;
            let _e954 = NdotV;
            fresnel = ((1f - _e952) * (1f - _e954));
            let _e957 = edge;
            let _e958 = fresnel;
            blend_weight = (_e957 * _e958);
            let _e960 = fallback;
            let _e961 = result;
            let _e962 = blend_weight;
            result = mix(_e960, _e961, vec4(_e962));
        }
        let _e965 = result;
        o_displace_ssr = _e965;
    }
    return;
}

@fragment 
fn main(@location(1) f_uv: vec2<f32>, @location(8) f_world_position: vec4<f32>, @location(5) f_normal: vec3<f32>) -> FragmentOutput {
    f_uv_1 = f_uv;
    f_world_position_1 = f_world_position;
    f_normal_1 = f_normal;
    main_1();
    let _e8 = o_displace_mask;
    let _e9 = o_displace_ssr;
    return FragmentOutput(_e8, _e9);
}
