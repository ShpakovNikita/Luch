#include <Luch/Render/SharedBuffer.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/BufferCreateInfo.h>

namespace Luch::Render
{
    SharedBuffer::SharedBuffer(RefPtr<Buffer> aBuffer)
        : buffer(aBuffer)
    {
        Reset();
        [[maybe_unused]] auto [mapResult, _] = buffer->MapMemory(buffer->GetCreateInfo().length, 0);
        LUCH_ASSERT(mapResult == GraphicsResult::Success);
    }

    SharedBuffer::~SharedBuffer()
    {
        buffer->UnmapMemory();
    }

    bool SharedBuffer::CanSuballocate(int32 size, int32 alignment)
    {
        int32 padding = (-offset & (alignment - 1));
        int32 alignedOffset = padding + offset;

        return (alignedOffset + size) < buffer->GetCreateInfo().length;
    }

    SharedBufferSuballocation SharedBuffer::Suballocate(int32 size, int32 alignment)
    {
        LUCH_ASSERT(CanSuballocate(size, alignment));

        int32 padding = (-offset & (alignment - 1));
        int32 alignedOffset = padding + offset;

        SharedBufferSuballocation suballocation;
        suballocation.buffer = buffer;
        suballocation.offsetMemory = (Byte*)buffer->GetMappedMemory() + alignedOffset;
        suballocation.offset = alignedOffset;
        suballocation.size = padding + size;
        offset += padding + size;
        return suballocation;
    }

    void SharedBuffer::Reset()
    {
        const auto& createInfo = buffer->GetCreateInfo();
        offset = 0;
    }
}


