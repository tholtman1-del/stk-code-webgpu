struct ObjectData {
    m_translation: vec3<f32>,
    m_hue_change: f32,
    m_rotation: vec4<f32>,
    m_scale: vec3<f32>,
    m_custom_vertex_color: u32,
    m_skinning_offset: i32,
    m_material_id: i32,
    m_texture_trans: vec2<f32>,
}

struct ObjectBuffer {
    m_objects: array<ObjectData>,
}

struct gl_PerVertex {
    @builtin(position) gl_Position: vec4<f32>,
    gl_PointSize: f32,
    gl_ClipDistance: array<f32, 1>,
    gl_CullDistance: array<f32, 1>,
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

struct VertexOutput {
    @location(8) member: vec4<f32>,
    @builtin(position) gl_Position: vec4<f32>,
    @location(0) member_1: vec4<f32>,
    @location(1) member_2: vec2<f32>,
    @location(2) member_3: vec2<f32>,
    @location(3) @interpolate(flat) member_4: i32,
    @location(4) member_5: f32,
    @location(5) member_6: vec3<f32>,
    @location(6) member_7: vec3<f32>,
    @location(7) member_8: vec3<f32>,
}

@group(1) @binding(1) 
var<storage> u_object_buffer: ObjectBuffer;
var<private> gl_InstanceIndex_1: i32;
var<private> v_position_1: vec3<f32>;
var<private> f_world_position: vec4<f32>;
var<private> unnamed: gl_PerVertex = gl_PerVertex(vec4<f32>(0f, 0f, 0f, 1f), 1f, array<f32, 1>(), array<f32, 1>());
@group(1) @binding(0) 
var<uniform> u_camera: CameraBuffer;
var<private> f_vertex_color: vec4<f32>;
var<private> v_color_1: vec4<f32>;
var<private> f_uv: vec2<f32>;
var<private> v_uv_1: vec2<f32>;
var<private> f_uv_two: vec2<f32>;
var<private> v_uv_two_1: vec2<f32>;
var<private> f_material_id: i32;
var<private> f_hue_change: f32;
var<private> f_normal: vec3<f32>;
var<private> f_tangent: vec3<f32>;
var<private> f_bitangent: vec3<f32>;

fn main_1() {
    var local: vec4<f32>;
    var local_1: vec4<f32>;
    var local_2: vec3<f32>;
    var local_3: vec4<f32>;
    var local_4: vec3<f32>;
    var local_5: vec4<f32>;
    var v_world_position: vec4<f32>;
    var param: vec3<f32>;
    var param_1: vec4<f32>;
    var param_2: vec3<f32>;
    var param_3: vec3<f32>;
    var param_4: u32;

    let _e47 = gl_InstanceIndex_1;
    let _e48 = gl_InstanceIndex_1;
    let _e49 = gl_InstanceIndex_1;
    let _e53 = u_object_buffer.m_objects[_e47].m_translation;
    param = _e53;
    let _e57 = u_object_buffer.m_objects[_e48].m_rotation;
    param_1 = _e57;
    let _e61 = u_object_buffer.m_objects[_e49].m_scale;
    param_2 = _e61;
    let _e62 = v_position_1;
    param_3 = _e62;
    let _e63 = param_3;
    let _e64 = param_2;
    param_3 = (_e63 * _e64);
    let _e66 = param_1;
    local_3 = _e66;
    let _e67 = param_3;
    local_4 = _e67;
    let _e68 = local_4;
    let _e69 = local_4;
    let _e70 = local_3;
    let _e74 = local_3[3u];
    let _e75 = local_4;
    let _e78 = local_3;
    local_2 = (_e68 + (cross((cross(_e69, _e70.xyz) + (_e75 * _e74)), _e78.xyz) * 2f));
    let _e83 = local_2;
    param_3 = _e83;
    let _e84 = param_3;
    let _e85 = param;
    param_3 = (_e84 + _e85);
    let _e87 = param_3;
    local_5 = vec4<f32>(_e87.x, _e87.y, _e87.z, 1f);
    let _e92 = local_5;
    v_world_position = _e92;
    let _e93 = v_world_position;
    f_world_position = _e93;
    let _e95 = u_camera.m_projection_view_matrix;
    let _e96 = v_world_position;
    unnamed.gl_Position = (_e95 * _e96);
    let _e99 = v_color_1;
    let _e101 = gl_InstanceIndex_1;
    let _e105 = u_object_buffer.m_objects[_e101].m_custom_vertex_color;
    param_4 = _e105;
    let _e106 = param_4;
    local[3u] = (f32((_e106 >> bitcast<u32>(24i))) / 255f);
    let _e112 = param_4;
    local[0u] = (f32(((_e112 >> bitcast<u32>(16i)) & 255u)) / 255f);
    let _e119 = param_4;
    local[1u] = (f32(((_e119 >> bitcast<u32>(8i)) & 255u)) / 255f);
    let _e126 = param_4;
    local[2u] = (f32((_e126 & 255u)) / 255f);
    let _e131 = local;
    local_1 = _e131;
    let _e132 = local_1;
    f_vertex_color = (_e99.zyxw * _e132);
    let _e134 = v_uv_1;
    let _e135 = gl_InstanceIndex_1;
    let _e139 = u_object_buffer.m_objects[_e135].m_texture_trans;
    f_uv = (_e134 + _e139);
    let _e141 = v_uv_two_1;
    f_uv_two = _e141;
    let _e142 = gl_InstanceIndex_1;
    let _e146 = u_object_buffer.m_objects[_e142].m_material_id;
    f_material_id = _e146;
    let _e147 = gl_InstanceIndex_1;
    let _e151 = u_object_buffer.m_objects[_e147].m_hue_change;
    f_hue_change = _e151;
    return;
}

@vertex 
fn main(@builtin(instance_index) gl_InstanceIndex: u32, @location(0) v_position: vec3<f32>, @location(2) v_color: vec4<f32>, @location(3) v_uv: vec2<f32>, @location(4) v_uv_two: vec2<f32>) -> VertexOutput {
    gl_InstanceIndex_1 = i32(gl_InstanceIndex);
    v_position_1 = v_position;
    v_color_1 = v_color;
    v_uv_1 = v_uv;
    v_uv_two_1 = v_uv_two;
    main_1();
    let _e23 = unnamed.gl_Position.y;
    unnamed.gl_Position.y = -(_e23);
    let _e25 = f_world_position;
    let _e26 = unnamed.gl_Position;
    let _e27 = f_vertex_color;
    let _e28 = f_uv;
    let _e29 = f_uv_two;
    let _e30 = f_material_id;
    let _e31 = f_hue_change;
    let _e32 = f_normal;
    let _e33 = f_tangent;
    let _e34 = f_bitangent;
    return VertexOutput(_e25, _e26, _e27, _e28, _e29, _e30, _e31, _e32, _e33, _e34);
}
