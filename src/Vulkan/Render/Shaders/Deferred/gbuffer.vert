#version 450
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (std140, set = 0, binding = 0) uniform CameraUniformBufferObject
{
    mat4x4 view;
    mat4x4 projection;
    vec3 positionWS;
} camera;

layout (std140, set = 1, binding = 0) uniform MeshUniformBufferObject
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
    #if HAS_BITANGENT_DIRECTION
        layout (location = 2) in vec4 tangentLS;
    #else
        layout (location = 2) in vec3 tangentLS;
    #endif
#endif

#if HAS_TEXCOORD_0
    layout (location = 3) in vec2 inTexCoord;
#endif

layout (location = 0) out vec3 outPositionVS;

#if HAS_NORMAL
    layout (location = 1) out vec3 outNormalVS;
#endif

#if HAS_TANGENT
    layout (location = 2) out vec4 outTangentVS;
#endif

#if HAS_TEXCOORD_0
    layout (location = 3) out vec2 outTexCoord;
#endif

void main()
{
    mat4 viewModel = camera.view * mesh.model;

    vec4 positionVS = viewModel * vec4(positionLS.x, -positionLS.y, positionLS.z, 1.0);

    outPositionVS = positionVS.xyz;

    mat4x4 normalMatrix = transpose(inverse(viewModel));

    #if HAS_NORMAL
        outNormalVS = (normalMatrix * vec4(normalLS, 0.0)).xyz;
        outNormalVS.y = -outNormalVS.y;
    #endif

    #if HAS_TEXCOORD_0
        outTexCoord = inTexCoord;
    #endif

    #if HAS_TANGENT
        // Normal matrix or viewmodel matrix?
        outTangentVS.xyz = (normalMatrix * vec4(tangentLS.xyz, 0.0)).xyz;
        #if HAS_BITANGENT_DIRECTION
            outTangentVS.w = tangentLS.w;
        #else
            outTangentVS.w = 1.0;
        #endif
    #endif

    gl_Position = camera.projection * positionVS;
}
