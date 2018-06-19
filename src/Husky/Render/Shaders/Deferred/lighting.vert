#version 450
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPositionLS;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 outTexCoord;

void main()
{
    outTexCoord = inTexCoord;
    gl_Position.xyz = inPositionLS;
}
