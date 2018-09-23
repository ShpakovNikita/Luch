#include <Husky/Metal/MetalBuffer.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
{
    using namespace Graphics;

    MetalBuffer::MetalBuffer(
        MetalGraphicsDevice* device,
        const BufferCreateInfo& aCreateInfo,
        mtlpp::Buffer aBuffer)
        : Buffer(device)
        , createInfo(aCreateInfo)
        , buffer(aBuffer)
    {
    }
}
