#include <Luch/Metal/MetalTextureCreateInfo.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Metal/MetalFormat.h>
#include <Luch/Types.h>
#include <Luch/Assert.h>

namespace Luch::Metal
{
    using namespace Graphics;

    mtlpp::TextureType ToMetalTextureType(TextureType textureType)
    {
        switch(textureType)
        {
        case TextureType::Texture1D:
            return mtlpp::TextureType::Texture1D;
        case TextureType::Texture1DArray:
            return mtlpp::TextureType::Texture1DArray;
        case TextureType::Texture2D:
            return mtlpp::TextureType::Texture2D;
        case TextureType::Texture2DArray:
            return mtlpp::TextureType::Texture2DArray;
        case TextureType::Texture2DMultisample:
            return mtlpp::TextureType::Texture2DMultisample;
        case TextureType::Texture2DMultisampleArray:
            return mtlpp::TextureType::Texture2DMultisampleArray;
        case TextureType::TextureCube:
            return mtlpp::TextureType::TextureCube;
        case TextureType::TextureCubeArray:
            return mtlpp::TextureType::TextureCubeArray;
        case TextureType::Texture3D:
            return mtlpp::TextureType::Texture3D;
        case TextureType::TextureBuffer:
            return mtlpp::TextureType::TextureBuffer;
        default:
            LUCH_ASSERT_MSG(false, "Unknown texture type");
        }
    }

    mtlpp::TextureUsage ToMetalTextureUsage(TextureUsageFlags flags)
    {
        if(flags == TextureUsageFlags::Unknown)
        {
            return mtlpp::TextureUsage::Unknown;
        }

        uint32 bits = 0;

        if((flags & TextureUsageFlags::ColorAttachment) == TextureUsageFlags::ColorAttachment)
        {
            bits |= (uint32)mtlpp::TextureUsage::RenderTarget;
        }

        if((flags & TextureUsageFlags::DepthStencilAttachment) == TextureUsageFlags::DepthStencilAttachment)
        {
            bits |= (uint32)mtlpp::TextureUsage::RenderTarget;
        }

        if((flags & TextureUsageFlags::ShaderRead) == TextureUsageFlags::ShaderRead)
        {
            bits |= (uint32)mtlpp::TextureUsage::ShaderRead;
        }

        if((flags & TextureUsageFlags::ShaderWrite) == TextureUsageFlags::ShaderWrite)
        {
            bits |= (uint32)mtlpp::TextureUsage::ShaderWrite;
        }

        // Metal doesn't cae for TextureUsageFlags::TransferSource and TextureUsageFlags::TransferDestination 

        return static_cast<mtlpp::TextureUsage>(bits);
    }

    mtlpp::TextureDescriptor ToMetalTextureDescriptor(const TextureCreateInfo& createInfo)
    {
        mtlpp::TextureDescriptor d;

        d.SetTextureType(ToMetalTextureType(createInfo.textureType));
        d.SetWidth((uint32)createInfo.width);
        d.SetHeight((uint32)createInfo.height);
        d.SetDepth((uint32)createInfo.depth);
        //d.SetSampleCount((uint32)createInfo.sampleCount);
        d.SetArrayLength((uint32)createInfo.arrayLength);
        d.SetMipmapLevelCount((uint32)createInfo.mipmapLevelCount);
        d.SetUsage(ToMetalTextureUsage(createInfo.usage));
        d.SetPixelFormat(ToMetalPixelFormat(createInfo.format));

        switch(createInfo.storageMode)
        {
        case ResourceStorageMode::DeviceLocal:
            d.SetStorageMode(mtlpp::StorageMode::Private);
            break;
        case ResourceStorageMode::Shared:
            d.SetStorageMode(mtlpp::StorageMode::Shared);
            break;
        }

        return d;
    }
}

