#pragma once

#include <Husky/Types.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/UniquePtr.h>
#include <Husky/Format.h>
#include <Husky/SceneV1/Forwards.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/Vulkan/GlslShaderCompiler.h>
#include <Husky/Vulkan/DescriptorSetBinding.h>

namespace Husky::Render
{
    using namespace Husky::Vulkan;

#pragma pack(push)
#pragma pack(1)
    struct CameraUniformBuffer
    {
        Mat4x4 view;
        Mat4x4 projection;
        Vec3 position;
        float32 _padding0;
    };

    static_assert(sizeof(CameraUniformBuffer) % 4 == 0);

    struct MeshUniformBuffer
    {
        Mat4x4 transform;
    };

    static_assert(sizeof(MeshUniformBuffer) % 4 == 0);

    struct MaterialPushConstants
    {
        Vec4 baseColorFactor;
        Vec3 emissiveFactor;
        int32 hasBaseColorTexture;
        int32 hasMetallicRoughnessTexture;
        int32 hasNormalTexture;
        int32 hasOcclusionTexture;
        int32 hasEmissiveTexture;
        int32 isAlphaMask;
        float32 alphaCutoff;
        float32 metallicFactor;
        float32 roughnessFactor;
        float32 normalScale;
        float32 occlusionStrength;
    };

    static_assert(sizeof(MaterialPushConstants) % 4 == 0);

    struct LightUniform
    {
        Vec4 positionWS = { 0.0, 0.0, 0.0, 1.0 };
        Vec4 directionWS = { 0.0, 0.0, 0.0, 0.0 };
        Vec4 positionVS = { 0.0, 0.0, 0.0, 1.0 };
        Vec4 directionVS = { 0.0, 0.0, 0.0, 0.01 };
        Vec4 color = { 1.0, 1.0, 1.0, 1.0 };
        int32 enabled = 0;
        int32 type = 0;
        float32 spotlightAngle = 0;
        float32 range = 0;
        float32 intensity = 1.0;
        float32 __padding0 = 0.0;
        float32 __padding1 = 0.0;
        float32 __padding2 = 0.0;
    };

    static_assert(sizeof(LightUniform) % 4 == 0);
#pragma pack(pop)

    struct CommandPoolCreateResult
    {
        vk::ResultValue<vk::CommandPool> graphicsCommandPool;
        vk::ResultValue<vk::CommandPool> presentCommandPool;
        vk::ResultValue<vk::CommandPool> computeCommandPool;

        Vector<vk::CommandPool> uniqueCommandPools;
    };

    struct Shader
    {
        GLSLShaderCompiler::Bytecode bytecode;
        RefPtr<ShaderModule> module;
    };
}