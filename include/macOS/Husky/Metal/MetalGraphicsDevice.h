#pragma once

#include <Husky/Graphics/GraphicsDevice.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalGraphicsDevice : public GraphicsDevice
    {
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
            void* initialData = nullptr) override;

        GraphicsResultRefPtr<Sampler> CreateSampler(
            const SamplerCreateInfo& createInfo) override;
    private:
        PhysicalDevice* physicalDevice = nullptr;
        mtlpp::Device device;
    };
}
