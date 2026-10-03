// Vulkan reads the G-buffer through subpass input attachments. WebGPU has no
// subpasses, so there the G-buffer is bound as regular textures and read with
// texelFetch at the current fragment (depth uses an unfilterable-float layout).
#ifdef GE_WEBGPU
#define GE_SUBPASS_INPUT(index, bind, name) \
    layout(binding = bind) uniform sampler2D name;
#define GE_SUBPASS_LOAD(name) texelFetch(name, ivec2(gl_FragCoord.xy), 0)
#else
#define GE_SUBPASS_INPUT(index, bind, name) \
    layout(input_attachment_index = index, binding = bind) uniform subpassInput name;
#define GE_SUBPASS_LOAD(name) subpassLoad(name)
#endif
