#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/ResultValue.h>

namespace Luch::Render
{
    using namespace Graphics;

    struct SharedBufferSuballocation
    {
        RefPtr<Buffer> buffer;
        void* offsetMemory = nullptr;
        int32 offset = 0;
        int32 size = 0;
    };

    class SharedBuffer
    {
    public:
        SharedBuffer(RefPtr<Buffer> buffer);
        ~SharedBuffer();

        SharedBufferSuballocation Suballocate(int32 size, int32 alignment);
        void Reset();
    private:
        RefPtr<Buffer> buffer;
        int32 remainingSize = 0;
        int32 offset = 0;
    };
}

