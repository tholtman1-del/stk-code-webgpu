struct PushConstants {
    size: i32,
    sampleCount: i32,
    mipmapLevel: i32,
    mipmapCount: i32,
}

var<private> gl_GlobalInvocationID_1: vec3<u32>;
var<immediate> pc: PushConstants;
@group(0) @binding(16) 
var uSkybox_sampler: sampler;
@group(0) @binding(0) 
var uSkybox_image: texture_cube<f32>;
@group(0) @binding(1) 
var uIrradianceMap: texture_storage_2d_array<rgba8unorm,write>;

fn main_1() {
    var local: f32;
    var local_1: u32;
    var local_2: vec2<f32>;
    var local_3: vec3<f32>;
    var local_4: vec3<f32>;
    var pix: vec2<i32>;
    var uv: vec2<f32>;
    var face: i32;
    var normal: vec3<f32>;
    var param: i32;
    var param_1: vec2<f32>;
    var up: vec3<f32>;
    var right: vec3<f32>;
    var tangent: vec3<f32>;
    var irradiance: vec3<f32>;
    var weight: f32;
    var sampleCount: u32;
    var i: u32;
    var xi: vec2<f32>;
    var param_2: u32;
    var param_3: u32;
    var phi: f32;
    var cosTheta: f32;
    var sinTheta: f32;
    var sampleDir: vec3<f32>;
    var sampleVec: vec3<f32>;
    var sampleColor: vec3<f32>;
    var phi_206_: bool;

    let _e64 = gl_GlobalInvocationID_1;
    pix = bitcast<vec2<i32>>(_e64.xy);
    let _e68 = pix[0u];
    let _e70 = pc.size;
    let _e71 = (_e68 >= _e70);
    phi_206_ = _e71;
    if !(_e71) {
        let _e74 = pix[1u];
        let _e76 = pc.size;
        phi_206_ = (_e74 >= _e76);
    }
    let _e79 = phi_206_;
    if _e79 {
        return;
    }
    let _e80 = pix;
    let _e85 = pc.size;
    let _e88 = pc.size;
    uv = ((vec2<f32>(_e80) + vec2(0.5f)) / vec2<f32>(f32(_e85), f32(_e88)));
    let _e93 = gl_GlobalInvocationID_1[2u];
    face = bitcast<i32>(_e93);
    let _e95 = face;
    param = _e95;
    let _e96 = uv;
    param_1 = _e96;
    let _e97 = param_1;
    param_1 = ((_e97 * 2f) - vec2(1f));
    let _e101 = param;
    if (_e101 == 0i) {
        let _e104 = param_1[1u];
        let _e107 = param_1[0u];
        local_3 = vec3<f32>(1f, -(_e104), -(_e107));
    } else {
        let _e110 = param;
        if (_e110 == 1i) {
            let _e113 = param_1[1u];
            let _e116 = param_1[0u];
            local_3 = vec3<f32>(-1f, -(_e113), _e116);
        } else {
            let _e118 = param;
            if (_e118 == 2i) {
                let _e121 = param_1[0u];
                let _e123 = param_1[1u];
                local_3 = vec3<f32>(_e121, 1f, _e123);
            } else {
                let _e125 = param;
                if (_e125 == 3i) {
                    let _e128 = param_1[0u];
                    let _e130 = param_1[1u];
                    local_3 = vec3<f32>(_e128, -1f, -(_e130));
                } else {
                    let _e133 = param;
                    if (_e133 == 4i) {
                        let _e136 = param_1[0u];
                        let _e138 = param_1[1u];
                        local_3 = vec3<f32>(_e136, -(_e138), 1f);
                    } else {
                        let _e141 = param;
                        if (_e141 == 5i) {
                            let _e144 = param_1[0u];
                            let _e147 = param_1[1u];
                            local_3 = vec3<f32>(-(_e144), -(_e147), -1f);
                        }
                    }
                }
            }
        }
    }
    let _e150 = local_3;
    local_4 = normalize(_e150);
    let _e152 = local_4;
    normal = _e152;
    let _e154 = normal[1u];
    up = select(vec3<f32>(1f, 0f, 0f), vec3<f32>(0f, 1f, 0f), vec3((abs(_e154) < 0.999f)));
    let _e159 = up;
    let _e160 = normal;
    right = normalize(cross(_e159, _e160));
    let _e163 = normal;
    let _e164 = right;
    tangent = cross(_e163, _e164);
    irradiance = vec3<f32>(0f, 0f, 0f);
    weight = 0f;
    let _e167 = pc.sampleCount;
    sampleCount = bitcast<u32>(_e167);
    i = 0u;
    loop {
        let _e169 = i;
        let _e170 = sampleCount;
        if (_e169 < _e170) {
            let _e172 = i;
            param_2 = _e172;
            let _e173 = sampleCount;
            param_3 = _e173;
            let _e174 = param_2;
            let _e176 = param_3;
            let _e179 = param_2;
            local_1 = _e179;
            let _e180 = local_1;
            let _e183 = local_1;
            local_1 = ((_e180 << bitcast<u32>(16u)) | (_e183 >> bitcast<u32>(16u)));
            let _e187 = local_1;
            let _e191 = local_1;
            local_1 = (((_e187 & 1431655765u) << bitcast<u32>(1u)) | ((_e191 & 2863311530u) >> bitcast<u32>(1u)));
            let _e196 = local_1;
            let _e200 = local_1;
            local_1 = (((_e196 & 858993459u) << bitcast<u32>(2u)) | ((_e200 & 3435973836u) >> bitcast<u32>(2u)));
            let _e205 = local_1;
            let _e209 = local_1;
            local_1 = (((_e205 & 252645135u) << bitcast<u32>(4u)) | ((_e209 & 4042322160u) >> bitcast<u32>(4u)));
            let _e214 = local_1;
            let _e218 = local_1;
            local_1 = (((_e214 & 16711935u) << bitcast<u32>(8u)) | ((_e218 & 4278255360u) >> bitcast<u32>(8u)));
            let _e223 = local_1;
            local = (f32(_e223) * 0.00000000023283064f);
            let _e226 = local;
            local_2 = vec2<f32>((f32(_e174) / f32(_e176)), _e226);
            let _e228 = local_2;
            xi = _e228;
            let _e230 = xi[0u];
            phi = (6.2831855f * _e230);
            let _e233 = xi[1u];
            cosTheta = sqrt((1f - _e233));
            let _e237 = xi[1u];
            sinTheta = sqrt(_e237);
            let _e239 = phi;
            let _e241 = sinTheta;
            let _e243 = phi;
            let _e245 = sinTheta;
            let _e247 = cosTheta;
            sampleDir = vec3<f32>((cos(_e239) * _e241), (sin(_e243) * _e245), _e247);
            let _e249 = right;
            let _e251 = sampleDir[0u];
            let _e253 = tangent;
            let _e255 = sampleDir[1u];
            let _e258 = normal;
            let _e260 = sampleDir[2u];
            sampleVec = normalize((((_e249 * _e251) + (_e253 * _e255)) + (_e258 * _e260)));
            let _e264 = sampleVec;
            let _e265 = textureSampleLevel(uSkybox_image, uSkybox_sampler, _e264, 0f);
            sampleColor = _e265.xyz;
            let _e267 = sampleColor;
            let _e268 = cosTheta;
            let _e270 = irradiance;
            irradiance = (_e270 + (_e267 * _e268));
            let _e272 = cosTheta;
            let _e273 = weight;
            weight = (_e273 + _e272);
            continue;
        } else {
            break;
        }
        continuing {
            let _e275 = i;
            i = (_e275 + bitcast<u32>(1i));
        }
    }
    let _e278 = weight;
    let _e279 = irradiance;
    irradiance = (_e279 / vec3(_e278));
    let _e282 = pix;
    let _e283 = face;
    let _e286 = vec3<i32>(_e282.x, _e282.y, _e283);
    let _e287 = irradiance;
    textureStore(uIrradianceMap, vec2<i32>(_e286.x, _e286.y), i32(_e286.z), vec4<f32>(_e287.x, _e287.y, _e287.z, 1f));
    return;
}

@compute @workgroup_size(16, 16, 1) 
fn main(@builtin(global_invocation_id) gl_GlobalInvocationID: vec3<u32>) {
    gl_GlobalInvocationID_1 = gl_GlobalInvocationID;
    main_1();
}
