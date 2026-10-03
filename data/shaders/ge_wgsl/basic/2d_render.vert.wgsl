struct gl_PerVertex {
    @builtin(position) gl_Position: vec4<f32>,
    gl_PointSize: f32,
    gl_ClipDistance: array<f32, 1>,
    gl_CullDistance: array<f32, 1>,
}

struct VertexOutput {
    @builtin(position) gl_Position: vec4<f32>,
    @location(0) member: vec4<f32>,
    @location(1) member_1: vec2<f32>,
    @location(2) @interpolate(flat) member_2: i32,
}

var<private> unnamed: gl_PerVertex = gl_PerVertex(vec4<f32>(0f, 0f, 0f, 1f), 1f, array<f32, 1>(), array<f32, 1>());
var<private> v_position_1: vec2<f32>;
var<private> f_color: vec4<f32>;
var<private> v_color_1: vec4<f32>;
var<private> f_uv: vec2<f32>;
var<private> v_uv_1: vec2<f32>;
var<private> f_sampler_index: i32;
var<private> v_sampler_index_1: i32;

fn main_1() {
    let _e12 = v_position_1;
    unnamed.gl_Position = vec4<f32>(_e12.x, _e12.y, 0f, 1f);
    let _e17 = v_color_1;
    f_color = _e17.zyxw;
    let _e19 = v_uv_1;
    f_uv = _e19;
    let _e20 = v_sampler_index_1;
    f_sampler_index = _e20;
    return;
}

@vertex 
fn main(@location(0) v_position: vec2<f32>, @location(1) v_color: vec4<f32>, @location(2) v_uv: vec2<f32>, @location(3) v_sampler_index: i32) -> VertexOutput {
    v_position_1 = v_position;
    v_color_1 = v_color;
    v_uv_1 = v_uv;
    v_sampler_index_1 = v_sampler_index;
    main_1();
    let _e14 = unnamed.gl_Position.y;
    unnamed.gl_Position.y = -(_e14);
    let _e16 = unnamed.gl_Position;
    let _e17 = f_color;
    let _e18 = f_uv;
    let _e19 = f_sampler_index;
    return VertexOutput(_e16, _e17, _e18, _e19);
}
