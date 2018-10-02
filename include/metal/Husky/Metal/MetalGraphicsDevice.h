#pragma once

#include <Husky/Graphics/GraphicsDevice.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalGraphicsDevice : public GraphicsDevice
    {
        friend class MetalSwapchain;
    public:
        MetalGraphicsDevice(
            PhysicalDevice* physicalDevice,
            mtlpp::Device device);

        PhysicalDevice* GetPhysicalDevice() override { return physicalDevice; }

        int32 MaxBoundBuffers() const;
        int32 MaxBoundTextures() const;
        int32 MaxBoundSamplers() const;

        GraphicsResultRefPtr<CommandQueue> CreateCommandQueue() override;

        GraphicsResultRefPtr<DescriptorPool> CreateDescriptorPool(
            const DescriptorPoolCreateInfo& createInfo) override;

        GraphicsResultRefPtr<DescriptorSetLayout> CreateDescriptorSetLayout(
            const DescriptorSetLayoutCreateInfo& createInfo) override;

        GraphicsResultRefPtr<PipelineLayout> CreatePipelineLayout(
            const PipelineLayoutCreateInfo& createInfo) override;

        GraphicsResultRefPtr<RenderPass> CreateRenderPass(
            const RenderPassCreateInfo& createInfo) override;

        GraphicsResultRefPtr<PipelineState> CreatePipelineState(
            const PipelineStateCreateInfo& createInfo) override;

        GraphicsResultRefPtr<Texture> CreateTexture(
            const TextureCreateInfo& createInfo) override;

        GraphicsResultRefPtr<Buffer> CreateBuffer(
            const BufferCreateInfo& createInfo,
            const void* initialData = nullptr) override;

        GraphicsResultRefPtr<Sampler> CreateSampler(
            const SamplerCreateInfo& createInfo) override;

        GraphicsResultRefPtr<Swapchain> CreateSwapchain(
            const SwapchainInfo& createInfo,
            Surface* surface) override;

        GraphicsResultRefPtr<ShaderLibrary> CreateShaderLibraryFromSource(
            const Vector<Byte>& source,
            const UnorderedMap<String, Variant<int32, String>>& defines) override;

        GraphicsResultRefPtr<Semaphore> CreateSemaphore() override;
    private:
        PhysicalDevice* physicalDevice = nullptr;
        mtlpp::Device device;
    };
}
