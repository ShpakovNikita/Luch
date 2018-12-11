#pragma once

#include <Luch/Graphics/FrameBuffer.h>
#include <Luch/Graphics/FrameBufferCreateInfo.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;
    class MetalFrameBuffer : public FrameBuffer
    {
    public:
        MetalFrameBuffer(
            MetalGraphicsDevice* device,
            mtlpp::RenderPassDescriptor descriptor,
            const FrameBufferCreateInfo& createInfo);

        const FrameBufferCreateInfo& GetCreateInfo() const override { return createInfo; }
        mtlpp::RenderPassDescriptor GetRenderPassDescriptor() { return descriptor; }
    private:
        FrameBufferCreateInfo createInfo;
        mtlpp::RenderPassDescriptor descriptor;
    };
}
