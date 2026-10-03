@id(0) override u_ibl: bool = true;

@group(0) @binding(16) 
var f_mesh_texture_0_sampler: sampler;
@group(0) @binding(0) 
var f_mesh_texture_0_image: texture_2d<f32>;
var<private> f_uv_1: vec2<f32>;
var<private> f_hue_change_1: f32;
var<private> f_vertex_color_1: vec4<f32>;
var<private> o_color: vec4<f32>;

fn convertColor_u0028_vf3_u003b(input_color: ptr<function, vec3<f32>>) -> vec3<f32> {
    if u_ibl {
        let _e32 = (*input_color);
        let _e33 = (*input_color);
        let _e38 = (*input_color);
        let _e39 = (*input_color);
        return ((_e32 * ((_e33 * 6.5f) + vec3(0.45f))) / ((_e38 * ((_e39 * 5f) + vec3(1.75f))) + vec3(0.05f)));
    } else {
        let _e47 = (*input_color);
        let _e48 = (*input_color);
        let _e53 = (*input_color);
        let _e54 = (*input_color);
        return ((_e47 * ((_e48 * 7f) + vec3(0.75f))) / ((_e53 * ((_e54 * 5f) + vec3(1.75f))) + vec3(0.05f)));
    }
}

fn main_1() {
    var local: vec4<f32>;
    var local_1: vec3<f32>;
    var local_2: vec3<f32>;
    var local_3: vec4<f32>;
    var local_4: vec4<f32>;
    var local_5: vec4<f32>;
    var local_6: f32;
    var local_7: f32;
    var local_8: vec3<f32>;
    var local_9: vec4<f32>;
    var tex_color: vec4<f32>;
    var param: vec2<f32>;
    var mask: f32;
    var old_hsv: vec3<f32>;
    var param_1: vec3<f32>;
    var mask_step: f32;
    var saturation: f32;
    var new_xy: vec2<f32>;
    var new_color: vec3<f32>;
    var param_2: vec3<f32>;
    var mixed_color: vec3<f32>;
    var param_3: vec3<f32>;

    let _e53 = f_uv_1;
    param = _e53;
    let _e54 = param;
    let _e55 = textureSample(f_mesh_texture_0_image, f_mesh_texture_0_sampler, _e54);
    local_9 = _e55;
    let _e56 = local_9;
    tex_color = _e56;
    let _e57 = f_hue_change_1;
    if (_e57 > 0f) {
        let _e60 = tex_color[3u];
        mask = _e60;
        let _e61 = tex_color;
        param_1 = _e61.xyz;
        local_3 = vec4<f32>(0f, -0.33333334f, 0.6666667f, -1f);
        let _e63 = param_1;
        let _e64 = _e63.zy;
        let _e65 = local_3;
        let _e66 = _e65.wz;
        let _e72 = param_1;
        let _e73 = _e72.yz;
        let _e74 = local_3;
        let _e75 = _e74.xy;
        let _e82 = param_1[2u];
        let _e84 = param_1[1u];
        local_4 = mix(vec4<f32>(_e64.x, _e64.y, _e66.x, _e66.y), vec4<f32>(_e73.x, _e73.y, _e75.x, _e75.y), vec4(step(_e82, _e84)));
        let _e88 = local_4;
        let _e89 = _e88.xyw;
        let _e91 = param_1[0u];
        let _e97 = param_1[0u];
        let _e98 = local_4;
        let _e99 = _e98.yzx;
        let _e105 = local_4[0u];
        let _e107 = param_1[0u];
        local_5 = mix(vec4<f32>(_e89.x, _e89.y, _e89.z, _e91), vec4<f32>(_e97, _e99.x, _e99.y, _e99.z), vec4(step(_e105, _e107)));
        let _e112 = local_5[0u];
        let _e114 = local_5[3u];
        let _e116 = local_5[1u];
        local_6 = (_e112 - min(_e114, _e116));
        local_7 = 0.0000000001f;
        let _e120 = local_5[2u];
        let _e122 = local_5[3u];
        let _e124 = local_5[1u];
        let _e126 = local_6;
        let _e128 = local_7;
        let _e133 = local_6;
        let _e135 = local_5[0u];
        let _e136 = local_7;
        let _e140 = local_5[0u];
        local_8 = vec3<f32>(abs((_e120 + ((_e122 - _e124) / ((6f * _e126) + _e128)))), (_e133 / (_e135 + _e136)), _e140);
        let _e142 = local_8;
        old_hsv = _e142;
        let _e143 = mask;
        mask_step = step(_e143, 0.5f);
        let _e145 = mask;
        saturation = (_e145 * 2.5f);
        let _e148 = old_hsv[0u];
        let _e150 = old_hsv[1u];
        let _e152 = f_hue_change_1;
        let _e154 = old_hsv[1u];
        let _e155 = saturation;
        let _e158 = mask_step;
        new_xy = mix(vec2<f32>(_e148, _e150), vec2<f32>(_e152, max(_e154, _e155)), vec2(_e158));
        let _e162 = new_xy[0u];
        let _e164 = new_xy[1u];
        let _e166 = old_hsv[2u];
        param_2 = vec3<f32>(_e162, _e164, _e166);
        local = vec4<f32>(1f, 0.6666667f, 0.33333334f, 3f);
        let _e168 = param_2;
        let _e170 = local;
        let _e175 = local;
        local_1 = abs(((fract((_e168.xxx + _e170.xyz)) * 6f) - _e175.www));
        let _e180 = param_2[2u];
        let _e181 = local;
        let _e183 = local_1;
        let _e184 = local;
        let _e191 = param_2[1u];
        local_2 = (mix(_e181.xxx, clamp((_e183 - _e184.xxx), vec3(0f), vec3(1f)), vec3(_e191)) * _e180);
        let _e195 = local_2;
        new_color = _e195;
        let _e197 = new_color[0u];
        let _e199 = new_color[1u];
        let _e201 = new_color[2u];
        tex_color = vec4<f32>(_e197, _e199, _e201, 1f);
    }
    let _e203 = tex_color;
    let _e205 = f_vertex_color_1;
    mixed_color = (_e203.xyz * _e205.xyz);
    let _e208 = mixed_color;
    param_3 = _e208;
    let _e209 = convertColor_u0028_vf3_u003b((&param_3));
    mixed_color = _e209;
    let _e210 = mixed_color;
    let _e211 = (_e210 * 0.5f);
    o_color = vec4<f32>(_e211.x, _e211.y, _e211.z, 0.5f);
    return;
}

@fragment 
fn main(@location(1) f_uv: vec2<f32>, @location(4) f_hue_change: f32, @location(0) f_vertex_color: vec4<f32>) -> @location(0) vec4<f32> {
    f_uv_1 = f_uv;
    f_hue_change_1 = f_hue_change;
    f_vertex_color_1 = f_vertex_color;
    main_1();
    let _e7 = o_color;
    return _e7;
}
