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
    @location(7) member_6: vec3<f32>,
    @location(6) member_7: vec3<f32>,
    @location(5) member_8: vec3<f32>,
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
var<private> v_normal_1: vec4<f32>;
var<private> v_tangent_1: vec4<f32>;
var<private> f_bitangent: vec3<f32>;
var<private> f_tangent: vec3<f32>;
var<private> f_normal: vec3<f32>;

fn main_1() {
    var local: vec3<f32>;
    var local_1: vec3<f32>;
    var local_2: vec4<f32>;
    var local_3: vec4<f32>;
    var local_4: vec3<f32>;
    var local_5: vec4<f32>;
    var local_6: vec3<f32>;
    var local_7: vec4<f32>;
    var v_world_position: vec4<f32>;
    var param: vec3<f32>;
    var param_1: vec4<f32>;
    var param_2: vec3<f32>;
    var param_3: vec3<f32>;
    var param_4: u32;
    var world_normal: vec3<f32>;
    var param_5: vec4<f32>;
    var param_6: vec3<f32>;
    var world_tangent: vec3<f32>;
    var param_7: vec4<f32>;
    var param_8: vec3<f32>;

    let _e57 = gl_InstanceIndex_1;
    let _e58 = gl_InstanceIndex_1;
    let _e59 = gl_InstanceIndex_1;
    let _e63 = u_object_buffer.m_objects[_e57].m_translation;
    param = _e63;
    let _e67 = u_object_buffer.m_objects[_e58].m_rotation;
    param_1 = _e67;
    let _e71 = u_object_buffer.m_objects[_e59].m_scale;
    param_2 = _e71;
    let _e72 = v_position_1;
    param_3 = _e72;
    let _e73 = param_3;
    let _e74 = param_2;
    param_3 = (_e73 * _e74);
    let _e76 = param_1;
    local_5 = _e76;
    let _e77 = param_3;
    local_6 = _e77;
    let _e78 = local_6;
    let _e79 = local_6;
    let _e80 = local_5;
    let _e84 = local_5[3u];
    let _e85 = local_6;
    let _e88 = local_5;
    local_4 = (_e78 + (cross((cross(_e79, _e80.xyz) + (_e85 * _e84)), _e88.xyz) * 2f));
    let _e93 = local_4;
    param_3 = _e93;
    let _e94 = param_3;
    let _e95 = param;
    param_3 = (_e94 + _e95);
    let _e97 = param_3;
    local_7 = vec4<f32>(_e97.x, _e97.y, _e97.z, 1f);
    let _e102 = local_7;
    v_world_position = _e102;
    let _e103 = v_world_position;
    f_world_position = _e103;
    let _e105 = u_camera.m_projection_view_matrix;
    let _e106 = v_world_position;
    unnamed.gl_Position = (_e105 * _e106);
    let _e109 = v_color_1;
    let _e111 = gl_InstanceIndex_1;
    let _e115 = u_object_buffer.m_objects[_e111].m_custom_vertex_color;
    param_4 = _e115;
    let _e116 = param_4;
    local_2[3u] = (f32((_e116 >> bitcast<u32>(24i))) / 255f);
    let _e122 = param_4;
    local_2[0u] = (f32(((_e122 >> bitcast<u32>(16i)) & 255u)) / 255f);
    let _e129 = param_4;
    local_2[1u] = (f32(((_e129 >> bitcast<u32>(8i)) & 255u)) / 255f);
    let _e136 = param_4;
    local_2[2u] = (f32((_e136 & 255u)) / 255f);
    let _e141 = local_2;
    local_3 = _e141;
    let _e142 = local_3;
    f_vertex_color = (_e109.zyxw * _e142);
    let _e144 = v_uv_1;
    let _e145 = gl_InstanceIndex_1;
    let _e149 = u_object_buffer.m_objects[_e145].m_texture_trans;
    f_uv = (_e144 + _e149);
    let _e151 = v_uv_two_1;
    f_uv_two = _e151;
    let _e152 = gl_InstanceIndex_1;
    let _e156 = u_object_buffer.m_objects[_e152].m_material_id;
    f_material_id = _e156;
    let _e157 = gl_InstanceIndex_1;
    let _e161 = u_object_buffer.m_objects[_e157].m_hue_change;
    f_hue_change = _e161;
    let _e162 = gl_InstanceIndex_1;
    let _e166 = u_object_buffer.m_objects[_e162].m_rotation;
    param_5 = _e166;
    let _e167 = v_normal_1;
    param_6 = _e167.xyz;
    let _e169 = param_6;
    let _e170 = param_6;
    let _e171 = param_5;
    let _e175 = param_5[3u];
    let _e176 = param_6;
    let _e179 = param_5;
    local_1 = (_e169 + (cross((cross(_e170, _e171.xyz) + (_e176 * _e175)), _e179.xyz) * 2f));
    let _e184 = local_1;
    world_normal = _e184;
    let _e185 = gl_InstanceIndex_1;
    let _e189 = u_object_buffer.m_objects[_e185].m_rotation;
    param_7 = _e189;
    let _e190 = v_tangent_1;
    param_8 = _e190.xyz;
    let _e192 = param_8;
    let _e193 = param_8;
    let _e194 = param_7;
    let _e198 = param_7[3u];
    let _e199 = param_8;
    let _e202 = param_7;
    local = (_e192 + (cross((cross(_e193, _e194.xyz) + (_e199 * _e198)), _e202.xyz) * 2f));
    let _e207 = local;
    world_tangent = _e207;
    let _e208 = world_normal;
    let _e209 = world_tangent;
    let _e212 = v_tangent_1[3u];
    f_bitangent = (cross(_e208, _e209) * _e212);
    let _e214 = world_tangent;
    f_tangent = _e214;
    let _e215 = world_normal;
    f_normal = _e215;
    return;
}

@vertex 
fn main(@builtin(instance_index) gl_InstanceIndex: u32, @location(0) v_position: vec3<f32>, @location(2) v_color: vec4<f32>, @location(3) v_uv: vec2<f32>, @location(4) v_uv_two: vec2<f32>, @location(1) v_normal: vec4<f32>, @location(5) v_tangent: vec4<f32>) -> VertexOutput {
    gl_InstanceIndex_1 = i32(gl_InstanceIndex);
    v_position_1 = v_position;
    v_color_1 = v_color;
    v_uv_1 = v_uv;
    v_uv_two_1 = v_uv_two;
    v_normal_1 = v_normal;
    v_tangent_1 = v_tangent;
    main_1();
    let _e27 = unnamed.gl_Position.y;
    unnamed.gl_Position.y = -(_e27);
    let _e29 = f_world_position;
    let _e30 = unnamed.gl_Position;
    let _e31 = f_vertex_color;
    let _e32 = f_uv;
    let _e33 = f_uv_two;
    let _e34 = f_material_id;
    let _e35 = f_hue_change;
    let _e36 = f_bitangent;
    let _e37 = f_tangent;
    let _e38 = f_normal;
    return VertexOutput(_e29, _e30, _e31, _e32, _e33, _e34, _e35, _e36, _e37, _e38);
}
