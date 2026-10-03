@group(0) @binding(16) 
var f_mesh_texture_0_sampler: sampler;
@group(0) @binding(0) 
var f_mesh_texture_0_image: texture_2d<f32>;
@group(0) @binding(17) 
var f_mesh_texture_1_sampler: sampler;
@group(0) @binding(1) 
var f_mesh_texture_1_image: texture_2d<f32>;
var<private> f_uv_1: vec2<f32>;
var<private> f_uv_two_1: vec2<f32>;
var<private> o_color: vec4<f32>;

fn main_1() {
    var local: vec4<f32>;
    var local_1: vec4<f32>;
    var color: vec4<f32>;
    var param: vec2<f32>;
    var layer_two_tex: vec4<f32>;
    var param_1: vec2<f32>;
    var final_color: vec3<f32>;

    let _e19 = f_uv_1;
    param = _e19;
    let _e20 = param;
    let _e21 = textureSample(f_mesh_texture_0_image, f_mesh_texture_0_sampler, _e20);
    local_1 = _e21;
    let _e22 = local_1;
    color = _e22;
    let _e23 = f_uv_two_1;
    param_1 = _e23;
    let _e24 = param_1;
    let _e25 = textureSample(f_mesh_texture_1_image, f_mesh_texture_1_sampler, _e24);
    local = _e25;
    let _e26 = local;
    layer_two_tex = _e26;
    let _e28 = layer_two_tex[3u];
    let _e29 = layer_two_tex;
    let _e31 = (_e29.xyz * _e28);
    layer_two_tex[0u] = _e31.x;
    layer_two_tex[1u] = _e31.y;
    layer_two_tex[2u] = _e31.z;
    let _e38 = layer_two_tex;
    let _e40 = color;
    let _e43 = layer_two_tex[3u];
    final_color = (_e38.xyz + (_e40.xyz * (1f - _e43)));
    let _e47 = final_color;
    o_color = vec4<f32>(_e47.x, _e47.y, _e47.z, 1f);
    return;
}

@fragment 
fn main(@location(1) f_uv: vec2<f32>, @location(2) f_uv_two: vec2<f32>) -> @location(0) vec4<f32> {
    f_uv_1 = f_uv;
    f_uv_two_1 = f_uv_two;
    main_1();
    let _e5 = o_color;
    return _e5;
}
