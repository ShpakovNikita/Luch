#pragma once

#include <Luch/Types.h>
#include <Luch/VectorTypes.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Graphics/Format.h>
#include <Luch/SceneV1/SceneV1Forwards.h>

namespace Luch::Render
{
    using namespace Luch::Graphics;

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
        Mat4x4 view = Mat4x4 { 1.0f };
        Mat4x4 inverseView = Mat4x4 { 1.0f };
        Mat4x4 projection = Mat4x4 { 1.0f };
        Mat4x4 inverseProjection = Mat4x4 { 1.0f };
        Mat4x4 viewProjection = Mat4x4 { 1.0f };
        Mat4x4 inverseViewProjection = Mat4x4 { 1.0f };
        Vec4 positionWS = Vec4 { 0.0, 0.0, 0.0, 1.0 };
        Vec2 zMinMax = Vec2 { 0.0, 1.0 };
    };

    static_assert(sizeof(CameraUniform) % 4 == 0);

    struct MeshUniform
    {
        Mat4x4 transform = Mat4x4 { 1.0f };
        Mat4x4 inverseTransform = Mat4x4 { 1.0f };
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

    struct LightingParamsUniform
    {
        uint16 lightCount = 0;
        uint16 __padding0 = 0;
        int32 __padding1 = 0;
        int32 __padding2 = 0;
        int32 __padding3 = 0;
    };

    static_assert(sizeof(LightingParamsUniform) % 4 == 0);

    struct LightUniform
    {
        Vec4 positionWS = { 0.0, 0.0, 0.0, 1.0 };
        Vec4 directionWS = { 0.0, 0.0, 0.0, 0.0 };
        Vec4 color = { 1.0, 1.0, 1.0, 1.0 };
        uint16 enabled = 1;
        uint16 type = 0;
        float32 innerConeAngle = 0;
        float32 outerConeAngle = 0;
        float32 range = 0;
        float32 intensity = 1.0;
        float32 __padding0 = 0.0;
        float32 __padding1 = 0.0;
        float32 __padding2 = 0.0;
    };

    static_assert(sizeof(LightUniform) % 4 == 0);
#pragma pack(pop)
}
