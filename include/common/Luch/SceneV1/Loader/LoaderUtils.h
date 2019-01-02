#pragma once

#include <Luch/Graphics/SamplerAddressMode.h>
#include <Luch/Graphics/SamplerMinMagFilter.h>
#include <Luch/Graphics/SamplerMipFilter.h>
#include <Luch/glTF2/glTF.h>
#include <Luch/Assert.h>
#include <Luch/SceneV1/Primitive.h>

namespace Luch::SceneV1::Loader
{
    using namespace Graphics;

    struct MinFilter
    {
        SamplerMinMagFilter minFilter;
        SamplerMipFilter mipFilter;
        float32 minLod = 0;
        float32 maxLod = 0;
    };

    SamplerAddressMode ToSamplerAddresMode(glTF::WrapMode mode);
    SamplerMinMagFilter ToMagFilter(glTF::MagFilter mode);
    MinFilter ToMinFilter(glTF::MinFilter mode);
    int32 CalculateStride(AttributeType attributeType, ComponentType componentType);

    void GenerateTangents(
        glTF::PrimitiveMode mode,
        int32 indexCount,
        int32 vertexCount,
        uint32* indices,
        Vec3* positions,
        Vec3* normals,
        Vec2* texcoords,
        Vec4* outTangents);
}
