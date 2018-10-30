#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/Render/Common.h>
#include <Husky/ResultValue.h>

namespace Husky::Render
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

