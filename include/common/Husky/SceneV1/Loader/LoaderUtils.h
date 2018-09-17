#pragma once

#include <Husky/Vulkan.h>
#include <Husky/glTF2/glTF.h>
#include <Husky/Assert.h>
#include <Husky/SceneV1/Primitive.h>

namespace Husky::SceneV1::Loader
{
    struct MinFilter
    {
        vk::Filter minFilter;
        vk::SamplerMipmapMode mipmapMode;
        float32 minLod = 0;
        float32 maxLod = 0;
    };

    vk::SamplerAddressMode ToVkSamplerAddresMode(glTF::WrapMode mode);
    vk::Filter ToVkMagFilter(glTF::MagFilter mode);
    MinFilter ToVkMinFilter(glTF::MinFilter mode);
    int32 CalculateStride(ComponentType componentType, AttributeType attributeType);
}
