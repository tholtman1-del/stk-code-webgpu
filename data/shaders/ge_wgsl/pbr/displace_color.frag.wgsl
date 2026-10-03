diagnostic(off, derivative_uniformity);
struct Constants {
    m_has_displace: u32,
}

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

var<private> gl_FragCoord_1: vec4<f32>;
@group(1) @binding(4)
var<uniform> u_push_constants: Constants;
@group(0) @binding(16) 
var u_displace_mask_sampler: sampler;
@group(0) @binding(0) 
var u_displace_mask_image: texture_2d<f32>;
@group(1) @binding(0) 
var<uniform> u_camera: CameraBuffer;
var<private> o_color: vec4<f32>;
@group(0) @binding(18) 
var u_displace_color_sampler: sampler;
@group(0) @binding(2) 
var u_displace_color_image: texture_2d<f32>;

fn main_1() {
    var local: vec2<i32>;
    var local_1: vec2<i32>;
    var local_2: vec2<i32>;
    var local_3: vec2<i32>;
    var local_4: vec2<f32>;
    var local_5: vec2<i32>;
    var uv: vec2<i32>;
    var mask: vec2<f32>;
    var shift: vec2<f32>;
    var param: vec2<f32>;
    var param_1: vec4<f32>;
    var phi_113_: bool;
    var phi_191_: bool;

    let _e27 = gl_FragCoord_1;
    uv = vec2<i32>(_e27.xy);
    let _e31 = u_push_constants.m_has_displace;
    if (_e31 != 0u) {
        let _e33 = uv;
        let _e34 = textureLoad(u_displace_mask_image, _e33, 0i);
        mask = _e34.xy;
        let _e37 = mask[0u];
        let _e38 = (_e37 == 0f);
        phi_113_ = _e38;
        if _e38 {
            let _e40 = mask[1u];
            phi_113_ = (_e40 == 0f);
        }
        let _e43 = phi_113_;
        if !(_e43) {
            let _e45 = mask;
            shift = ((_e45 * 2f) - vec2(1f));
            let _e49 = shift;
            param = _e49;
            let _e51 = u_camera.m_viewport;
            param_1 = _e51;
            let _e52 = gl_FragCoord_1;
            local = vec2<i32>(_e52.xy);
            let _e55 = param_1;
            let _e58 = param;
            param = (_e58 * (_e55.zw * 0.02f));
            let _e60 = param_1;
            local_1 = vec2<i32>(_e60.xy);
            let _e63 = param_1;
            let _e65 = param_1;
            local_2 = vec2<i32>((_e63.xy + _e65.zw));
            let _e69 = gl_FragCoord_1;
            let _e72 = param;
            let _e75 = local_1;
            let _e76 = local_2;
            local_3 = clamp((vec2<i32>(_e69.xy) + vec2<i32>(_e72)), _e75, _e76);
            let _e78 = local_3;
            let _e79 = textureLoad(u_displace_mask_image, _e78, 0i);
            local_4 = _e79.xy;
            let _e82 = local_4[0u];
            let _e83 = (_e82 == 0f);
            phi_191_ = _e83;
            if _e83 {
                let _e85 = local_4[1u];
                phi_191_ = (_e85 == 0f);
            }
            let _e88 = phi_191_;
            if !(_e88) {
                let _e90 = local_3;
                local = _e90;
            }
            let _e91 = local;
            local_5 = _e91;
            let _e92 = local_5;
            uv = _e92;
        }
    }
    let _e93 = uv;
    let _e94 = textureLoad(u_displace_color_image, _e93, 0i);
    o_color = _e94;
    return;
}

@fragment 
fn main(@builtin(position) gl_FragCoord: vec4<f32>) -> @location(0) vec4<f32> {
    gl_FragCoord_1 = gl_FragCoord;
    main_1();
    let _e3 = o_color;
    return _e3;
}
