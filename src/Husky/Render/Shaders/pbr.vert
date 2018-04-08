#version 450
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform CameraUniformBufferObject
{
    mat4x4 view;
    mat4x4 projection;
} camera;

layout (set = 1, binding = 0) uniform MeshUniformBufferObject
{
    mat4x4 model;
} mesh;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
//layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 inTexCoord;
layout (location = 1) out vec3 outNormal;
layout (location = 3) out vec2 outTexCoord;

void main()
{
   outTexCoord = inTexCoord;
   outTexCoord = inTexCoord;
   outNormal = vec3(normal.x, -normal.y, normal.z);
   vec4 intermediate = camera.projection * camera.view * mesh.model * vec4(position, 1.0);
   gl_Position = vec4(intermediate.x, -intermediate.y, intermediate.zw);
}
