#pragma once

#include <Luch/Types.h>
#include <Luch/Graphics/CompareFunction.h>
#include <Luch/Graphics/SamplerAddressMode.h>
#include <Luch/Graphics/SamplerMinMagFilter.h>
#include <Luch/Graphics/SamplerMipFilter.h>
#include <Luch/Graphics/SamplerBorderColor.h>

namespace Luch::Graphics
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
