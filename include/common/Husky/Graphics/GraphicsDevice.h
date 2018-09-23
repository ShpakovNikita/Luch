#pragma once

#include <Husky/Types.h>
#include <Husky/RefPtr.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/GraphicsResultValue.h>

namespace Husky::Graphics
{
    class GraphicsDevice : public BaseObject
    {
    public:
        virtual ~GraphicsDevice() = 0;

        virtual PhysicalDevice* GetPhysicalDevice() = 0;

        virtual GraphicsResultRefPtr<CommandQueue> CreateCommandQueue() = 0;

        virtual GraphicsResultRefPtr<CommandPool> CreateCommandPool() = 0;

        virtual GraphicsResultRefPtr<DescriptorPool> CreateDescriptorPool(
            const DescriptorPoolCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<DescriptorSetLayout> CreateDescriptorSetLayout(
            const DescriptorSetLayoutCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<PipelineLayout> CreatePipelineLayout(
            const PipelineLayoutCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<RenderPass> CreateRenderPass(
            const RenderPassCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<PipelineState> CreatePipelineState(
            const PipelineStateCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<Texture> CreateTexture(
            const TextureCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<Buffer> CreateBuffer(
            const BufferCreateInfo& createInfo,
            void* initialData = nullptr) = 0;

        virtual GraphicsResultRefPtr<Sampler> CreateSampler(
            const SamplerCreateInfo& createInfo) = 0;
    };

    inline GraphicsDevice::~GraphicsDevice() {}
}
