struct gl_PerVertex {
    @builtin(position) gl_Position: vec4<f32>,
    gl_PointSize: f32,
    gl_ClipDistance: array<f32, 1>,
    gl_CullDistance: array<f32, 1>,
}

struct VertexOutput {
    @location(0) member: vec2<f32>,
    @builtin(position) gl_Position: vec4<f32>,
}

var<private> f_uv: vec2<f32>;
var<private> gl_VertexIndex_1: i32;
var<private> unnamed: gl_PerVertex = gl_PerVertex(vec4<f32>(0f, 0f, 0f, 1f), 1f, array<f32, 1>(), array<f32, 1>());

fn main_1() {
    let _e9 = gl_VertexIndex_1;
    let _e14 = gl_VertexIndex_1;
    f_uv = vec2<f32>(f32(((_e9 << bitcast<u32>(1i)) & 2i)), f32((_e14 & 2i)));
    let _e18 = f_uv;
    let _e21 = ((_e18 * 2f) - vec2(1f));
    unnamed.gl_Position = vec4<f32>(_e21.x, _e21.y, 1f, 1f);
    return;
}

@vertex 
fn main(@builtin(vertex_index) gl_VertexIndex: u32) -> VertexOutput {
    gl_VertexIndex_1 = i32(gl_VertexIndex);
    main_1();
    let _e7 = unnamed.gl_Position.y;
    unnamed.gl_Position.y = -(_e7);
    let _e9 = f_uv;
    let _e10 = unnamed.gl_Position;
    return VertexOutput(_e9, _e10);
}
