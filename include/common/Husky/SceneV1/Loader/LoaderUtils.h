#pragma once

#include <Husky/Graphics/SamplerAddressMode.h>
#include <Husky/Graphics/SamplerMinMagFilter.h>
#include <Husky/Graphics/SamplerMipFilter.h>
#include <Husky/glTF2/glTF.h>
#include <Husky/Assert.h>
#include <Husky/SceneV1/Primitive.h>

namespace Husky::SceneV1::Loader
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
        uint32* indices,
        Vec3* positions,
        Vec3* normals,
        Vec2* texcoords,
        Vec4* outTangents);
}
