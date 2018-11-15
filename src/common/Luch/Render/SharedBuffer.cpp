#include <Luch/Render/SharedBuffer.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/BufferCreateInfo.h>

namespace Luch::Render
{
    SharedBuffer::SharedBuffer(RefPtr<Buffer> aBuffer)
        : buffer(aBuffer)
    {
        Reset();
        [[maybe_unused]] auto [mapResult, _] = buffer->MapMemory(remainingSize, 0);
        LUCH_ASSERT(mapResult == GraphicsResult::Success);
    }

    SharedBuffer::~SharedBuffer()
    {
        buffer->UnmapMemory();
    }

    SharedBufferSuballocation SharedBuffer::Suballocate(int32 size, int32 alignment)
    {
        LUCH_ASSERT(size <= remainingSize);

        int32 padding = (-offset & (alignment - 1));
        int32 alignedOffset = padding + offset;

        SharedBufferSuballocation suballocation;
        suballocation.buffer = buffer;
        suballocation.offsetMemory = (Byte*)buffer->GetMappedMemory() + alignedOffset;
        suballocation.offset = alignedOffset;
        suballocation.size = padding + size;
        offset += padding + size;
        remainingSize -= padding + size;
        return suballocation;
    }

    void SharedBuffer::Reset()
    {
        const auto& createInfo = buffer->GetCreateInfo();
        remainingSize = createInfo.length;
        offset = 0;
    }
}


