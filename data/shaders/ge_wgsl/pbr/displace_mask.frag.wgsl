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

const u_hiz_iterations: u32 = 0u;
const u_ssr: bool = false;

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
    var phi_1439_: bool;
    var phi_1493_: bool;
    var phi_1503_: bool;
    var phi_976_: bool;
    var phi_983_: bool;
    var phi_990_: bool;
    var phi_997_: bool;

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
        if true {
            let _e358 = reflected;
            param_5 = _e358;
            let _e359 = xpos;
            param_6 = _e359;
            let _e361 = u_camera.m_projection_matrix;
            param_7 = _e361;
            let _e362 = viewport_scale;
            param_8 = _e362;
            let _e363 = viewport_offset;
            param_9 = _e363;
            let _e364 = param_5;
            param_5 = (_e364 * 0.5f);
            let _e366 = param_5;
            let _e367 = param_6;
            param_6 = (_e367 + _e366);
            let _e369 = param_6;
            local_97 = _e369;
            let _e370 = param_7;
            local_98 = _e370;
            let _e371 = param_8;
            local_99 = _e371;
            let _e372 = param_9;
            local_100 = _e372;
            let _e373 = local_98;
            let _e374 = local_97;
            local_94 = (_e373 * vec4<f32>(_e374.x, _e374.y, _e374.z, 1f));
            let _e381 = local_94[3u];
            let _e382 = local_94;
            let _e385 = (_e382.xyz / vec3(_e381));
            local_94[0u] = _e385.x;
            local_94[1u] = _e385.y;
            local_94[2u] = _e385.z;
            let _e392 = local_94;
            let _e396 = ((_e392.xy * 0.5f) + vec2(0.5f));
            local_94[0u] = _e396.x;
            local_94[1u] = _e396.y;
            let _e401 = local_94;
            let _e403 = local_99;
            let _e405 = local_100;
            let _e406 = ((_e401.xy * _e403) + _e405);
            local_94[0u] = _e406.x;
            local_94[1u] = _e406.y;
            let _e411 = local_94;
            local_95 = _e411.xyz;
            let _e413 = local_95;
            local_96 = _e413;
            local_101 = 1f;
            local_102 = 0i;
            loop {
                let _e414 = local_102;
                if (_e414 < 32i) {
                    let _e416 = local_96;
                    let _e419 = textureSampleCompare(u_depth_image, u_depth_sampler, _e416.xy, _e416.z);
                    local_103 = _e419;
                    let _e420 = local_103;
                    let _e421 = local_101;
                    local_101 = (_e421 * _e420);
                    let _e423 = param_5;
                    let _e424 = local_101;
                    param_5 = (_e423 * (0.5f + (0.5f * _e424)));
                    let _e428 = param_5;
                    let _e429 = local_103;
                    let _e433 = param_6;
                    param_6 = (_e433 + (_e428 * ((2f * _e429) - 1f)));
                    let _e435 = param_6;
                    local_104 = _e435;
                    let _e436 = param_7;
                    local_105 = _e436;
                    let _e437 = param_8;
                    local_106 = _e437;
                    let _e438 = param_9;
                    local_107 = _e438;
                    let _e439 = local_105;
                    let _e440 = local_104;
                    local_92 = (_e439 * vec4<f32>(_e440.x, _e440.y, _e440.z, 1f));
                    let _e447 = local_92[3u];
                    let _e448 = local_92;
                    let _e451 = (_e448.xyz / vec3(_e447));
                    local_92[0u] = _e451.x;
                    local_92[1u] = _e451.y;
                    local_92[2u] = _e451.z;
                    let _e458 = local_92;
                    let _e462 = ((_e458.xy * 0.5f) + vec2(0.5f));
                    local_92[0u] = _e462.x;
                    local_92[1u] = _e462.y;
                    let _e467 = local_92;
                    let _e469 = local_106;
                    let _e471 = local_107;
                    let _e472 = ((_e467.xy * _e469) + _e471);
                    local_92[0u] = _e472.x;
                    local_92[1u] = _e472.y;
                    let _e477 = local_92;
                    local_93 = _e477.xyz;
                    let _e479 = local_93;
                    local_96 = _e479;
                    continue;
                } else {
                    break;
                }
                continuing {
                    let _e480 = local_102;
                    local_102 = (_e480 + 1i);
                }
            }
            let _e482 = local_96;
            local_108 = _e482.xy;
            let _e484 = local_108;
            coords = _e484;
        } else {
            let _e485 = xpos;
            param_10 = _e485;
            let _e487 = u_camera.m_projection_matrix;
            param_11 = _e487;
            param_12 = vec2<f32>(1f, 1f);
            param_13 = vec2<f32>(0f, 0f);
            let _e488 = param_11;
            let _e489 = param_10;
            local_90 = (_e488 * vec4<f32>(_e489.x, _e489.y, _e489.z, 1f));
            let _e496 = local_90[3u];
            let _e497 = local_90;
            let _e500 = (_e497.xyz / vec3(_e496));
            local_90[0u] = _e500.x;
            local_90[1u] = _e500.y;
            local_90[2u] = _e500.z;
            let _e507 = local_90;
            let _e511 = ((_e507.xy * 0.5f) + vec2(0.5f));
            local_90[0u] = _e511.x;
            local_90[1u] = _e511.y;
            let _e516 = local_90;
            let _e518 = param_12;
            let _e520 = param_13;
            let _e521 = ((_e516.xy * _e518) + _e520);
            local_90[0u] = _e521.x;
            local_90[1u] = _e521.y;
            let _e526 = local_90;
            local_91 = _e526.xyz;
            let _e528 = local_91;
            positionSS = _e528;
            let _e529 = xpos;
            let _e530 = reflected;
            position2VS = (_e529 + (_e530 * 1000f));
            let _e534 = u_camera.m_projection_matrix;
            let _e535 = position2VS;
            position2CS = (_e534 * vec4<f32>(_e535.x, _e535.y, _e535.z, 1f));
            let _e542 = position2CS[3u];
            let _e543 = position2CS;
            position2CS = (_e543 / vec4(_e542));
            let _e546 = position2CS;
            position2SS = _e546.xyz;
            let _e548 = position2SS;
            let _e551 = (vec2<f32>(0.5f, 0.5f) + (_e548.xy * 0.5f));
            position2SS[0u] = _e551.x;
            position2SS[1u] = _e551.y;
            let _e556 = position2SS;
            let _e557 = positionSS;
            reflectionDirSS = normalize((_e556 - _e557));
            let _e560 = positionSS;
            param_14 = _e560;
            let _e561 = reflectionDirSS;
            param_15 = _e561;
            let _e562 = textureNumLevels(u_hiz_depth_image);
            local_36 = min(6i, (i32(_e562) - 1i));
            let _e566 = param_14;
            local_38 = _e566;
            let _e567 = param_15;
            local_39 = _e567;
            let _e569 = local_39[0u];
            if (_e569 < 0f) {
                let _e572 = local_38[0u];
                let _e574 = local_39[0u];
                local_34[0u] = (_e572 / -(_e574));
            } else {
                let _e579 = local_38[0u];
                let _e582 = local_39[0u];
                local_34[0u] = ((1f - _e579) / _e582);
            }
            let _e586 = local_39[1u];
            if (_e586 < 0f) {
                let _e589 = local_38[1u];
                let _e591 = local_39[1u];
                local_34[1u] = (_e589 / -(_e591));
            } else {
                let _e596 = local_38[1u];
                let _e599 = local_39[1u];
                local_34[1u] = ((1f - _e596) / _e599);
            }
            let _e603 = local_39[2u];
            if (_e603 < 0f) {
                let _e606 = local_38[2u];
                let _e608 = local_39[2u];
                local_34[2u] = (_e606 / -(_e608));
            } else {
                let _e613 = local_38[2u];
                let _e616 = local_39[2u];
                local_34[2u] = ((1f - _e613) / _e616);
            }
            let _e620 = local_34[0u];
            let _e622 = local_34[1u];
            let _e624 = local_34[2u];
            local_35 = min(_e620, min(_e622, _e624));
            let _e627 = local_35;
            local_37 = _e627;
            let _e629 = param_15[0u];
            let _e634 = param_15[1u];
            local_40 = vec2<f32>(f32(select(-1i, 1i, (_e629 >= 0f))), f32(select(-1i, 1i, (_e634 >= 0f))));
            let _e639 = local_40;
            let _e641 = u_camera.m_viewport;
            local_41 = ((_e639 / _e641.zw) / vec2(128f));
            let _e646 = local_40;
            local_40 = clamp(_e646, vec2(0f), vec2(1f));
            let _e650 = param_14;
            local_42 = _e650;
            let _e652 = local_42[2u];
            local_43 = _e652;
            let _e654 = local_42[2u];
            let _e656 = param_15[2u];
            let _e657 = local_37;
            local_44 = (_e654 + (_e656 * _e657));
            let _e660 = local_44;
            let _e661 = local_43;
            local_45 = (_e660 - _e661);
            let _e663 = local_42;
            local_46 = _e663;
            let _e664 = param_15;
            let _e665 = local_37;
            local_47 = (_e664 * _e665);
            local_48 = 0i;
            local_49 = 0u;
            let _e668 = param_15[2u];
            local_50 = (_e668 < 0f);
            let _e670 = local_50;
            local_51 = select(1f, -1f, _e670);
            let _e672 = local_48;
            local_53 = _e672;
            let _e673 = local_53;
            let _e674 = textureDimensions(u_hiz_depth_image, _e673);
            local_33 = vec2<i32>(_e674);
            let _e676 = local_33;
            local_52 = _e676;
            let _e677 = local_42;
            local_55 = _e677.xy;
            let _e679 = local_52;
            local_56 = _e679;
            let _e680 = local_55;
            let _e681 = local_56;
            local_32 = vec2<i32>((_e680 * vec2<f32>(_e681)));
            let _e685 = local_32;
            local_54 = _e685;
            let _e686 = local_41;
            let _e688 = local_46;
            local_57 = _e688;
            let _e689 = local_47;
            local_58 = _e689;
            let _e690 = local_54;
            local_59 = _e690;
            let _e691 = local_52;
            local_60 = _e691;
            let _e692 = local_40;
            local_61 = _e692;
            local_62 = (_e686 * 64f);
            local_23 = vec3<f32>(0f, 0f, 0f);
            let _e693 = local_59;
            let _e695 = local_61;
            local_24 = (vec2<f32>(_e693) + _e695);
            let _e697 = local_24;
            let _e698 = local_60;
            local_25 = (_e697 / vec2<f32>(_e698));
            let _e701 = local_62;
            let _e702 = local_25;
            local_25 = (_e702 + _e701);
            let _e704 = local_25;
            let _e705 = local_57;
            local_26 = (_e704 - _e705.xy);
            let _e708 = local_58;
            let _e710 = local_26;
            local_26 = (_e710 / _e708.xy);
            let _e713 = local_26[0u];
            let _e715 = local_26[1u];
            local_27 = min(_e713, _e715);
            let _e717 = local_57;
            local_28 = _e717;
            let _e718 = local_58;
            local_29 = _e718;
            let _e719 = local_27;
            local_30 = _e719;
            let _e720 = local_28;
            let _e721 = local_29;
            let _e722 = local_30;
            local_22 = (_e720 + (_e721 * _e722));
            let _e725 = local_22;
            local_23 = _e725;
            let _e726 = local_23;
            local_31 = _e726;
            let _e727 = local_31;
            local_42 = _e727;
            loop {
                let _e728 = local_48;
                let _e729 = (_e728 >= 0i);
                phi_1439_ = _e729;
                if _e729 {
                    let _e731 = local_42[2u];
                    let _e732 = local_51;
                    let _e734 = local_44;
                    let _e735 = local_51;
                    phi_1439_ = ((_e731 * _e732) <= (_e734 * _e735));
                }
                let _e739 = phi_1439_;
                let _e740 = local_49;
                if (_e739 && (_e740 < u_hiz_iterations)) {
                    let _e743 = local_48;
                    local_64 = _e743;
                    let _e744 = local_64;
                    let _e745 = textureDimensions(u_hiz_depth_image, _e744);
                    local_21 = vec2<i32>(_e745);
                    let _e747 = local_21;
                    local_63 = _e747;
                    let _e748 = local_42;
                    local_66 = _e748.xy;
                    let _e750 = local_63;
                    local_67 = _e750;
                    let _e751 = local_66;
                    let _e752 = local_67;
                    local_20 = vec2<i32>((_e751 * vec2<f32>(_e752)));
                    let _e756 = local_20;
                    local_65 = _e756;
                    let _e757 = local_65;
                    local_69 = _e757;
                    let _e758 = local_48;
                    local_70 = _e758;
                    let _e759 = local_69;
                    let _e760 = local_70;
                    let _e761 = textureLoad(u_hiz_depth_image, _e759, _e760);
                    local_19 = _e761.x;
                    let _e763 = local_19;
                    local_68 = _e763;
                    let _e764 = local_68;
                    let _e766 = local_42[2u];
                    let _e768 = local_50;
                    if ((_e764 > _e766) && !(_e768)) {
                        let _e771 = local_68;
                        let _e772 = local_43;
                        let _e774 = local_45;
                        let _e776 = local_46;
                        local_72 = _e776;
                        let _e777 = local_47;
                        local_73 = _e777;
                        local_74 = ((_e771 - _e772) / _e774);
                        let _e778 = local_72;
                        let _e779 = local_73;
                        let _e780 = local_74;
                        local_18 = (_e778 + (_e779 * _e780));
                        let _e783 = local_18;
                        local_71 = _e783;
                    } else {
                        let _e784 = local_42;
                        local_71 = _e784;
                    }
                    let _e785 = local_71;
                    local_76 = _e785.xy;
                    let _e787 = local_63;
                    local_77 = _e787;
                    let _e788 = local_76;
                    let _e789 = local_77;
                    local_17 = vec2<i32>((_e788 * vec2<f32>(_e789)));
                    let _e793 = local_17;
                    local_75 = _e793;
                    let _e794 = local_48;
                    if (_e794 == 0i) {
                        let _e797 = local_42[2u];
                        let _e798 = local_68;
                        local_79 = (_e797 - _e798);
                    } else {
                        local_79 = 0f;
                    }
                    let _e800 = local_79;
                    local_78 = _e800;
                    let _e801 = local_50;
                    phi_1493_ = _e801;
                    if _e801 {
                        let _e802 = local_68;
                        let _e804 = local_42[2u];
                        phi_1493_ = (_e802 > _e804);
                    }
                    let _e807 = phi_1493_;
                    let _e808 = local_78;
                    let _e810 = (_e807 || (_e808 > 0.001f));
                    phi_1503_ = _e810;
                    if !(_e810) {
                        let _e812 = local_65;
                        local_81 = _e812;
                        let _e813 = local_75;
                        local_82 = _e813;
                        let _e814 = local_81;
                        let _e815 = local_82;
                        local_16 = any((_e814 != _e815));
                        let _e818 = local_16;
                        phi_1503_ = _e818;
                    }
                    let _e820 = phi_1503_;
                    local_80 = _e820;
                    let _e821 = local_80;
                    if _e821 {
                        let _e822 = local_46;
                        local_83 = _e822;
                        let _e823 = local_47;
                        local_84 = _e823;
                        let _e824 = local_65;
                        local_85 = _e824;
                        let _e825 = local_63;
                        local_86 = _e825;
                        let _e826 = local_40;
                        local_87 = _e826;
                        let _e827 = local_41;
                        local_88 = _e827;
                        local_7 = vec3<f32>(0f, 0f, 0f);
                        let _e828 = local_85;
                        let _e830 = local_87;
                        local_8 = (vec2<f32>(_e828) + _e830);
                        let _e832 = local_8;
                        let _e833 = local_86;
                        local_9 = (_e832 / vec2<f32>(_e833));
                        let _e836 = local_88;
                        let _e837 = local_9;
                        local_9 = (_e837 + _e836);
                        let _e839 = local_9;
                        let _e840 = local_83;
                        local_10 = (_e839 - _e840.xy);
                        let _e843 = local_84;
                        let _e845 = local_10;
                        local_10 = (_e845 / _e843.xy);
                        let _e848 = local_10[0u];
                        let _e850 = local_10[1u];
                        local_11 = min(_e848, _e850);
                        let _e852 = local_83;
                        local_12 = _e852;
                        let _e853 = local_84;
                        local_13 = _e853;
                        let _e854 = local_11;
                        local_14 = _e854;
                        let _e855 = local_12;
                        let _e856 = local_13;
                        let _e857 = local_14;
                        local_6 = (_e855 + (_e856 * _e857));
                        let _e860 = local_6;
                        local_7 = _e860;
                        let _e861 = local_7;
                        local_15 = _e861;
                        let _e862 = local_15;
                        local_42 = _e862;
                        let _e863 = local_36;
                        let _e864 = local_48;
                        local_48 = min(_e863, (_e864 + 1i));
                    } else {
                        let _e867 = local_71;
                        local_42 = _e867;
                        let _e868 = local_48;
                        local_48 = (_e868 - 1i);
                    }
                    let _e870 = local_49;
                    local_49 = (_e870 + 1u);
                    continue;
                } else {
                    break;
                }
            }
            let _e872 = local_42;
            param_16 = _e872.xy;
            let _e874 = local_48;
            let _e876 = local_49;
            local_89 = ((_e874 < 0i) && (_e876 < u_hiz_iterations));
            let _e879 = local_89;
            let _e880 = param_16;
            coords = _e880;
            hit = _e879;
            let _e881 = coords;
            let _e882 = viewport_scale;
            let _e884 = viewport_offset;
            coords = ((_e881 * _e882) + _e884);
        }
        let _e886 = coords;
        let _e887 = viewport_offset;
        let _e889 = viewport_scale;
        viewport_coords = ((_e886 - _e887) / _e889);
        let _e891 = hit;
        let _e892 = !(_e891);
        phi_976_ = _e892;
        if !(_e892) {
            let _e895 = viewport_coords[0u];
            phi_976_ = (_e895 < 0f);
        }
        let _e898 = phi_976_;
        phi_983_ = _e898;
        if !(_e898) {
            let _e901 = viewport_coords[0u];
            phi_983_ = (_e901 > 1f);
        }
        let _e904 = phi_983_;
        phi_990_ = _e904;
        if !(_e904) {
            let _e907 = viewport_coords[1u];
            phi_990_ = (_e907 < 0f);
        }
        let _e910 = phi_990_;
        phi_997_ = _e910;
        if !(_e910) {
            let _e913 = viewport_coords[1u];
            phi_997_ = (_e913 > 1f);
        }
        let _e916 = phi_997_;
        if _e916 {
            let _e917 = fallback;
            result = _e917;
        } else {
            let _e918 = coords;
            let _e919 = textureSample(u_displace_color_image, u_displace_color_sampler, _e918);
            result = _e919;
            let _e920 = coords;
            param_17 = _e920;
            let _e921 = viewport_scale;
            param_18 = _e921;
            let _e922 = viewport_offset;
            param_19 = _e922;
            let _e923 = param_17;
            let _e924 = param_19;
            let _e926 = param_18;
            local = ((_e923 - _e924) / _e926);
            let _e929 = local[0u];
            local_1 = smoothstep(0f, 0.4f, _e929);
            let _e932 = local[0u];
            local_2 = (1f - smoothstep(0.6f, 1f, _e932));
            let _e936 = local[1u];
            local_3 = smoothstep(0f, 0.4f, _e936);
            let _e939 = local[1u];
            local_4 = (1f - smoothstep(0.6f, 1f, _e939));
            let _e942 = local_1;
            let _e943 = local_2;
            let _e945 = local_3;
            let _e946 = local_4;
            local_5 = min(min(_e942, _e943), min(_e945, _e946));
            let _e949 = local_5;
            edge = _e949;
            let _e950 = NdotV;
            let _e952 = NdotV;
            fresnel = ((1f - _e950) * (1f - _e952));
            let _e955 = edge;
            let _e956 = fresnel;
            blend_weight = (_e955 * _e956);
            let _e958 = fallback;
            let _e959 = result;
            let _e960 = blend_weight;
            result = mix(_e958, _e959, vec4(_e960));
        }
        let _e963 = result;
        o_displace_ssr = _e963;
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
