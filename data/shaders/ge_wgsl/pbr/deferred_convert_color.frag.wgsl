@id(0) override u_ibl: bool = true;

var<private> o_color: vec4<f32>;
@group(0) @binding(16) 
var u_hdr_sampler: sampler;
@group(0) @binding(0) 
var u_hdr_image: texture_2d<f32>;
var<private> gl_FragCoord_1: vec4<f32>;

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
    var param: vec3<f32>;

    let _e15 = gl_FragCoord_1;
    let _e18 = textureLoad(u_hdr_image, vec2<i32>(_e15.xy), 0i);
    param = _e18.xyz;
    let _e20 = convertColor_u0028_vf3_u003b((&param));
    o_color = vec4<f32>(_e20.x, _e20.y, _e20.z, 1f);
    return;
}

@fragment 
fn main(@builtin(position) gl_FragCoord: vec4<f32>) -> @location(0) vec4<f32> {
    gl_FragCoord_1 = gl_FragCoord;
    main_1();
    let _e3 = o_color;
    return _e3;
}
