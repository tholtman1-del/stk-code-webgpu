struct Constants {
    m_billboard_rotation: vec4<f32>,
    m_fullscreen_light: i32,
}

struct LightData {
    m_position_radius: vec4<f32>,
    m_color_inverse_square_range: vec4<f32>,
    m_direction_scale_offset: vec4<f32>,
}

struct GlobalLightBuffer {
    m_ambient_color: vec3<f32>,
    m_sun_scatter: f32,
    m_sun_color: vec3<f32>,
    m_sun_angle_tan_half: f32,
    m_sun_direction: vec3<f32>,
    m_fog_density: f32,
    m_fog_color: vec4<f32>,
    m_skytop_color: vec3<f32>,
    m_light_count: i32,
    m_lights: array<LightData, 32>,
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

struct gl_PerVertex {
    @builtin(position) gl_Position: vec4<f32>,
    gl_PointSize: f32,
    gl_ClipDistance: array<f32, 1>,
    gl_CullDistance: array<f32, 1>,
}

struct VertexOutput {
    @location(0) @interpolate(flat) member: i32,
    @builtin(position) gl_Position: vec4<f32>,
}

var<private> light_idx: i32;
var<private> gl_InstanceIndex_1: i32;
@group(1) @binding(4)
var<uniform> u_push_constants: Constants;
@group(1) @binding(3) 
var<uniform> u_global_light: GlobalLightBuffer;
@group(1) @binding(0) 
var<uniform> u_camera: CameraBuffer;
var<private> gl_VertexIndex_1: i32;
var<private> unnamed: gl_PerVertex = gl_PerVertex(vec4<f32>(0f, 0f, 0f, 1f), 1f, array<f32, 1>(), array<f32, 1>());

fn main_1() {
    var local: vec3<f32>;
    var local_1: vec4<f32>;
    var local_2: vec3<f32>;
    var local_3: vec4<f32>;
    var light: LightData;
    var pos_radius: vec4<f32>;
    var camera_pos: vec3<f32>;
    var light_to_camera: vec3<f32>;
    var world_pos: vec4<f32>;
    var param: vec3<f32>;
    var param_1: vec4<f32>;
    var param_2: vec3<f32>;
    var param_3: vec3<f32>;
    var indexable: array<vec3<f32>, 4>;
    var pv: vec4<f32>;

    let _e42 = gl_InstanceIndex_1;
    let _e44 = u_push_constants.m_fullscreen_light;
    light_idx = (_e42 + _e44);
    let _e46 = light_idx;
    let _e49 = u_global_light.m_lights[_e46];
    light.m_position_radius = _e49.m_position_radius;
    light.m_color_inverse_square_range = _e49.m_color_inverse_square_range;
    light.m_direction_scale_offset = _e49.m_direction_scale_offset;
    let _e57 = light.m_position_radius;
    pos_radius = _e57;
    let _e60 = u_camera.m_inverse_view_matrix[3];
    camera_pos = vec3<f32>(_e60.x, _e60.y, _e60.z);
    let _e65 = camera_pos;
    let _e66 = pos_radius;
    light_to_camera = normalize((_e65 - _e66.xyz));
    let _e70 = pos_radius;
    let _e72 = light_to_camera;
    let _e74 = pos_radius[3u];
    let _e78 = pos_radius[3u];
    let _e80 = gl_VertexIndex_1;
    param = (_e70.xyz + (_e72 * _e74));
    let _e82 = u_push_constants.m_billboard_rotation;
    param_1 = _e82;
    param_2 = vec3(_e78);
    indexable = array<vec3<f32>, 4>(vec3<f32>(1f, 1f, 0f), vec3<f32>(1f, -1f, 0f), vec3<f32>(-1f, 1f, 0f), vec3<f32>(-1f, -1f, 0f));
    let _e84 = indexable[_e80];
    param_3 = _e84;
    let _e85 = param_3;
    let _e86 = param_2;
    param_3 = (_e85 * _e86);
    let _e88 = param_1;
    local_1 = _e88;
    let _e89 = param_3;
    local_2 = _e89;
    let _e90 = local_2;
    let _e91 = local_2;
    let _e92 = local_1;
    let _e96 = local_1[3u];
    let _e97 = local_2;
    let _e100 = local_1;
    local = (_e90 + (cross((cross(_e91, _e92.xyz) + (_e97 * _e96)), _e100.xyz) * 2f));
    let _e105 = local;
    param_3 = _e105;
    let _e106 = param_3;
    let _e107 = param;
    param_3 = (_e106 + _e107);
    let _e109 = param_3;
    local_3 = vec4<f32>(_e109.x, _e109.y, _e109.z, 1f);
    let _e114 = local_3;
    world_pos = _e114;
    let _e116 = u_camera.m_projection_view_matrix;
    let _e117 = world_pos;
    pv = (_e116 * _e117);
    let _e120 = pv[2u];
    if (_e120 < 0f) {
        let _e122 = pv;
        let _e123 = _e122.xy;
        unnamed.gl_Position = vec4<f32>(_e123.x, _e123.y, 0f, 1f);
    } else {
        let _e128 = pv;
        unnamed.gl_Position = _e128;
    }
    return;
}

@vertex 
fn main(@builtin(instance_index) gl_InstanceIndex: u32, @builtin(vertex_index) gl_VertexIndex: u32) -> VertexOutput {
    gl_InstanceIndex_1 = i32(gl_InstanceIndex);
    gl_VertexIndex_1 = i32(gl_VertexIndex);
    main_1();
    let _e10 = unnamed.gl_Position.y;
    unnamed.gl_Position.y = -(_e10);
    let _e12 = light_idx;
    let _e13 = unnamed.gl_Position;
    return VertexOutput(_e12, _e13);
}
