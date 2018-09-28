#pragma once

#include <Husky/Graphics/Buffer.h>
#include <Husky/Graphics/BufferCreateInfo.h>
#include <Husky/Metal/MetalForwards.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalBuffer : public Buffer
    {
        friend class MetalGraphicsCommandList;
        friend class MetalCopyCommandList;
        friend class MetalDescriptorSet;
    public:
        MetalBuffer(
            MetalGraphicsDevice* device,
            const BufferCreateInfo& createInfo,
            mtlpp::Buffer buffer);

        const BufferCreateInfo& GetCreateInfo() const { return createInfo; }

        void* GetMappedMemory() override { return mappedMemory; }
        GraphicsResultValue<void*> MapMemory(int32 size, int32 offset) override;
        GraphicsResult UnmapMemory() override;
    private:
        BufferCreateInfo createInfo;
        mtlpp::Buffer buffer;
        void* mappedMemory = nullptr;
    };
}
