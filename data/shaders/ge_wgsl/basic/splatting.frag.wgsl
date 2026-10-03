diagnostic(off, derivative_uniformity);
struct FragmentOutput {
    @location(0) member: vec4<f32>,
    @location(1) member_1: vec4<f32>,
}

var<private> o_color: vec4<f32>;
var<private> o_normal: vec4<f32>;

fn main_1() {
    return;
}

@fragment 
fn main() -> FragmentOutput {
    main_1();
    let _e2 = o_color;
    let _e3 = o_normal;
    return FragmentOutput(_e2, _e3);
}
