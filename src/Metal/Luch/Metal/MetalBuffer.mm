#include <Luch/Metal/MetalBuffer.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
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

    GraphicsResultValue<void*> MetalBuffer::MapMemory(int32 size, int32 offset)
    {
        mappedMemory = static_cast<Byte*>(buffer.GetContents()) + offset;
        return { GraphicsResult::Success, mappedMemory };
    }

    void* MetalBuffer::GetMappedMemory()
    {
        HUSKY_ASSERT(mappedMemory != nullptr);
        return mappedMemory;
    }

    GraphicsResult MetalBuffer::UnmapMemory()
    {
        mappedMemory = nullptr;
        return GraphicsResult::Success;
    }
}
