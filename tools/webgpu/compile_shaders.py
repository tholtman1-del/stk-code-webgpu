#!/usr/bin/env python3
"""Offline GE shader pipeline for the WebGPU backend.

GLSL (data/shaders/ge_shaders) -> SPIR-V (glslangValidator) -> WGSL (naga).
WebGPU has no runtime GLSL compiler, so every shader variant the driver may
request is generated ahead of time into data/shaders/ge_wgsl/<variant>/.
"""
import argparse, os, re, subprocess, sys, tempfile, pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
SRC = ROOT / "data" / "shaders" / "ge_shaders"
OUT = ROOT / "data" / "shaders" / "ge_wgsl"
STAGES = {".vert": "vert", ".frag": "frag", ".comp": "comp"}

# WebGPU feature set: no bindless / descriptor indexing, one material bound
# per draw, no multi-draw-indirect.
def predefines(pbr):
    lines = ["#version 450"]
    if pbr:
        lines.append("#define PBR_ENABLED 1")
    lines.append("#define SAMPLER_SIZE 1")
    lines.append("#define TOTAL_MESH_TEXTURE_LAYER %d" % (8 if pbr else 2))
    lines.append("#define GE_SAMPLE_TEX_INDEX int")
    lines.append("#define GE_WEBGPU 1")
    return "\n".join(lines) + "\n"

def compile_one(src, variant, pbr, verbose):
    stage = STAGES[src.suffix]
    out_dir = OUT / variant
    out_dir.mkdir(parents=True, exist_ok=True)
    wgsl = out_dir / (src.name + ".wgsl")
    with tempfile.TemporaryDirectory() as tmp:
        # Write the predefined header + source next to the original so that
        # relative #include paths keep working.
        full = SRC / (".webgpu_tmp_" + src.name)
        full.write_text(predefines(pbr) +
            "#extension GL_GOOGLE_include_directive : enable\n" +
            src.read_text())
        spv = pathlib.Path(tmp) / "out.spv"
        try:
            r = subprocess.run(["glslangValidator", "-V", "--target-env",
                "vulkan1.1", "-S", stage, str(full), "-o", str(spv)],
                capture_output=True, text=True)
        finally:
            full.unlink()
        if r.returncode != 0:
            return False, "glslang: " + (r.stdout + r.stderr).strip()
        # naga's SPIR-V reader cannot follow combined image samplers through
        # function calls, so flatten everything first and split them.
        opt = pathlib.Path(tmp) / "opt.spv"
        r = subprocess.run(["spirv-opt", "--freeze-spec-const",
            "--fold-spec-const-op-composite", "--inline-entry-points-exhaustive",
            "--split-combined-image-sampler", "--eliminate-dead-functions",
            "--eliminate-dead-code-aggressive", str(spv), "-o", str(opt)],
            capture_output=True, text=True)
        if r.returncode != 0:
            return False, "spirv-opt: " + (r.stdout + r.stderr).strip()
        spv = opt
        remap_sampler_bindings(spv, tmp)
        r = subprocess.run(["naga", str(spv), str(wgsl)],
            capture_output=True, text=True)
        if r.returncode != 0:
            return False, "naga: " + (r.stdout + r.stderr).strip()
        text = push_constants_to_uniform(wgsl.read_text())
        if stage == "vert":
            text = unpack_packed_inputs(text)
        wgsl.write_text(text)
    return True, ""

# WebGPU has no push constants (naga emits var<immediate>). They become a
# uniform buffer with a dynamic offset, filled by the driver per pipeline.
PUSH_CONSTANTS_BINDING = "@group(1) @binding(4)"

def push_constants_to_uniform(src):
    return re.sub(r"var<immediate> (\w+): (\w+);",
        PUSH_CONSTANTS_BINDING + r"\nvar<uniform> \1: \2;", src)

# Normals and tangents are A2B10G10R10 snorm in S3DVertexSkinnedMesh, which
# WebGPU has no vertex format for. Those inputs are read as u32 and unpacked.
PACKED_SNORM_INPUTS = {1: "v_normal", 5: "v_tangent"}
UNPACK_FN = """
fn ge_unpack_snorm_10_10_10_2(p: u32) -> vec4<f32> {
    let v = vec4<f32>(f32(bitcast<i32>(p << 22u) >> 22u) / 511.0,
        f32(bitcast<i32>(p << 12u) >> 22u) / 511.0,
        f32(bitcast<i32>(p << 2u) >> 22u) / 511.0,
        f32(bitcast<i32>(p) >> 30u));
    return max(v, vec4<f32>(-1.0));
}
"""

def unpack_packed_inputs(src):
    unpacked = []
    for location, name in PACKED_SNORM_INPUTS.items():
        decl = "@location(%d) %s: vec4<f32>" % (location, name)
        if decl in src:
            src = src.replace(decl, "@location(%d) %s_packed: u32" %
                (location, name))
            unpacked.append(name)
    if not unpacked:
        return src
    m = re.search(r"@vertex\s*\nfn main\([^)]*\)[^{]*\{\n", src)
    lets = "".join("    let %s = ge_unpack_snorm_10_10_10_2(%s_packed);\n" %
        (n, n) for n in unpacked)
    src = src[:m.end()] + lets + src[m.end():]
    return src + UNPACK_FN

# Samplers produced by --split-combined-image-sampler share the binding of
# their texture. WGSL needs unique bindings, so samplers move up by
# SAMPLER_BINDING_OFFSET; the driver uses the same offset in its layouts.
SAMPLER_BINDING_OFFSET = 16

def remap_sampler_bindings(spv, tmp):
    asm = pathlib.Path(tmp) / "remap.spvasm"
    subprocess.run(["spirv-dis", "--raw-id", str(spv), "-o", str(asm)],
        check=True)
    lines = asm.read_text().splitlines()
    sampler_types, ptr_types, sampler_vars = set(), set(), set()
    for l in lines:
        t = l.split()
        if len(t) >= 3 and t[1] == "=" and t[2] == "OpTypeSampler":
            sampler_types.add(t[0])
    for l in lines:
        t = l.split()
        if (len(t) >= 5 and t[2] == "OpTypePointer" and t[4] in sampler_types):
            ptr_types.add(t[0])
    for l in lines:
        t = l.split()
        if len(t) >= 4 and t[2] == "OpVariable" and t[3] in ptr_types:
            sampler_vars.add(t[0])
    out = []
    for l in lines:
        t = l.split()
        if (len(t) == 4 and t[0] == "OpDecorate" and t[1] in sampler_vars and
            t[2] == "Binding"):
            l = "OpDecorate %s Binding %d" % (t[1],
                int(t[3]) + SAMPLER_BINDING_OFFSET)
        out.append(l)
    asm.write_text("\n".join(out) + "\n")
    subprocess.run(["spirv-as", "--preserve-numeric-ids", str(asm), "-o",
        str(spv)], check=True)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("filter", nargs="?", default="")
    args = ap.parse_args()
    failures = 0
    for variant, pbr in (("basic", False), ("pbr", True)):
        for src in sorted(SRC.iterdir()):
            if src.suffix not in STAGES or args.filter not in src.name:
                continue
            ok, msg = compile_one(src, variant, pbr, args.verbose)
            print("%-4s %-6s %s" % ("ok" if ok else "FAIL", variant, src.name))
            if not ok:
                failures += 1
                if args.verbose:
                    print("    " + msg.replace("\n", "\n    ")[:1500])
    print("%d failure(s)" % failures)
    sys.exit(1 if failures else 0)

if __name__ == "__main__":
    main()
