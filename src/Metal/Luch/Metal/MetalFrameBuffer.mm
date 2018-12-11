#include <Luch/Metal/MetalFrameBuffer.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
{
    using namespace Graphics;

    MetalFrameBuffer::MetalFrameBuffer(
        MetalGraphicsDevice* device,
        mtlpp::RenderPassDescriptor aDescriptor,
        const FrameBufferCreateInfo& aCreateInfo)
        : FrameBuffer(device)
        , createInfo(aCreateInfo)
        , descriptor(aDescriptor)
    {
    }
}
