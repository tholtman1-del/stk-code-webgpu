diagnostic(off, derivative_uniformity);
@group(0) @binding(16) 
var f_mesh_texture_0_sampler: sampler;
@group(0) @binding(0) 
var f_mesh_texture_0_image: texture_2d<f32>;
var<private> f_uv_1: vec2<f32>;
var<private> f_vertex_color_1: vec4<f32>;
var<private> f_hue_change_1: f32;
var<private> o_color: vec4<f32>;

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
    var old_hsv: vec3<f32>;
    var param_1: vec3<f32>;
    var new_xy: vec2<f32>;
    var new_color: vec3<f32>;
    var param_2: vec3<f32>;
    var mixed_color: vec3<f32>;

    let _e40 = f_uv_1;
    param = _e40;
    let _e41 = param;
    let _e42 = textureSample(f_mesh_texture_0_image, f_mesh_texture_0_sampler, _e41);
    local_9 = _e42;
    let _e43 = local_9;
    tex_color = _e43;
    let _e45 = tex_color[3u];
    let _e47 = f_vertex_color_1[3u];
    if ((_e45 * _e47) < 0.5f) {
        discard;
    }
    let _e50 = f_hue_change_1;
    if (_e50 > 0f) {
        let _e52 = tex_color;
        param_1 = _e52.xyz;
        local_3 = vec4<f32>(0f, -0.33333334f, 0.6666667f, -1f);
        let _e54 = param_1;
        let _e55 = _e54.zy;
        let _e56 = local_3;
        let _e57 = _e56.wz;
        let _e63 = param_1;
        let _e64 = _e63.yz;
        let _e65 = local_3;
        let _e66 = _e65.xy;
        let _e73 = param_1[2u];
        let _e75 = param_1[1u];
        local_4 = mix(vec4<f32>(_e55.x, _e55.y, _e57.x, _e57.y), vec4<f32>(_e64.x, _e64.y, _e66.x, _e66.y), vec4(step(_e73, _e75)));
        let _e79 = local_4;
        let _e80 = _e79.xyw;
        let _e82 = param_1[0u];
        let _e88 = param_1[0u];
        let _e89 = local_4;
        let _e90 = _e89.yzx;
        let _e96 = local_4[0u];
        let _e98 = param_1[0u];
        local_5 = mix(vec4<f32>(_e80.x, _e80.y, _e80.z, _e82), vec4<f32>(_e88, _e90.x, _e90.y, _e90.z), vec4(step(_e96, _e98)));
        let _e103 = local_5[0u];
        let _e105 = local_5[3u];
        let _e107 = local_5[1u];
        local_6 = (_e103 - min(_e105, _e107));
        local_7 = 0.0000000001f;
        let _e111 = local_5[2u];
        let _e113 = local_5[3u];
        let _e115 = local_5[1u];
        let _e117 = local_6;
        let _e119 = local_7;
        let _e124 = local_6;
        let _e126 = local_5[0u];
        let _e127 = local_7;
        let _e131 = local_5[0u];
        local_8 = vec3<f32>(abs((_e111 + ((_e113 - _e115) / ((6f * _e117) + _e119)))), (_e124 / (_e126 + _e127)), _e131);
        let _e133 = local_8;
        old_hsv = _e133;
        let _e134 = f_hue_change_1;
        let _e136 = old_hsv[1u];
        new_xy = vec2<f32>(_e134, _e136);
        let _e139 = new_xy[0u];
        let _e141 = new_xy[1u];
        let _e143 = old_hsv[2u];
        param_2 = vec3<f32>(_e139, _e141, _e143);
        local = vec4<f32>(1f, 0.6666667f, 0.33333334f, 3f);
        let _e145 = param_2;
        let _e147 = local;
        let _e152 = local;
        local_1 = abs(((fract((_e145.xxx + _e147.xyz)) * 6f) - _e152.www));
        let _e157 = param_2[2u];
        let _e158 = local;
        let _e160 = local_1;
        let _e161 = local;
        let _e168 = param_2[1u];
        local_2 = (mix(_e158.xxx, clamp((_e160 - _e161.xxx), vec3(0f), vec3(1f)), vec3(_e168)) * _e157);
        let _e172 = local_2;
        new_color = _e172;
        let _e174 = new_color[0u];
        let _e176 = new_color[1u];
        let _e178 = new_color[2u];
        let _e180 = tex_color[3u];
        tex_color = vec4<f32>(_e174, _e176, _e178, _e180);
    }
    let _e182 = tex_color;
    let _e184 = f_vertex_color_1;
    mixed_color = (_e182.xyz * _e184.xyz);
    let _e187 = mixed_color;
    o_color = vec4<f32>(_e187.x, _e187.y, _e187.z, 1f);
    return;
}

@fragment 
fn main(@location(1) f_uv: vec2<f32>, @location(0) f_vertex_color: vec4<f32>, @location(4) f_hue_change: f32) -> @location(0) vec4<f32> {
    f_uv_1 = f_uv;
    f_vertex_color_1 = f_vertex_color;
    f_hue_change_1 = f_hue_change;
    main_1();
    let _e7 = o_color;
    return _e7;
}
