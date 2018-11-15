#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/SamplerAddressMode.h>
#include <Luch/Graphics/SamplerMinMagFilter.h>
#include <Luch/Graphics/SamplerMipFilter.h>
#include <Luch/Graphics/SamplerBorderColor.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    mtlpp::SamplerAddressMode ToMetalSamplerAddressMode(SamplerAddressMode addressMode);
    mtlpp::SamplerMinMagFilter ToMetalSamplerMinMagFilter(SamplerMinMagFilter filter);
    mtlpp::SamplerMipFilter ToMetalSamplerMipFilter(SamplerMipFilter mipFilter);
    mtlpp::SamplerBorderColor ToMetalSamplerBorderColor(SamplerBorderColor borderColor);
    mtlpp::SamplerDescriptor ToMetalSamplerDescriptor(const SamplerCreateInfo& createInfo);
}

