#version 450
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (set = 2, binding = 0) uniform texture2D baseColorMap;
layout (set = 2, binding = 1) uniform texture2D roughnessMap;
layout (set = 2, binding = 2) uniform texture2D normalMap;
layout (set = 2, binding = 3) uniform texture2D occlusionMap;
layout (set = 2, binding = 4) uniform texture2D emissiveMap;
layout (set = 2, binding = 5) uniform sampler baseColorSampler;
layout (set = 2, binding = 6) uniform sampler roughnessSampler;
layout (set = 2, binding = 7) uniform sampler normalSampler;
layout (set = 2, binding = 8) uniform sampler occlusionSampler;
layout (set = 2, binding = 9) uniform sampler emissiveSampler;

layout (location = 1) in vec3 normal;
layout (location = 3) in vec2 texCoord;
layout (location = 0) out vec4 outColor;

void main()
{
   vec4 baseColor = texture(sampler2D(baseColorMap, baseColorSampler), texCoord);
   outColor = vec4(baseColor.xyz, 1.0);
}
