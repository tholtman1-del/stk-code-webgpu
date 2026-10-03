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

@id(2) override u_deferred: bool = false;

var<private> f_uv_1: vec2<f32>;
@group(1) @binding(0) 
var<uniform> u_camera: CameraBuffer;
var<private> o_color: vec4<f32>;
@group(0) @binding(19) 
var f_skybox_texture_srgb_sampler: sampler;
@group(0) @binding(3) 
var f_skybox_texture_srgb_image: texture_cube<f32>;
@group(0) @binding(18) 
var f_skybox_texture_sampler: sampler;
@group(0) @binding(2) 
var f_skybox_texture_image: texture_cube<f32>;

fn main_1() {
    var uv: vec2<f32>;
    var front: vec4<f32>;
    var back: vec4<f32>;
    var dir: vec3<f32>;

    let _e17 = f_uv_1;
    uv = ((_e17 * 2f) - vec2(1f));
    let _e22 = u_camera.m_inverse_projection_view_matrix;
    let _e23 = uv;
    front = (_e22 * vec4<f32>(_e23.x, _e23.y, -1f, 1f));
    let _e29 = u_camera.m_inverse_projection_view_matrix;
    let _e30 = uv;
    back = (_e29 * vec4<f32>(_e30.x, _e30.y, 1f, 1f));
    let _e35 = back;
    let _e38 = back[3u];
    let _e41 = front;
    let _e44 = front[3u];
    dir = ((_e35.xyz / vec3(_e38)) - (_e41.xyz / vec3(_e44)));
    if u_deferred {
        let _e48 = dir;
        let _e49 = textureSample(f_skybox_texture_srgb_image, f_skybox_texture_srgb_sampler, _e48);
        o_color = _e49;
    } else {
        let _e50 = dir;
        let _e51 = textureSample(f_skybox_texture_image, f_skybox_texture_sampler, _e50);
        o_color = _e51;
    }
    return;
}

@fragment 
fn main(@location(0) f_uv: vec2<f32>) -> @location(0) vec4<f32> {
    f_uv_1 = f_uv;
    main_1();
    let _e3 = o_color;
    return _e3;
}
