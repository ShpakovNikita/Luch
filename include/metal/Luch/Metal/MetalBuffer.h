#pragma once

#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Metal/MetalForwards.h>
#include <mtlpp.hpp>

namespace Luch::Metal
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

        const BufferCreateInfo& GetCreateInfo() const override { return createInfo; }

        void* GetMappedMemory() override;
        GraphicsResultValue<void*> MapMemory(int32 size, int32 offset) override;
        GraphicsResult UnmapMemory() override;
    private:
        BufferCreateInfo createInfo;
        mtlpp::Buffer buffer;
        void* mappedMemory = nullptr;
    };
}
