struct PushConstants {
    u_offset_miplevel: vec3<i32>,
}

var<private> gl_GlobalInvocationID_1: vec3<u32>;
@group(0) @binding(1) 
var u_hiz_depth: texture_storage_2d<r32float,write>;
@group(1) @binding(4)
var<uniform> pc: PushConstants;
@group(0) @binding(16) 
var u_depth_sampler: sampler;
@group(0) @binding(0) 
var u_depth_image: texture_2d<f32>;

fn main_1() {
    var dst: vec2<i32>;
    var current_size: vec2<i32>;
    var d: f32;
    var src: vec2<i32>;
    var prev_level: i32;
    var prev_size: vec2<i32>;
    var d0_: f32;
    var d1_: f32;
    var d2_: f32;
    var d3_: f32;
    var min_depth: f32;
    var extra_sample_x: bool;
    var extra_sample_y: bool;
    var d4_: f32;
    var d5_: f32;
    var d6_: f32;
    var d7_: f32;
    var d8_: f32;
    var phi_42_: bool;

    let _e40 = gl_GlobalInvocationID_1;
    dst = bitcast<vec2<i32>>(_e40.xy);
    let _e43 = textureDimensions(u_hiz_depth);
    current_size = vec2<i32>(_e43);
    let _e46 = dst[0u];
    let _e48 = current_size[0u];
    let _e49 = (_e46 >= _e48);
    phi_42_ = _e49;
    if !(_e49) {
        let _e52 = dst[1u];
        let _e54 = current_size[1u];
        phi_42_ = (_e52 >= _e54);
    }
    let _e57 = phi_42_;
    if _e57 {
        return;
    }
    let _e60 = pc.u_offset_miplevel[2u];
    if (_e60 == 0i) {
        let _e62 = dst;
        let _e64 = pc.u_offset_miplevel;
        let _e67 = textureLoad(u_depth_image, (_e62 + _e64.xy), 0i);
        d = _e67.x;
        let _e69 = dst;
        let _e70 = d;
        textureStore(u_hiz_depth, _e69, vec4(_e70));
    } else {
        let _e72 = dst;
        src = (_e72 * vec2(2i));
        let _e77 = pc.u_offset_miplevel[2u];
        prev_level = (_e77 - 1i);
        let _e79 = prev_level;
        let _e80 = textureDimensions(u_depth_image, _e79);
        prev_size = vec2<i32>(_e80);
        let _e82 = src;
        let _e84 = prev_level;
        let _e85 = textureLoad(u_depth_image, (_e82 + vec2<i32>(0i, 0i)), _e84);
        d0_ = _e85.x;
        let _e87 = src;
        let _e89 = prev_level;
        let _e90 = textureLoad(u_depth_image, (_e87 + vec2<i32>(1i, 0i)), _e89);
        d1_ = _e90.x;
        let _e92 = src;
        let _e94 = prev_level;
        let _e95 = textureLoad(u_depth_image, (_e92 + vec2<i32>(0i, 1i)), _e94);
        d2_ = _e95.x;
        let _e97 = src;
        let _e99 = prev_level;
        let _e100 = textureLoad(u_depth_image, (_e97 + vec2<i32>(1i, 1i)), _e99);
        d3_ = _e100.x;
        let _e102 = d0_;
        let _e103 = d1_;
        let _e105 = d2_;
        let _e106 = d3_;
        min_depth = min(min(_e102, _e103), min(_e105, _e106));
        let _e110 = current_size[0u];
        let _e113 = prev_size[0u];
        extra_sample_x = ((_e110 * 2i) < _e113);
        let _e116 = current_size[1u];
        let _e119 = prev_size[1u];
        extra_sample_y = ((_e116 * 2i) < _e119);
        let _e121 = extra_sample_x;
        if _e121 {
            let _e122 = src;
            let _e124 = prev_level;
            let _e125 = textureLoad(u_depth_image, (_e122 + vec2<i32>(2i, 0i)), _e124);
            d4_ = _e125.x;
            let _e127 = src;
            let _e129 = prev_level;
            let _e130 = textureLoad(u_depth_image, (_e127 + vec2<i32>(2i, 1i)), _e129);
            d5_ = _e130.x;
            let _e132 = min_depth;
            let _e133 = d4_;
            let _e134 = d5_;
            min_depth = min(_e132, min(_e133, _e134));
        }
        let _e137 = extra_sample_y;
        if _e137 {
            let _e138 = src;
            let _e140 = prev_level;
            let _e141 = textureLoad(u_depth_image, (_e138 + vec2<i32>(0i, 2i)), _e140);
            d6_ = _e141.x;
            let _e143 = src;
            let _e145 = prev_level;
            let _e146 = textureLoad(u_depth_image, (_e143 + vec2<i32>(1i, 2i)), _e145);
            d7_ = _e146.x;
            let _e148 = min_depth;
            let _e149 = d6_;
            let _e150 = d7_;
            min_depth = min(_e148, min(_e149, _e150));
        }
        let _e153 = extra_sample_x;
        let _e154 = extra_sample_y;
        if (_e153 && _e154) {
            let _e156 = src;
            let _e158 = prev_level;
            let _e159 = textureLoad(u_depth_image, (_e156 + vec2<i32>(2i, 2i)), _e158);
            d8_ = _e159.x;
            let _e161 = min_depth;
            let _e162 = d8_;
            min_depth = min(_e161, _e162);
        }
        let _e164 = dst;
        let _e165 = min_depth;
        textureStore(u_hiz_depth, _e164, vec4(_e165));
    }
    return;
}

@compute @workgroup_size(16, 16, 1) 
fn main(@builtin(global_invocation_id) gl_GlobalInvocationID: vec3<u32>) {
    gl_GlobalInvocationID_1 = gl_GlobalInvocationID;
    main_1();
}
