@group(0) @binding(16) 
var f_mesh_texture_0_sampler: sampler;
@group(0) @binding(0) 
var f_mesh_texture_0_image: texture_2d<f32>;
var<private> f_uv_1: vec2<f32>;
var<private> f_vertex_color_1: vec4<f32>;

fn main_1() {
    var local: vec4<f32>;
    var tex_color: vec4<f32>;
    var param: vec2<f32>;

    let _e9 = f_uv_1;
    param = _e9;
    let _e10 = param;
    let _e11 = textureSample(f_mesh_texture_0_image, f_mesh_texture_0_sampler, _e10);
    local = _e11;
    let _e12 = local;
    tex_color = _e12;
    let _e14 = tex_color[3u];
    let _e16 = f_vertex_color_1[3u];
    if ((_e14 * _e16) < 0.5f) {
        discard;
    }
    return;
}

@fragment 
fn main(@location(1) f_uv: vec2<f32>, @location(0) f_vertex_color: vec4<f32>) {
    f_uv_1 = f_uv;
    f_vertex_color_1 = f_vertex_color;
    main_1();
}
