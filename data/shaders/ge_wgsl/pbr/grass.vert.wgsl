struct Constants {
    m_wind_direction: vec3<f32>,
}

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

var<immediate> u_push_constants: Constants;
var<private> v_position_1: vec3<f32>;
@group(1) @binding(1) 
var<storage> u_object_buffer: ObjectBuffer;
var<private> gl_InstanceIndex_1: i32;
var<private> v_color_1: vec4<f32>;
var<private> f_world_position: vec4<f32>;
var<private> unnamed: gl_PerVertex = gl_PerVertex(vec4<f32>(0f, 0f, 0f, 1f), 1f, array<f32, 1>(), array<f32, 1>());
@group(1) @binding(0) 
var<uniform> u_camera: CameraBuffer;
var<private> f_vertex_color: vec4<f32>;
var<private> f_uv: vec2<f32>;
var<private> v_uv_1: vec2<f32>;
var<private> f_uv_two: vec2<f32>;
var<private> v_uv_two_1: vec2<f32>;
var<private> f_material_id: i32;
var<private> f_hue_change: f32;
var<private> f_normal: vec3<f32>;
var<private> v_normal_1: vec4<f32>;
var<private> f_tangent: vec3<f32>;
var<private> f_bitangent: vec3<f32>;

fn main_1() {
    var local: vec3<f32>;
    var local_1: vec3<f32>;
    var local_2: vec4<f32>;
    var local_3: vec3<f32>;
    var local_4: vec4<f32>;
    var offset: vec3<f32>;
    var v_world_position: vec4<f32>;
    var param: vec3<f32>;
    var param_1: vec4<f32>;
    var param_2: vec3<f32>;
    var param_3: vec3<f32>;
    var param_4: vec4<f32>;
    var param_5: vec3<f32>;

    let _e47 = u_push_constants.m_wind_direction;
    let _e49 = v_position_1[1u];
    offset = sin((_e47 * (_e49 * 0.1f)));
    let _e54 = u_push_constants.m_wind_direction;
    let _e57 = offset;
    offset = (_e57 + (cos(_e54) * 0.7f));
    let _e59 = gl_InstanceIndex_1;
    let _e63 = u_object_buffer.m_objects[_e59].m_translation;
    let _e64 = offset;
    let _e66 = v_color_1[0u];
    let _e69 = gl_InstanceIndex_1;
    let _e70 = gl_InstanceIndex_1;
    param = (_e63 + (_e64 * _e66));
    let _e74 = u_object_buffer.m_objects[_e69].m_rotation;
    param_1 = _e74;
    let _e78 = u_object_buffer.m_objects[_e70].m_scale;
    param_2 = _e78;
    let _e79 = v_position_1;
    param_3 = _e79;
    let _e80 = param_3;
    let _e81 = param_2;
    param_3 = (_e80 * _e81);
    let _e83 = param_1;
    local_2 = _e83;
    let _e84 = param_3;
    local_3 = _e84;
    let _e85 = local_3;
    let _e86 = local_3;
    let _e87 = local_2;
    let _e91 = local_2[3u];
    let _e92 = local_3;
    let _e95 = local_2;
    local_1 = (_e85 + (cross((cross(_e86, _e87.xyz) + (_e92 * _e91)), _e95.xyz) * 2f));
    let _e100 = local_1;
    param_3 = _e100;
    let _e101 = param_3;
    let _e102 = param;
    param_3 = (_e101 + _e102);
    let _e104 = param_3;
    local_4 = vec4<f32>(_e104.x, _e104.y, _e104.z, 1f);
    let _e109 = local_4;
    v_world_position = _e109;
    let _e110 = v_world_position;
    f_world_position = _e110;
    let _e112 = u_camera.m_projection_view_matrix;
    let _e113 = v_world_position;
    unnamed.gl_Position = (_e112 * _e113);
    f_vertex_color = vec4<f32>(1f, 1f, 1f, 1f);
    let _e116 = v_uv_1;
    f_uv = _e116;
    let _e117 = v_uv_two_1;
    f_uv_two = _e117;
    let _e118 = gl_InstanceIndex_1;
    let _e122 = u_object_buffer.m_objects[_e118].m_material_id;
    f_material_id = _e122;
    let _e123 = gl_InstanceIndex_1;
    let _e127 = u_object_buffer.m_objects[_e123].m_hue_change;
    f_hue_change = _e127;
    let _e128 = gl_InstanceIndex_1;
    let _e132 = u_object_buffer.m_objects[_e128].m_rotation;
    param_4 = _e132;
    let _e133 = v_normal_1;
    param_5 = _e133.xyz;
    let _e135 = param_5;
    let _e136 = param_5;
    let _e137 = param_4;
    let _e141 = param_4[3u];
    let _e142 = param_5;
    let _e145 = param_4;
    local = (_e135 + (cross((cross(_e136, _e137.xyz) + (_e142 * _e141)), _e145.xyz) * 2f));
    let _e150 = local;
    f_normal = _e150;
    return;
}

@vertex 
fn main(@location(0) v_position: vec3<f32>, @builtin(instance_index) gl_InstanceIndex: u32, @location(2) v_color: vec4<f32>, @location(3) v_uv: vec2<f32>, @location(4) v_uv_two: vec2<f32>, @location(1) v_normal_packed: u32) -> VertexOutput {
    let v_normal = ge_unpack_snorm_10_10_10_2(v_normal_packed);
    v_position_1 = v_position;
    gl_InstanceIndex_1 = i32(gl_InstanceIndex);
    v_color_1 = v_color;
    v_uv_1 = v_uv;
    v_uv_two_1 = v_uv_two;
    v_normal_1 = v_normal;
    main_1();
    let _e25 = unnamed.gl_Position.y;
    unnamed.gl_Position.y = -(_e25);
    let _e27 = f_world_position;
    let _e28 = unnamed.gl_Position;
    let _e29 = f_vertex_color;
    let _e30 = f_uv;
    let _e31 = f_uv_two;
    let _e32 = f_material_id;
    let _e33 = f_hue_change;
    let _e34 = f_normal;
    let _e35 = f_tangent;
    let _e36 = f_bitangent;
    return VertexOutput(_e27, _e28, _e29, _e30, _e31, _e32, _e33, _e34, _e35, _e36);
}

fn ge_unpack_snorm_10_10_10_2(p: u32) -> vec4<f32> {
    let v = vec4<f32>(f32(bitcast<i32>(p << 22u) >> 22u) / 511.0,
        f32(bitcast<i32>(p << 12u) >> 22u) / 511.0,
        f32(bitcast<i32>(p << 2u) >> 22u) / 511.0,
        f32(bitcast<i32>(p) >> 30u));
    return max(v, vec4<f32>(-1.0));
}
