#include <Luch/Metal/MetalGraphicsDevice.h>
#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Metal/MetalCommandQueue.h>
#include <Luch/Metal/MetalDescriptorPool.h>
#include <Luch/Metal/MetalPipelineState.h>
#include <Luch/Metal/MetalDescriptorSetLayout.h>
#include <Luch/Metal/MetalPipelineLayout.h>
#include <Luch/Metal/MetalShaderLibrary.h>
#include <Luch/Metal/MetalSemaphore.h>
#include <Luch/Metal/MetalRenderPass.h>
#include <Luch/Metal/MetalTexture.h>
#include <Luch/Metal/MetalBuffer.h>
#include <Luch/Metal/MetalSampler.h>
#include <Luch/Metal/MetalSwapchain.h>
#include <Luch/Metal/MetalSurface.h>
#include <Luch/Metal/MetalError.h>
#include <Luch/Metal/MetalFrameBuffer.h>
#include <Luch/Metal/MetalRenderPassCreateInfo.h>
#include <Luch/Metal/MetalPipelineStateCreateInfo.h>
#include <Luch/Metal/MetalTextureCreateInfo.h>
#include <Luch/Metal/MetalSamplerCreateInfo.h>
#include <Luch/Assert.h>
#import <QuartzCore/CAMetalLAyer.h>
#import <Metal/MTLLibrary.h>

namespace Luch::Metal
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

    GraphicsResultRefPtr<FrameBuffer> MetalGraphicsDevice::CreateFrameBuffer(
        const FrameBufferCreateInfo& createInfo)
    {
        auto renderPassDescriptor = ToMetalRenderPassDescriptor(createInfo);
        return { GraphicsResult::Success, MakeRef<MetalFrameBuffer>(this, renderPassDescriptor, createInfo) };
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

        Optional<mtlpp::DepthStencilState> mtlDepthStencilState;
        if(createInfo.depthStencil.depthTestEnable || createInfo.depthStencil.stencilTestEnable)
        {
            auto mtlDepthStencilDescriptor = ToMetalDepthStencilDescriptor(createInfo);
            mtlDepthStencilState = device.NewDepthStencilState(mtlDepthStencilDescriptor);
        }

        ns::Error error;

        auto mtlPipelineState = device.NewRenderPipelineState(mtlPipelineDescriptor, &error);

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
                LUCH_ASSERT_MSG(false, "Can't create GPU-only buffer with initial data");
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
        const SwapchainInfo& createInfo,
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
        ns::Error error;

        NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];

        for(const auto& define : defines)
        {
            auto defineName = define.first;
            NSString* key = [NSString stringWithCString:defineName.c_str() encoding:NSUTF8StringEncoding];
            id<NSObject> value;
            if(std::holds_alternative<int32>(define.second))
            {
                value = [NSNumber numberWithInt:std::get<int32>(define.second)];
            }
            else if(std::holds_alternative<String>(define.second))
            {
                auto defineString = std::get<String>(define.second);
                value = [NSString stringWithCString:defineString.c_str() encoding:NSUTF8StringEncoding];
            }
            else
            {
                LUCH_ASSERT_MSG(false, "Unknown define type");
                return { GraphicsResult::InvalidValue };
            }
            dict[key] = value;
        }

        MTLCompileOptions* o = (__bridge MTLCompileOptions*)options.GetPtr();
        [o setPreprocessorMacros:dict];

        auto mtlLibrary = device.NewLibrary((const char*)source.data(), options, &error);

        GraphicsResult result = LibraryErrorToGraphicsResult(error);
        if(mtlLibrary)
        {
            return { result, MakeRef<MetalShaderLibrary>(this, mtlLibrary) };
        }
        else
        {
            [[maybe_unused]] auto description = error.GetLocalizedDescription().GetCStr();
            LUCH_ASSERT(false);
            return { result };
        }
    }

    GraphicsResultRefPtr<Semaphore> MetalGraphicsDevice::CreateSemaphore()
    {
        return { GraphicsResult::Success, MakeRef<MetalSemaphore>(this) };
    }
}
