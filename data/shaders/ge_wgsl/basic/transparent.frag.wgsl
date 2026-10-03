@group(0) @binding(16) 
var f_mesh_texture_0_sampler: sampler;
@group(0) @binding(0) 
var f_mesh_texture_0_image: texture_2d<f32>;
var<private> f_uv_1: vec2<f32>;
var<private> f_vertex_color_1: vec4<f32>;
var<private> o_color: vec4<f32>;

fn main_1() {
    var local: vec4<f32>;
    var color: vec4<f32>;
    var param: vec2<f32>;
    var mixed_color: vec3<f32>;
    var alpha: f32;

    let _e11 = f_uv_1;
    param = _e11;
    let _e12 = param;
    let _e13 = textureSample(f_mesh_texture_0_image, f_mesh_texture_0_sampler, _e12);
    local = _e13;
    let _e14 = local;
    let _e15 = f_vertex_color_1;
    color = (_e14 * _e15);
    let _e17 = color;
    mixed_color = _e17.xyz;
    let _e20 = color[3u];
    alpha = _e20;
    let _e21 = mixed_color;
    let _e22 = alpha;
    let _e23 = (_e21 * _e22);
    let _e24 = alpha;
    o_color = vec4<f32>(_e23.x, _e23.y, _e23.z, _e24);
    return;
}

@fragment 
fn main(@location(1) f_uv: vec2<f32>, @location(0) f_vertex_color: vec4<f32>) -> @location(0) vec4<f32> {
    f_uv_1 = f_uv;
    f_vertex_color_1 = f_vertex_color;
    main_1();
    let _e5 = o_color;
    return _e5;
}
