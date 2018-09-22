#pragma once

#include <Husky/Types.h>
#include <Husky/Graphics/CompareFunction.h>
#include <Husky/Graphics/SamplerAddressMode.h>
#include <Husky/Graphics/SamplerMinMagFilter.h>
#include <Husky/Graphics/SamplerMipFilter.h>
#include <Husky/Graphics/SamplerBorderColor.h>

namespace Husky::Graphics
{
    struct SamplerCreateInfo
    {
        SamplerAddressMode uAddressMode = SamplerAddressMode::ClampToEdge;
        SamplerAddressMode vAddressMode = SamplerAddressMode::ClampToEdge;
        SamplerAddressMode wAddressMode = SamplerAddressMode::ClampToEdge;
        SamplerMinMagFilter minFilter = SamplerMinMagFilter::Nearest;
        SamplerMinMagFilter magFilter = SamplerMinMagFilter::Nearest;
        SamplerMipFilter mipFilter = SamplerMipFilter::NotMipmapped;
        CompareFunction compareFunction = CompareFunction::Never;
        SamplerBorderColor borderColor = SamplerBorderColor::OpaqueBlack;
        int32 maxAnisotropy = 1;
        float32 minLod = 0.0;
        float32 maxLod = Limits<float32>::max();
        bool normalizedCoordinates = true;
    };
}
