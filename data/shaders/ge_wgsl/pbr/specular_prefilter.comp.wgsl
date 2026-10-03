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
var uPrefilterMap: texture_storage_2d_array<rgba8unorm,write>;

fn main_1() {
    var local: f32;
    var local_1: f32;
    var local_2: f32;
    var local_3: f32;
    var local_4: f32;
    var local_5: u32;
    var local_6: vec2<f32>;
    var local_7: vec3<f32>;
    var local_8: vec3<f32>;
    var pix: vec2<i32>;
    var uv: vec2<f32>;
    var face: i32;
    var roughness: f32;
    var R: vec3<f32>;
    var param: i32;
    var param_1: vec2<f32>;
    var V: vec3<f32>;
    var color: vec4<f32>;
    var prefilteredColor: vec3<f32>;
    var totalWeight: f32;
    var sampleCount: u32;
    var i: u32;
    var xi: vec2<f32>;
    var param_2: u32;
    var param_3: u32;
    var a: f32;
    var phi: f32;
    var cosTheta: f32;
    var sinTheta: f32;
    var H: vec3<f32>;
    var up: vec3<f32>;
    var tangent: vec3<f32>;
    var bitangent: vec3<f32>;
    var TBN: mat3x3<f32>;
    var L: vec3<f32>;
    var NdotL: f32;
    var NoH: f32;
    var VoH: f32;
    var D: f32;
    var param_4: f32;
    var param_5: f32;
    var pdf: f32;
    var omegaS: f32;
    var omegaP: f32;
    var mipLevel: f32;
    var sampleColor: vec3<f32>;
    var phi_235_: bool;

    let _e87 = gl_GlobalInvocationID_1;
    pix = bitcast<vec2<i32>>(_e87.xy);
    let _e91 = pix[0u];
    let _e93 = pc.size;
    let _e94 = (_e91 >= _e93);
    phi_235_ = _e94;
    if !(_e94) {
        let _e97 = pix[1u];
        let _e99 = pc.size;
        phi_235_ = (_e97 >= _e99);
    }
    let _e102 = phi_235_;
    if _e102 {
        return;
    }
    let _e103 = pix;
    let _e108 = pc.size;
    let _e111 = pc.size;
    uv = ((vec2<f32>(_e103) + vec2(0.5f)) / vec2<f32>(f32(_e108), f32(_e111)));
    let _e116 = gl_GlobalInvocationID_1[2u];
    face = bitcast<i32>(_e116);
    let _e119 = pc.mipmapLevel;
    let _e122 = pc.mipmapCount;
    roughness = (f32(_e119) / f32((_e122 - 1i)));
    let _e126 = face;
    param = _e126;
    let _e127 = uv;
    param_1 = _e127;
    let _e128 = param_1;
    param_1 = ((_e128 * 2f) - vec2(1f));
    let _e132 = param;
    if (_e132 == 0i) {
        let _e135 = param_1[1u];
        let _e138 = param_1[0u];
        local_7 = vec3<f32>(1f, -(_e135), -(_e138));
    } else {
        let _e141 = param;
        if (_e141 == 1i) {
            let _e144 = param_1[1u];
            let _e147 = param_1[0u];
            local_7 = vec3<f32>(-1f, -(_e144), _e147);
        } else {
            let _e149 = param;
            if (_e149 == 2i) {
                let _e152 = param_1[0u];
                let _e154 = param_1[1u];
                local_7 = vec3<f32>(_e152, 1f, _e154);
            } else {
                let _e156 = param;
                if (_e156 == 3i) {
                    let _e159 = param_1[0u];
                    let _e161 = param_1[1u];
                    local_7 = vec3<f32>(_e159, -1f, -(_e161));
                } else {
                    let _e164 = param;
                    if (_e164 == 4i) {
                        let _e167 = param_1[0u];
                        let _e169 = param_1[1u];
                        local_7 = vec3<f32>(_e167, -(_e169), 1f);
                    } else {
                        let _e172 = param;
                        if (_e172 == 5i) {
                            let _e175 = param_1[0u];
                            let _e178 = param_1[1u];
                            local_7 = vec3<f32>(-(_e175), -(_e178), -1f);
                        }
                    }
                }
            }
        }
    }
    let _e181 = local_7;
    local_8 = normalize(_e181);
    let _e183 = local_8;
    R = _e183;
    let _e184 = R;
    V = normalize(_e184);
    let _e186 = roughness;
    if (_e186 == 0f) {
        let _e188 = V;
        let _e189 = textureSampleLevel(uSkybox_image, uSkybox_sampler, _e188, 0f);
        color = _e189;
        let _e190 = pix;
        let _e191 = face;
        let _e194 = vec3<i32>(_e190.x, _e190.y, _e191);
        let _e195 = color;
        textureStore(uPrefilterMap, vec2<i32>(_e194.x, _e194.y), i32(_e194.z), _e195);
        return;
    }
    prefilteredColor = vec3<f32>(0f, 0f, 0f);
    totalWeight = 0f;
    let _e202 = pc.sampleCount;
    sampleCount = bitcast<u32>(_e202);
    i = 0u;
    loop {
        let _e204 = i;
        let _e205 = sampleCount;
        if (_e204 < _e205) {
            let _e207 = i;
            param_2 = _e207;
            let _e208 = sampleCount;
            param_3 = _e208;
            let _e209 = param_2;
            let _e211 = param_3;
            let _e214 = param_2;
            local_5 = _e214;
            let _e215 = local_5;
            let _e218 = local_5;
            local_5 = ((_e215 << bitcast<u32>(16u)) | (_e218 >> bitcast<u32>(16u)));
            let _e222 = local_5;
            let _e226 = local_5;
            local_5 = (((_e222 & 1431655765u) << bitcast<u32>(1u)) | ((_e226 & 2863311530u) >> bitcast<u32>(1u)));
            let _e231 = local_5;
            let _e235 = local_5;
            local_5 = (((_e231 & 858993459u) << bitcast<u32>(2u)) | ((_e235 & 3435973836u) >> bitcast<u32>(2u)));
            let _e240 = local_5;
            let _e244 = local_5;
            local_5 = (((_e240 & 252645135u) << bitcast<u32>(4u)) | ((_e244 & 4042322160u) >> bitcast<u32>(4u)));
            let _e249 = local_5;
            let _e253 = local_5;
            local_5 = (((_e249 & 16711935u) << bitcast<u32>(8u)) | ((_e253 & 4278255360u) >> bitcast<u32>(8u)));
            let _e258 = local_5;
            local_4 = (f32(_e258) * 0.00000000023283064f);
            let _e261 = local_4;
            local_6 = vec2<f32>((f32(_e209) / f32(_e211)), _e261);
            let _e263 = local_6;
            xi = _e263;
            let _e264 = roughness;
            let _e265 = roughness;
            a = (_e264 * _e265);
            let _e268 = xi[0u];
            phi = (6.2831855f * _e268);
            let _e271 = xi[1u];
            let _e273 = a;
            let _e274 = a;
            let _e278 = xi[1u];
            cosTheta = sqrt(((1f - _e271) / (1f + (((_e273 * _e274) - 1f) * _e278))));
            let _e283 = cosTheta;
            let _e284 = cosTheta;
            sinTheta = sqrt((1f - (_e283 * _e284)));
            let _e288 = phi;
            let _e290 = sinTheta;
            let _e292 = phi;
            let _e294 = sinTheta;
            let _e296 = cosTheta;
            H = vec3<f32>((cos(_e288) * _e290), (sin(_e292) * _e294), _e296);
            let _e299 = V[1u];
            up = select(vec3<f32>(1f, 0f, 0f), vec3<f32>(0f, 1f, 0f), vec3((abs(_e299) < 0.999f)));
            let _e304 = up;
            let _e305 = V;
            tangent = normalize(cross(_e304, _e305));
            let _e308 = V;
            let _e309 = tangent;
            bitangent = cross(_e308, _e309);
            let _e311 = tangent;
            let _e312 = bitangent;
            let _e313 = V;
            TBN = mat3x3<f32>(vec3<f32>(_e311.x, _e311.y, _e311.z), vec3<f32>(_e312.x, _e312.y, _e312.z), vec3<f32>(_e313.x, _e313.y, _e313.z));
            let _e327 = TBN;
            let _e328 = H;
            H = normalize((_e327 * _e328));
            let _e331 = V;
            let _e333 = H;
            L = normalize(reflect(-(_e331), _e333));
            let _e336 = V;
            let _e337 = L;
            NdotL = max(dot(_e336, _e337), 0f);
            let _e340 = NdotL;
            if (_e340 > 0f) {
                let _e342 = V;
                let _e343 = H;
                NoH = max(dot(_e342, _e343), 0f);
                let _e346 = V;
                let _e347 = H;
                VoH = max(dot(_e346, _e347), 0f);
                let _e350 = roughness;
                param_4 = _e350;
                let _e351 = NoH;
                param_5 = _e351;
                let _e352 = param_5;
                let _e353 = param_5;
                local = (1f - (_e352 * _e353));
                let _e356 = param_5;
                let _e357 = param_4;
                local_1 = (_e356 * _e357);
                let _e359 = param_4;
                let _e360 = local;
                let _e361 = local_1;
                let _e362 = local_1;
                local_2 = (_e359 / (_e360 + (_e361 * _e362)));
                let _e366 = local_2;
                let _e367 = local_2;
                local_3 = ((_e366 * _e367) * 0.31830987f);
                let _e370 = local_3;
                D = _e370;
                let _e371 = D;
                let _e372 = NoH;
                let _e374 = VoH;
                pdf = ((_e371 * _e372) / (4f * _e374));
                let _e377 = sampleCount;
                let _e379 = pdf;
                omegaS = (1f / (f32(_e377) * _e379));
                let _e383 = pc.size;
                let _e385 = pc.size;
                omegaP = (12.566371f / (6f * f32((_e383 * _e385))));
                let _e390 = omegaS;
                let _e391 = omegaP;
                mipLevel = (0.5f * log2((_e390 / _e391)));
                let _e395 = L;
                let _e396 = mipLevel;
                let _e397 = textureSampleLevel(uSkybox_image, uSkybox_sampler, _e395, _e396);
                sampleColor = _e397.xyz;
                let _e399 = sampleColor;
                let _e400 = NdotL;
                let _e402 = prefilteredColor;
                prefilteredColor = (_e402 + (_e399 * _e400));
                let _e404 = NdotL;
                let _e405 = totalWeight;
                totalWeight = (_e405 + _e404);
            }
            continue;
        } else {
            break;
        }
        continuing {
            let _e407 = i;
            i = (_e407 + bitcast<u32>(1i));
        }
    }
    let _e410 = prefilteredColor;
    let _e411 = totalWeight;
    prefilteredColor = (_e410 / vec3(_e411));
    let _e414 = pix;
    let _e415 = face;
    let _e418 = vec3<i32>(_e414.x, _e414.y, _e415);
    let _e419 = prefilteredColor;
    textureStore(uPrefilterMap, vec2<i32>(_e418.x, _e418.y), i32(_e418.z), vec4<f32>(_e419.x, _e419.y, _e419.z, 1f));
    return;
}

@compute @workgroup_size(16, 16, 1) 
fn main(@builtin(global_invocation_id) gl_GlobalInvocationID: vec3<u32>) {
    gl_GlobalInvocationID_1 = gl_GlobalInvocationID;
    main_1();
}
