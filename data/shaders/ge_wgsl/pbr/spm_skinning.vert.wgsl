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
    @location(7) member_6: vec3<f32>,
    @location(6) member_7: vec3<f32>,
    @location(5) member_8: vec3<f32>,
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
    var offset: i32;
    var joint_matrix: mat4x4<f32>;
    var v_skinning_position: vec4<f32>;
    var v_world_position: vec4<f32>;
    var param: vec3<f32>;
    var param_1: vec4<f32>;
    var param_2: vec3<f32>;
    var param_3: vec3<f32>;
    var param_4: u32;
    var skinned_normal: vec4<f32>;
    var skinned_tangent: vec4<f32>;
    var world_normal: vec3<f32>;
    var param_5: vec4<f32>;
    var param_6: vec3<f32>;
    var world_tangent: vec3<f32>;
    var param_7: vec4<f32>;
    var param_8: vec3<f32>;

    let _e67 = gl_InstanceIndex_1;
    let _e71 = u_object_buffer.m_objects[_e67].m_skinning_offset;
    offset = _e71;
    let _e73 = v_weight_1[0u];
    let _e75 = v_joint_1[0u];
    let _e76 = offset;
    let _e81 = u_skinning_matrices.m_mat[max((_e75 + _e76), 0i)];
    let _e82 = (_e81 * _e73);
    let _e84 = v_weight_1[1u];
    let _e86 = v_joint_1[1u];
    let _e87 = offset;
    let _e92 = u_skinning_matrices.m_mat[max((_e86 + _e87), 0i)];
    let _e93 = (_e92 * _e84);
    let _e106 = mat4x4<f32>((_e82[0] + _e93[0]), (_e82[1] + _e93[1]), (_e82[2] + _e93[2]), (_e82[3] + _e93[3]));
    let _e108 = v_weight_1[2u];
    let _e110 = v_joint_1[2u];
    let _e111 = offset;
    let _e116 = u_skinning_matrices.m_mat[max((_e110 + _e111), 0i)];
    let _e117 = (_e116 * _e108);
    let _e130 = mat4x4<f32>((_e106[0] + _e117[0]), (_e106[1] + _e117[1]), (_e106[2] + _e117[2]), (_e106[3] + _e117[3]));
    let _e132 = v_weight_1[3u];
    let _e134 = v_joint_1[3u];
    let _e135 = offset;
    let _e140 = u_skinning_matrices.m_mat[max((_e134 + _e135), 0i)];
    let _e141 = (_e140 * _e132);
    joint_matrix = mat4x4<f32>((_e130[0] + _e141[0]), (_e130[1] + _e141[1]), (_e130[2] + _e141[2]), (_e130[3] + _e141[3]));
    let _e155 = joint_matrix;
    let _e156 = v_position_1;
    v_skinning_position = (_e155 * vec4<f32>(_e156.x, _e156.y, _e156.z, 1f));
    let _e162 = gl_InstanceIndex_1;
    let _e163 = gl_InstanceIndex_1;
    let _e164 = gl_InstanceIndex_1;
    let _e168 = u_object_buffer.m_objects[_e162].m_translation;
    param = _e168;
    let _e172 = u_object_buffer.m_objects[_e163].m_rotation;
    param_1 = _e172;
    let _e176 = u_object_buffer.m_objects[_e164].m_scale;
    param_2 = _e176;
    let _e177 = v_skinning_position;
    param_3 = _e177.xyz;
    let _e179 = param_3;
    let _e180 = param_2;
    param_3 = (_e179 * _e180);
    let _e182 = param_1;
    local_5 = _e182;
    let _e183 = param_3;
    local_6 = _e183;
    let _e184 = local_6;
    let _e185 = local_6;
    let _e186 = local_5;
    let _e190 = local_5[3u];
    let _e191 = local_6;
    let _e194 = local_5;
    local_4 = (_e184 + (cross((cross(_e185, _e186.xyz) + (_e191 * _e190)), _e194.xyz) * 2f));
    let _e199 = local_4;
    param_3 = _e199;
    let _e200 = param_3;
    let _e201 = param;
    param_3 = (_e200 + _e201);
    let _e203 = param_3;
    local_7 = vec4<f32>(_e203.x, _e203.y, _e203.z, 1f);
    let _e208 = local_7;
    v_world_position = _e208;
    let _e209 = v_world_position;
    f_world_position = _e209;
    let _e211 = u_camera.m_projection_view_matrix;
    let _e212 = v_world_position;
    unnamed.gl_Position = (_e211 * _e212);
    let _e215 = v_color_1;
    let _e217 = gl_InstanceIndex_1;
    let _e221 = u_object_buffer.m_objects[_e217].m_custom_vertex_color;
    param_4 = _e221;
    let _e222 = param_4;
    local_2[3u] = (f32((_e222 >> bitcast<u32>(24i))) / 255f);
    let _e228 = param_4;
    local_2[0u] = (f32(((_e228 >> bitcast<u32>(16i)) & 255u)) / 255f);
    let _e235 = param_4;
    local_2[1u] = (f32(((_e235 >> bitcast<u32>(8i)) & 255u)) / 255f);
    let _e242 = param_4;
    local_2[2u] = (f32((_e242 & 255u)) / 255f);
    let _e247 = local_2;
    local_3 = _e247;
    let _e248 = local_3;
    f_vertex_color = (_e215.zyxw * _e248);
    let _e250 = v_uv_1;
    let _e251 = gl_InstanceIndex_1;
    let _e255 = u_object_buffer.m_objects[_e251].m_texture_trans;
    f_uv = (_e250 + _e255);
    let _e257 = v_uv_two_1;
    f_uv_two = _e257;
    let _e258 = gl_InstanceIndex_1;
    let _e262 = u_object_buffer.m_objects[_e258].m_material_id;
    f_material_id = _e262;
    let _e263 = gl_InstanceIndex_1;
    let _e267 = u_object_buffer.m_objects[_e263].m_hue_change;
    f_hue_change = _e267;
    let _e268 = joint_matrix;
    let _e269 = v_normal_1;
    skinned_normal = (_e268 * _e269);
    let _e271 = joint_matrix;
    let _e272 = v_tangent_1;
    let _e273 = _e272.xyz;
    skinned_tangent = (_e271 * vec4<f32>(_e273.x, _e273.y, _e273.z, 0f));
    let _e279 = gl_InstanceIndex_1;
    let _e283 = u_object_buffer.m_objects[_e279].m_rotation;
    param_5 = _e283;
    let _e284 = skinned_normal;
    param_6 = _e284.xyz;
    let _e286 = param_6;
    let _e287 = param_6;
    let _e288 = param_5;
    let _e292 = param_5[3u];
    let _e293 = param_6;
    let _e296 = param_5;
    local_1 = (_e286 + (cross((cross(_e287, _e288.xyz) + (_e293 * _e292)), _e296.xyz) * 2f));
    let _e301 = local_1;
    world_normal = _e301;
    let _e302 = gl_InstanceIndex_1;
    let _e306 = u_object_buffer.m_objects[_e302].m_rotation;
    param_7 = _e306;
    let _e307 = skinned_tangent;
    param_8 = _e307.xyz;
    let _e309 = param_8;
    let _e310 = param_8;
    let _e311 = param_7;
    let _e315 = param_7[3u];
    let _e316 = param_8;
    let _e319 = param_7;
    local = (_e309 + (cross((cross(_e310, _e311.xyz) + (_e316 * _e315)), _e319.xyz) * 2f));
    let _e324 = local;
    world_tangent = _e324;
    let _e325 = world_normal;
    let _e326 = world_tangent;
    let _e329 = v_tangent_1[3u];
    f_bitangent = (cross(_e325, _e326) * _e329);
    let _e331 = world_tangent;
    f_tangent = _e331;
    let _e332 = world_normal;
    f_normal = _e332;
    return;
}

