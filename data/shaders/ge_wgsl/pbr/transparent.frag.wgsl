@id(0) override u_ibl: bool = true;

@group(0) @binding(16) 
var f_mesh_texture_0_sampler: sampler;
@group(0) @binding(0) 
var f_mesh_texture_0_image: texture_2d<f32>;
var<private> f_uv_1: vec2<f32>;
var<private> f_vertex_color_1: vec4<f32>;
var<private> o_color: vec4<f32>;

fn convertColor_u0028_vf3_u003b(input_color: ptr<function, vec3<f32>>) -> vec3<f32> {
    if u_ibl {
        let _e15 = (*input_color);
        let _e16 = (*input_color);
        let _e21 = (*input_color);
        let _e22 = (*input_color);
        return ((_e15 * ((_e16 * 6.5f) + vec3(0.45f))) / ((_e21 * ((_e22 * 5f) + vec3(1.75f))) + vec3(0.05f)));
    } else {
        let _e30 = (*input_color);
        let _e31 = (*input_color);
        let _e36 = (*input_color);
        let _e37 = (*input_color);
        return ((_e30 * ((_e31 * 7f) + vec3(0.75f))) / ((_e36 * ((_e37 * 5f) + vec3(1.75f))) + vec3(0.05f)));
    }
}

fn main_1() {
    var local: vec4<f32>;
    var color: vec4<f32>;
    var param: vec2<f32>;
    var mixed_color: vec3<f32>;
    var alpha: f32;
    var param_1: vec3<f32>;

    let _e20 = f_uv_1;
    param = _e20;
    let _e21 = param;
    let _e22 = textureSample(f_mesh_texture_0_image, f_mesh_texture_0_sampler, _e21);
    local = _e22;
    let _e23 = local;
    let _e24 = f_vertex_color_1;
    color = (_e23 * _e24);
    let _e26 = color;
    mixed_color = _e26.xyz;
    let _e29 = color[3u];
    alpha = _e29;
    let _e30 = mixed_color;
    param_1 = _e30;
    let _e31 = convertColor_u0028_vf3_u003b((&param_1));
    mixed_color = _e31;
    let _e32 = mixed_color;
    let _e33 = alpha;
    let _e34 = (_e32 * _e33);
    let _e35 = alpha;
    o_color = vec4<f32>(_e34.x, _e34.y, _e34.z, _e35);
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
