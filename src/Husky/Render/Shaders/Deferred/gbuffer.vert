#version 450
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform CameraUniformBufferObject
{
    mat4x4 view;
    mat4x4 projection;
    vec3 positionWS;
} camera;

layout (set = 1, binding = 0) uniform MeshUniformBufferObject
{
    mat4x4 model;
} mesh;

// WS - world space
// VS - view space
// LS - local space
layout (location = 0) in vec3 positionLS;

#if HAS_NORMAL
    layout (location = 1) in vec3 normalLS;
#endif

#if HAS_TANGENT
    layout (location = 2) in TANGENT_TYPE tangentLS;
#endif

#if HAS_TEXCOORD_0
    layout (location = 3) in vec2 inTexCoord;
#endif

layout (location = 0) out vec3 outPositionVS;

#if HAS_NORMAL
    layout (location = 1) out vec3 outNormalVS;
#endif

#if HAS_TANGENT
    layout (location = 2) out vec3 outTangentVS;
#endif

#if HAS_TEXCOORD_0
    layout (location = 3) out vec2 outTexCoord;
#endif

void main()
{
    mat4 viewModel = camera.view * mesh.model;

    vec3 truePositionLS = vec3(positionLS.x, -positionLS.y, positionLS.z);
    vec4 positionWS = mesh.model * vec4(truePositionLS, 1.0);
    vec4 positionVS = viewModel * vec4(truePositionLS, 1.0);

    // TODO proper normal transform

    outPositionVS = positionVS.xyz;
    #if HAS_NORMAL
        //outNormalVS = (viewModel * vec4(normalLS, 0.0)).xyz;
        outNormalVS = normalLS;
    #endif

    #if HAS_TEXCOORD_0
        outTexCoord = inTexCoord;
    #endif

    #if HAS_TANGENT
        outTangentVS = (viewModel * vec4(tangentLS.xyz, 0.0)).xyz;
    #endif

    gl_Position = camera.projection * positionVS;
}
