#include <Luch/Metal/MetalSamplerCreateInfo.h>
#include <Luch/Metal/MetalDepthStencilAttachment.h> // TODO
#include <Luch/Graphics/SamplerCreateInfo.h>
#include <Luch/Types.h>
#include <Luch/Assert.h>

namespace Luch::Metal
{
    using namespace Graphics;

    mtlpp::SamplerAddressMode ToMetalSamplerAddressMode(SamplerAddressMode addressMode)
    {
        switch(addressMode)
        {
        case SamplerAddressMode::ClampToEdge:
            return mtlpp::SamplerAddressMode::ClampToEdge;
        case SamplerAddressMode::MirrorClampToEdge:
            return mtlpp::SamplerAddressMode::MirrorClampToEdge;
        case SamplerAddressMode::Repeat:
            return mtlpp::SamplerAddressMode::Repeat;
        case SamplerAddressMode::MirrorRepeat:
            return mtlpp::SamplerAddressMode::MirrorRepeat;
        case SamplerAddressMode::ClampToBorder:
            return mtlpp::SamplerAddressMode::ClampToBorderColor;
        default:
            LUCH_ASSERT_MSG(false, "Unknown address mode");
            return mtlpp::SamplerAddressMode::ClampToEdge;
        }
    }

    mtlpp::SamplerMinMagFilter ToMetalSamplerMinMagFilter(SamplerMinMagFilter filter)
    {
        switch(filter)
        {
        case SamplerMinMagFilter::Nearest:
            return mtlpp::SamplerMinMagFilter::Nearest;
        case SamplerMinMagFilter::Linear:
            return mtlpp::SamplerMinMagFilter::Linear;
        default:
            LUCH_ASSERT_MSG(false, "Unknown sampler min mag filter");
            return mtlpp::SamplerMinMagFilter::Nearest;
        }
    }

    mtlpp::SamplerMipFilter ToMetalSamplerMipFilter(SamplerMipFilter mipFilter)
    {
        switch(mipFilter)
        {
        case SamplerMipFilter::NotMipmapped:
            return mtlpp::SamplerMipFilter::NotMipmapped;
        case SamplerMipFilter::Nearest:
            return mtlpp::SamplerMipFilter::Nearest;
        case SamplerMipFilter::Linear:
            return mtlpp::SamplerMipFilter::Linear;
        default:
            LUCH_ASSERT_MSG(false, "Unknown sampler mip filter");
            return mtlpp::SamplerMipFilter::NotMipmapped;
        }
    }

    mtlpp::SamplerBorderColor ToMetalSamplerBorderColor(SamplerBorderColor borderColor)
    {
        switch(borderColor)
        {
        case SamplerBorderColor::TransparentBlack:
            return mtlpp::SamplerBorderColor::TransparentBlack;
        case SamplerBorderColor::OpaqueBlack:
            return mtlpp::SamplerBorderColor::OpaqueBlack;
        case SamplerBorderColor::OpaqueWhite:
            return mtlpp::SamplerBorderColor::OpaqueWhite;
        default:
            LUCH_ASSERT_MSG(false, "Unknown sampler border color");
            return mtlpp::SamplerBorderColor::OpaqueBlack;
        }
    }

    mtlpp::SamplerDescriptor ToMetalSamplerDescriptor(const SamplerCreateInfo& createInfo)
    {
        mtlpp::SamplerDescriptor d;

        d.SetRAddressMode(ToMetalSamplerAddressMode(createInfo.wAddressMode));
        d.SetSAddressMode(ToMetalSamplerAddressMode(createInfo.uAddressMode));
        d.SetTAddressMode(ToMetalSamplerAddressMode(createInfo.vAddressMode));
        d.SetMinFilter(ToMetalSamplerMinMagFilter(createInfo.minFilter));
        d.SetMagFilter(ToMetalSamplerMinMagFilter(createInfo.magFilter));
        d.SetMipFilter(ToMetalSamplerMipFilter(createInfo.mipFilter));
        d.SetCompareFunction(ToMetalCompareFunction(createInfo.compareFunction));
        d.SetBorderColor(ToMetalSamplerBorderColor(createInfo.borderColor));
        d.SetMaxAnisotropy(1);
        d.SetLodMinClamp(createInfo.minLod);
        d.SetLodMaxClamp(createInfo.maxLod);
        d.SetNormalizedCoordinates(createInfo.normalizedCoordinates);

        return d;
    }
}


