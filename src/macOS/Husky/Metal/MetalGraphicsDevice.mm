#include <Husky/Metal/MetalGraphicsDevice.h>
#include <Husky/Metal/MetalCommandQueue.h>
#include <Husky/Metal/MetalDescriptorPool.h>
#include <Husky/Metal/MetalDescriptorSetLayout.h>
#include <Husky/Metal/MetalPipelineLayout.h>
#include <Husky/Metal/MetalShaderLibrary.h>
#include <Husky/Metal/MetalRenderPass.h>
#include <Husky/Metal/MetalError.h>
#include <Husky/Metal/MetalPipelineStateCreateInfo.h>
#include <Husky/Assert.h>

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
        auto mtlPipelineDescriptor = ToMetalPiplineStateCreateInfo(createInfo);
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

    GraphicsResultRefPtr<ShaderLibrary> MetalGraphicsDevice::CreateShaderLibraryFromSource(
        const Vector<Byte>& source,
        const UnorderedMap<String, Variant<int32, String>>& defines)
    {
        mtlpp::CompileOptions options;
        // TODO Defines
        ns::Error error;
        auto mtlLibrary = device.NewLibrary((const char*)source.data(), options, &error);

        GraphicsResult result = ToGraphicsResult(error);
        if(mtlLibrary)
        {
            return { result, MakeRef<MetalShaderLibrary>(this, mtlLibrary) };
        }
        else
        {
            return { result };
        }
    }
}
