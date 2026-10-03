#include "utils/subpass_input.glsl"
GE_SUBPASS_INPUT(0, 0, u_hdr)

layout(location = 0) out vec4 o_color;

#include "utils/constants_utils.glsl"

void main()
{
    o_color = vec4(convertColor(GE_SUBPASS_LOAD(u_hdr).xyz), 1.0);
}
