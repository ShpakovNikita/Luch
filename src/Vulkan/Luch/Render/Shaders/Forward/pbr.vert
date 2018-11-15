#version 450
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define MAX_LIGHTS_COUNT 8

struct Light
{
    vec4 color;
    vec4 direction;
    bool enabled;
    int type;
    float spotlightAngle;
    float range;
    float intensity;
    vec3 padding;
};

layout (set = 0, binding = 0) uniform LightBufferObject
{
    Light lights[MAX_LIGHTS_COUNT];
} lights;

layout (set = 1, binding = 0) uniform CameraUniformBufferObject
{
    mat4x4 view;
    mat4x4 projection;
    vec3 positionWS;
} camera;

layout (set = 2, binding = 0) uniform MeshUniformBufferObject
{
    mat4x4 model;
} mesh;

// WS - world space
// VS - view space
// LS - local space
layout (location = 0) in vec3 positionLS;
layout (location = 1) in vec3 normalLS;

#ifdef HAS_TANGENT
layout (location = 2) in vec3 tangentLS;
#endif

layout (location = 3) in vec2 inTexCoord;

layout (location = 0) out vec3 outPositionVS;
layout (location = 1) out vec3 outNormalVS;
layout (location = 2) out vec3 outTangentVS;
layout (location = 3) out vec2 outTexCoord;

void main()
{
    mat4 viewModel = camera.view * mesh.model;
    
    vec4 positionWS = mesh.model * vec4(positionLS, 1.0);
    vec4 positionVS = viewModel * vec4(positionLS, 1.0);

    vec4 intermediate = camera.projection * positionVS;

    // TODO proper normal transform

    outPositionVS = positionVS.xyz;
    outNormalVS = (viewModel * vec4(normalLS, 0.0)).xyz;
    outTexCoord = inTexCoord;
#ifdef HAS_TANGENT
    outTangentVS = (viewModel * vec4(tangentLS, 0.0)).xyz;
#endif
    // TODO figure out handedness
    //gl_Position = vec4(intermediate.x, -intermediate.y, intermediate.zw);
    gl_Position = intermediate;
}
