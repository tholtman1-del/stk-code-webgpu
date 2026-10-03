@group(0) @binding(16) 
var f_tex_sampler: sampler;
@group(0) @binding(0) 
var f_tex_image: texture_2d<f32>;
var<private> f_uv_1: vec2<f32>;
var<private> o_color: vec4<f32>;
var<private> f_color_1: vec4<f32>;

fn main_1() {
    var tex_color: vec4<f32>;

    let _e6 = f_uv_1;
    let _e7 = textureSample(f_tex_image, f_tex_sampler, _e6);
    tex_color = _e7;
    let _e8 = tex_color;
    let _e9 = f_color_1;
    o_color = (_e8 * _e9);
    return;
}

@fragment 
fn main(@location(1) f_uv: vec2<f32>, @location(0) f_color: vec4<f32>) -> @location(0) vec4<f32> {
    f_uv_1 = f_uv;
    f_color_1 = f_color;
    main_1();
    let _e5 = o_color;
    return _e5;
}
