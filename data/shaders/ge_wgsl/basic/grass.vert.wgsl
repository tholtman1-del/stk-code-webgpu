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

@group(1) @binding(4)
var<uniform> u_push_constants: Constants;
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
var<private> f_tangent: vec3<f32>;
var<private> f_bitangent: vec3<f32>;

fn main_1() {
    var local: vec3<f32>;
    var local_1: vec4<f32>;
    var local_2: vec3<f32>;
    var local_3: vec4<f32>;
    var offset: vec3<f32>;
    var v_world_position: vec4<f32>;
    var param: vec3<f32>;
    var param_1: vec4<f32>;
    var param_2: vec3<f32>;
    var param_3: vec3<f32>;

    let _e43 = u_push_constants.m_wind_direction;
    let _e45 = v_position_1[1u];
    offset = sin((_e43 * (_e45 * 0.1f)));
    let _e50 = u_push_constants.m_wind_direction;
    let _e53 = offset;
    offset = (_e53 + (cos(_e50) * 0.7f));
    let _e55 = gl_InstanceIndex_1;
    let _e59 = u_object_buffer.m_objects[_e55].m_translation;
    let _e60 = offset;
    let _e62 = v_color_1[0u];
    let _e65 = gl_InstanceIndex_1;
    let _e66 = gl_InstanceIndex_1;
    param = (_e59 + (_e60 * _e62));
    let _e70 = u_object_buffer.m_objects[_e65].m_rotation;
    param_1 = _e70;
    let _e74 = u_object_buffer.m_objects[_e66].m_scale;
    param_2 = _e74;
    let _e75 = v_position_1;
    param_3 = _e75;
    let _e76 = param_3;
    let _e77 = param_2;
    param_3 = (_e76 * _e77);
    let _e79 = param_1;
    local_1 = _e79;
    let _e80 = param_3;
    local_2 = _e80;
    let _e81 = local_2;
    let _e82 = local_2;
    let _e83 = local_1;
    let _e87 = local_1[3u];
    let _e88 = local_2;
    let _e91 = local_1;
    local = (_e81 + (cross((cross(_e82, _e83.xyz) + (_e88 * _e87)), _e91.xyz) * 2f));
    let _e96 = local;
    param_3 = _e96;
    let _e97 = param_3;
    let _e98 = param;
    param_3 = (_e97 + _e98);
    let _e100 = param_3;
    local_3 = vec4<f32>(_e100.x, _e100.y, _e100.z, 1f);
    let _e105 = local_3;
    v_world_position = _e105;
    let _e106 = v_world_position;
    f_world_position = _e106;
    let _e108 = u_camera.m_projection_view_matrix;
    let _e109 = v_world_position;
    unnamed.gl_Position = (_e108 * _e109);
    f_vertex_color = vec4<f32>(1f, 1f, 1f, 1f);
    let _e112 = v_uv_1;
    f_uv = _e112;
    let _e113 = v_uv_two_1;
    f_uv_two = _e113;
    let _e114 = gl_InstanceIndex_1;
    let _e118 = u_object_buffer.m_objects[_e114].m_material_id;
    f_material_id = _e118;
    let _e119 = gl_InstanceIndex_1;
    let _e123 = u_object_buffer.m_objects[_e119].m_hue_change;
    f_hue_change = _e123;
    return;
}

@vertex 
fn main(@location(0) v_position: vec3<f32>, @builtin(instance_index) gl_InstanceIndex: u32, @location(2) v_color: vec4<f32>, @location(3) v_uv: vec2<f32>, @location(4) v_uv_two: vec2<f32>) -> VertexOutput {
    v_position_1 = v_position;
    gl_InstanceIndex_1 = i32(gl_InstanceIndex);
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
