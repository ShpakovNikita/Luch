#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/GraphicsResultValue.h>

namespace Luch::Graphics
{
    class GraphicsDevice : public BaseObject
    {
    public:
        virtual ~GraphicsDevice() = 0;

        virtual PhysicalDevice* GetPhysicalDevice() = 0;

        virtual GraphicsResultRefPtr<CommandQueue> CreateCommandQueue() = 0;

        virtual GraphicsResultRefPtr<DescriptorPool> CreateDescriptorPool(
            const DescriptorPoolCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<DescriptorSetLayout> CreateDescriptorSetLayout(
            const DescriptorSetLayoutCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<PipelineLayout> CreatePipelineLayout(
            const PipelineLayoutCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<RenderPass> CreateRenderPass(
            const RenderPassCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<FrameBuffer> CreateFrameBuffer(
            const FrameBufferCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<PipelineState> CreatePipelineState(
            const PipelineStateCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<Texture> CreateTexture(
            const TextureCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<Buffer> CreateBuffer(
            const BufferCreateInfo& createInfo,
            const void* initialData = nullptr) = 0;

        virtual GraphicsResultRefPtr<Sampler> CreateSampler(
            const SamplerCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<ShaderLibrary> CreateShaderLibraryFromSource(
            const Vector<Byte>& source,
            const UnorderedMap<String, Variant<int32, String>>& defines) = 0;

        virtual GraphicsResultRefPtr<Swapchain> CreateSwapchain(
            const SwapchainInfo& createInfo,
            Surface* surface) = 0;

        virtual GraphicsResultRefPtr<Semaphore> CreateSemaphore() = 0;
    };

    inline GraphicsDevice::~GraphicsDevice() {}
}
