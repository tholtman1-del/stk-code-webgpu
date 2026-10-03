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

struct SkinningMatrices {
    m_mat: array<mat4x4<f32>>,
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
var<private> v_weight_1: vec4<f32>;
@group(1) @binding(2) 
var<storage> u_skinning_matrices: SkinningMatrices;
var<private> v_joint_1: vec4<i32>;
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
    var offset: i32;
    var joint_matrix: mat4x4<f32>;
    var v_skinning_position: vec4<f32>;
    var v_world_position: vec4<f32>;
    var param: vec3<f32>;
    var param_1: vec4<f32>;
    var param_2: vec3<f32>;
    var param_3: vec3<f32>;
    var param_4: u32;

    let _e54 = gl_InstanceIndex_1;
    let _e58 = u_object_buffer.m_objects[_e54].m_skinning_offset;
    offset = _e58;
    let _e60 = v_weight_1[0u];
    let _e62 = v_joint_1[0u];
    let _e63 = offset;
    let _e68 = u_skinning_matrices.m_mat[max((_e62 + _e63), 0i)];
    let _e69 = (_e68 * _e60);
    let _e71 = v_weight_1[1u];
    let _e73 = v_joint_1[1u];
    let _e74 = offset;
    let _e79 = u_skinning_matrices.m_mat[max((_e73 + _e74), 0i)];
    let _e80 = (_e79 * _e71);
    let _e93 = mat4x4<f32>((_e69[0] + _e80[0]), (_e69[1] + _e80[1]), (_e69[2] + _e80[2]), (_e69[3] + _e80[3]));
    let _e95 = v_weight_1[2u];
    let _e97 = v_joint_1[2u];
    let _e98 = offset;
    let _e103 = u_skinning_matrices.m_mat[max((_e97 + _e98), 0i)];
    let _e104 = (_e103 * _e95);
    let _e117 = mat4x4<f32>((_e93[0] + _e104[0]), (_e93[1] + _e104[1]), (_e93[2] + _e104[2]), (_e93[3] + _e104[3]));
    let _e119 = v_weight_1[3u];
    let _e121 = v_joint_1[3u];
    let _e122 = offset;
    let _e127 = u_skinning_matrices.m_mat[max((_e121 + _e122), 0i)];
    let _e128 = (_e127 * _e119);
    joint_matrix = mat4x4<f32>((_e117[0] + _e128[0]), (_e117[1] + _e128[1]), (_e117[2] + _e128[2]), (_e117[3] + _e128[3]));
    let _e142 = joint_matrix;
    let _e143 = v_position_1;
    v_skinning_position = (_e142 * vec4<f32>(_e143.x, _e143.y, _e143.z, 1f));
    let _e149 = gl_InstanceIndex_1;
    let _e150 = gl_InstanceIndex_1;
    let _e151 = gl_InstanceIndex_1;
    let _e155 = u_object_buffer.m_objects[_e149].m_translation;
    param = _e155;
    let _e159 = u_object_buffer.m_objects[_e150].m_rotation;
    param_1 = _e159;
    let _e163 = u_object_buffer.m_objects[_e151].m_scale;
    param_2 = _e163;
    let _e164 = v_skinning_position;
    param_3 = _e164.xyz;
    let _e166 = param_3;
    let _e167 = param_2;
    param_3 = (_e166 * _e167);
    let _e169 = param_1;
    local_3 = _e169;
    let _e170 = param_3;
    local_4 = _e170;
    let _e171 = local_4;
    let _e172 = local_4;
    let _e173 = local_3;
    let _e177 = local_3[3u];
    let _e178 = local_4;
    let _e181 = local_3;
    local_2 = (_e171 + (cross((cross(_e172, _e173.xyz) + (_e178 * _e177)), _e181.xyz) * 2f));
    let _e186 = local_2;
    param_3 = _e186;
    let _e187 = param_3;
    let _e188 = param;
    param_3 = (_e187 + _e188);
    let _e190 = param_3;
    local_5 = vec4<f32>(_e190.x, _e190.y, _e190.z, 1f);
    let _e195 = local_5;
    v_world_position = _e195;
    let _e196 = v_world_position;
    f_world_position = _e196;
    let _e198 = u_camera.m_projection_view_matrix;
    let _e199 = v_world_position;
    unnamed.gl_Position = (_e198 * _e199);
    let _e202 = v_color_1;
    let _e204 = gl_InstanceIndex_1;
    let _e208 = u_object_buffer.m_objects[_e204].m_custom_vertex_color;
    param_4 = _e208;
    let _e209 = param_4;
    local[3u] = (f32((_e209 >> bitcast<u32>(24i))) / 255f);
    let _e215 = param_4;
    local[0u] = (f32(((_e215 >> bitcast<u32>(16i)) & 255u)) / 255f);
    let _e222 = param_4;
    local[1u] = (f32(((_e222 >> bitcast<u32>(8i)) & 255u)) / 255f);
    let _e229 = param_4;
    local[2u] = (f32((_e229 & 255u)) / 255f);
    let _e234 = local;
    local_1 = _e234;
    let _e235 = local_1;
    f_vertex_color = (_e202.zyxw * _e235);
    let _e237 = v_uv_1;
    let _e238 = gl_InstanceIndex_1;
    let _e242 = u_object_buffer.m_objects[_e238].m_texture_trans;
    f_uv = (_e237 + _e242);
    let _e244 = v_uv_two_1;
    f_uv_two = _e244;
    let _e245 = gl_InstanceIndex_1;
    let _e249 = u_object_buffer.m_objects[_e245].m_material_id;
    f_material_id = _e249;
    let _e250 = gl_InstanceIndex_1;
    let _e254 = u_object_buffer.m_objects[_e250].m_hue_change;
    f_hue_change = _e254;
    return;
}

@vertex 
fn main(@builtin(instance_index) gl_InstanceIndex: u32, @location(7) v_weight: vec4<f32>, @location(6) v_joint: vec4<i32>, @location(0) v_position: vec3<f32>, @location(2) v_color: vec4<f32>, @location(3) v_uv: vec2<f32>, @location(4) v_uv_two: vec2<f32>) -> VertexOutput {
    gl_InstanceIndex_1 = i32(gl_InstanceIndex);
    v_weight_1 = v_weight;
    v_joint_1 = v_joint;
    v_position_1 = v_position;
    v_color_1 = v_color;
    v_uv_1 = v_uv;
    v_uv_two_1 = v_uv_two;
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
    let _e36 = f_normal;
    let _e37 = f_tangent;
    let _e38 = f_bitangent;
    return VertexOutput(_e29, _e30, _e31, _e32, _e33, _e34, _e35, _e36, _e37, _e38);
}
