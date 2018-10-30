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
    struct QuadVertex
    {
        Vec3 position;
        Vec2 texCoord;
    };

    struct Vertex
    {
        Vec3 position;
        Vec3 normal;
        Vec4 tangent;
        Vec2 texcoord;
    };

    struct CameraUniform
    {
        Mat4x4 view = Mat4x4 { 1.0f };;
        Mat4x4 inverseView = Mat4x4 { 1.0f };;
        Mat4x4 projection = Mat4x4 { 1.0f };;
        Mat4x4 inverseProjection = Mat4x4 { 1.0f };;
        Mat4x4 viewProjection = Mat4x4 { 1.0f };;
        Mat4x4 inverseViewProjection = Mat4x4 { 1.0f };;
        Vec4 position = Vec4 { 0.0, 0.0, 0.0, 1.0 };
        Vec2 zMinMax = Vec2 { 0.0, 1.0 };
    };

    static_assert(sizeof(CameraUniform) % 4 == 0);

    struct MeshUniform
    {
        Mat4x4 transform = Mat4x4 { 1.0f };
        Mat4x4 inverseTransform = Mat4x4 { 1.0f };;
    };

    static_assert(sizeof(MeshUniform) % 4 == 0);

    struct MaterialUniform
    {
        Vec4 baseColorFactor = Vec4 { 1.0 };
        Vec3 emissiveFactor = Vec3 { 1.0 };
        float32 alphaCutoff = 0.0f;
        float32 metallicFactor = 1.0f;
        float32 roughnessFactor = 1.0f;
        float32 normalScale = 1.0f;
        float32 occlusionStrength = 1.0f;
    };

    static_assert(sizeof(MaterialUniform) % 4 == 0);

    struct LightUniform
    {
        Vec4 positionWS = { 0.0, 0.0, 0.0, 1.0 };
        Vec4 directionWS = { 0.0, 0.0, 0.0, 0.0 };
        Vec4 color = { 1.0, 1.0, 1.0, 1.0 };
        int32 enabled = 1;
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