@vertex 
fn main(@builtin(instance_index) gl_InstanceIndex: u32, @location(7) v_weight: vec4<f32>, @location(6) v_joint: vec4<i32>, @location(0) v_position: vec3<f32>, @location(2) v_color: vec4<f32>, @location(3) v_uv: vec2<f32>, @location(4) v_uv_two: vec2<f32>, @location(1) v_normal: vec4<f32>, @location(5) v_tangent: vec4<f32>) -> VertexOutput {
    gl_InstanceIndex_1 = i32(gl_InstanceIndex);
    v_weight_1 = v_weight;
    v_joint_1 = v_joint;
    v_position_1 = v_position;
    v_color_1 = v_color;
    v_uv_1 = v_uv;
    v_uv_two_1 = v_uv_two;
    v_normal_1 = v_normal;
    v_tangent_1 = v_tangent;
    main_1();
    let _e31 = unnamed.gl_Position.y;
    unnamed.gl_Position.y = -(_e31);
    let _e33 = f_world_position;
    let _e34 = unnamed.gl_Position;
    let _e35 = f_vertex_color;
    let _e36 = f_uv;
    let _e37 = f_uv_two;
    let _e38 = f_material_id;
    let _e39 = f_hue_change;
    let _e40 = f_bitangent;
    let _e41 = f_tangent;
    let _e42 = f_normal;
    return VertexOutput(_e33, _e34, _e35, _e36, _e37, _e38, _e39, _e40, _e41, _e42);
}
