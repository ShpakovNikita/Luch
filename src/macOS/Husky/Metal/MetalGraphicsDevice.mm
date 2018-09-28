#include <Husky/Metal/MetalGraphicsDevice.h>
#include <Husky/Graphics/BufferCreateInfo.h>
#include <Husky/Metal/MetalCommandQueue.h>
#include <Husky/Metal/MetalDescriptorPool.h>
#include <Husky/Metal/MetalPipelineState.h>
#include <Husky/Metal/MetalDescriptorSetLayout.h>
#include <Husky/Metal/MetalPipelineLayout.h>
#include <Husky/Metal/MetalShaderLibrary.h>
#include <Husky/Metal/MetalSemaphore.h>
#include <Husky/Metal/MetalRenderPass.h>
#include <Husky/Metal/MetalTexture.h>
#include <Husky/Metal/MetalBuffer.h>
#include <Husky/Metal/MetalSampler.h>
#include <Husky/Metal/MetalSwapchain.h>
#include <Husky/Metal/MetalSurface.h>
#include <Husky/Metal/MetalError.h>
#include <Husky/Metal/MetalPipelineStateCreateInfo.h>
#include <Husky/Metal/MetalTextureCreateInfo.h>
#include <Husky/Metal/MetalSamplerCreateInfo.h>
#include <Husky/Assert.h>
#import <QuartzCore/CAMetalLAyer.h>

namespace Husky::Metal
{
    MetalGraphicsDevice::MetalGraphicsDevice(
        PhysicalDevice* aPhysicalDevice,
        mtlpp::Device aDevice)
        : physicalDevice(aPhysicalDevice)
        , device(aDevice)
    {
    }

    int32 MetalGraphicsDevice::MaxBoundBuffers() const
    {
        return 31;
    }

    int32 MetalGraphicsDevice::MaxBoundTextures() const
    {
        return 31;
    }

    int32 MetalGraphicsDevice::MaxBoundSamplers() const
    {
        return 16;
    }

    GraphicsResultRefPtr<CommandQueue> MetalGraphicsDevice::CreateCommandQueue()
    {
        auto metalQueue = device.NewCommandQueue();
        return { GraphicsResult::Success, MakeRef<MetalCommandQueue>(this, metalQueue) };
    }

    GraphicsResultRefPtr<DescriptorPool> MetalGraphicsDevice::CreateDescriptorPool(
        const DescriptorPoolCreateInfo& createInfo)
    {
        return { GraphicsResult::Success, MakeRef<MetalDescriptorPool>(this, createInfo) };
    }

    GraphicsResultRefPtr<DescriptorSetLayout> MetalGraphicsDevice::CreateDescriptorSetLayout(
        const DescriptorSetLayoutCreateInfo& createInfo)
    {
        return { GraphicsResult::Success, MakeRef<MetalDescriptorSetLayout>(this, createInfo) };
    }

    GraphicsResultRefPtr<RenderPass> MetalGraphicsDevice::CreateRenderPass(
        const RenderPassCreateInfo& createInfo)
    {
        return { GraphicsResult::Success, MakeRef<MetalRenderPass>(this, createInfo) };
    }

    GraphicsResultRefPtr<PipelineLayout> MetalGraphicsDevice::CreatePipelineLayout(
        const PipelineLayoutCreateInfo &createInfo)
    {
        return { GraphicsResult::Success, MakeRef<MetalPipelineLayout>(this, createInfo) };
    }

    GraphicsResultRefPtr<PipelineState> MetalGraphicsDevice::CreatePipelineState(
        const PipelineStateCreateInfo& createInfo)
    {
        auto mtlPipelineDescriptor = ToMetalPipelineStateCreateInfo(createInfo);
        auto mtlDepthStencilDescriptor = ToMetalDepthStencilDescriptor(createInfo);
        ns::Error error;

        auto mtlPipelineState = device.NewRenderPipelineState(mtlPipelineDescriptor, &error);
        auto mtlDepthStencilState = device.NewDepthStencilState(mtlDepthStencilDescriptor);

        auto result = PipelineErrorToGraphicsResult(error);

        if(result == GraphicsResult::Success)
        {
            return { result, MakeRef<MetalPipelineState>(this, createInfo, mtlPipelineState, mtlDepthStencilState) };
        }
        else
        {
            return { result };
        }
    }

    GraphicsResultRefPtr<Texture> MetalGraphicsDevice::CreateTexture(
        const TextureCreateInfo& createInfo)
    {
        auto d = ToMetalTextureDescriptor(createInfo);
        auto mtlTexture = device.NewTexture(d);
        return { GraphicsResult::Success, MakeRef<MetalTexture>(this, createInfo, mtlTexture) };
    }

    GraphicsResultRefPtr<Buffer> MetalGraphicsDevice::CreateBuffer(
        const BufferCreateInfo& createInfo,
        const void* initialData)
    {
        uint32 optionBits = (uint32)mtlpp::ResourceOptions::CpuCacheModeDefaultCache;
        switch(createInfo.storageMode)
        {
        case ResourceStorageMode::DeviceLocal:
            if(initialData != nullptr)
            {
                HUSKY_ASSERT_MSG(false, "Can't create GPU-only buffer with initial data");
                return { GraphicsResult::InvalidValue };
            }
            optionBits |= (uint32)mtlpp::ResourceOptions::StorageModePrivate;
            break;
        case ResourceStorageMode::Shared:
            optionBits |= (uint32)mtlpp::ResourceOptions::StorageModeShared;
            break;
        }

        mtlpp::ResourceOptions options = static_cast<mtlpp::ResourceOptions>(optionBits);

        mtlpp::Buffer mtlBuffer;
        if(initialData != nullptr)
        {
            mtlBuffer = device.NewBuffer(initialData, createInfo.length, options);
        }
        else
        {
            mtlBuffer = device.NewBuffer(createInfo.length, options);
        }

        return { GraphicsResult::Success, MakeRef<MetalBuffer>(this, createInfo, mtlBuffer) };
    }

    GraphicsResultRefPtr<Sampler> MetalGraphicsDevice::CreateSampler(
        const SamplerCreateInfo& createInfo)
    {
        auto d = ToMetalSamplerDescriptor(createInfo);
        auto mtlSampler = device.NewSamplerState(d);
        return { GraphicsResult::Success, MakeRef<MetalSampler>(this, createInfo, mtlSampler) };
    }

    GraphicsResultRefPtr<Swapchain> MetalGraphicsDevice::CreateSwapchain(
        const SwapchainCreateInfo& createInfo,
        Surface* surface)
    {
        auto mtlSurface = static_cast<MetalSurface*>(surface);
        return { GraphicsResult::Success, MakeRef<MetalSwapchain>(this, createInfo, (CAMetalLayer*)mtlSurface->layer) };
    }

    GraphicsResultRefPtr<ShaderLibrary> MetalGraphicsDevice::CreateShaderLibraryFromSource(
        const Vector<Byte>& source,
        const UnorderedMap<String, Variant<int32, String>>& defines)
    {
        mtlpp::CompileOptions options;
        // TODO Defines
        ns::Error error;
        auto mtlLibrary = device.NewLibrary((const char*)source.data(), options, &error);

        GraphicsResult result = LibraryErrorToGraphicsResult(error);
        if(mtlLibrary)
        {
            return { result, MakeRef<MetalShaderLibrary>(this, mtlLibrary) };
        }
        else
        {
            return { result };
        }
    }

    GraphicsResultRefPtr<Semaphore> MetalGraphicsDevice::CreateSemaphore()
    {
        return { GraphicsResult::Success, MakeRef<MetalSemaphore>(this) };
    }
}
