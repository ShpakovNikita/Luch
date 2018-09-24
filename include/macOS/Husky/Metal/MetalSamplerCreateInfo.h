#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/SamplerAddressMode.h>
#include <Husky/Graphics/SamplerMinMagFilter.h>
#include <Husky/Graphics/SamplerMipFilter.h>
#include <Husky/Graphics/SamplerBorderColor.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
{
    using namespace Graphics;

    mtlpp::SamplerAddressMode ToMetalSamplerAddressMode(SamplerAddressMode addressMode);
    mtlpp::SamplerMinMagFilter ToMetalSamplerMinMagFilter(SamplerMinMagFilter filter);
    mtlpp::SamplerMipFilter ToMetalSamplerMipFilter(SamplerMipFilter mipFilter);
    mtlpp::SamplerBorderColor ToMetalSamplerBorderColor(SamplerBorderColor borderColor);
    mtlpp::SamplerDescriptor ToMetalSamplerDescriptor(const SamplerCreateInfo& createInfo);
}

