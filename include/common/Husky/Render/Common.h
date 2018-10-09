#pragma once

#include <Husky/Types.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/UniquePtr.h>
#include <Husky/Graphics/Format.h>
#include <Husky/SceneV1/SceneV1Forwards.h>

namespace Husky::Render
{
    using namespace Husky::Graphics;

#pragma pack(push, 1)
    struct Vertex
    {
        Vec3 position;
        Vec3 normal;
        Vec4 tangent;
        Vec2 texcoord;
    };

    struct CameraUniformBuffer
    {
        Mat4x4 view;
        Mat4x4 inverseView;
        Mat4x4 projection;
        Mat4x4 inverseProjection;
        Mat4x4 viewProjection;
        Mat4x4 inverseViewProjection;
        Vec4 position;
        Vec2 zMinMax;
    };

    static_assert(sizeof(CameraUniformBuffer) % 4 == 0);

    struct MeshUniformBuffer
    {
        Mat4x4 transform;
        Mat4x4 inverseTransform;
    };

    static_assert(sizeof(MeshUniformBuffer) % 4 == 0);

    struct MaterialUniform
    {
        Vec4 baseColorFactor;
        Vec3 emissiveFactor;
        float32 alphaCutoff;
        float32 metallicFactor;
        float32 roughnessFactor;
        float32 normalScale;
        float32 occlusionStrength;
    };

    static_assert(sizeof(MaterialUniform) % 4 == 0);

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


}
