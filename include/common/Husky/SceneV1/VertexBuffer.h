#pragma once

#include <Husky/RefPtr.h>
#include <Husky/SceneV1/Buffer.h>

namespace Husky::SceneV1
{
    struct VertexBuffer
    {
        RefPtr<Buffer> backingBuffer;
        int32 stride = 4;
        int32 byteOffset = 0;
        int32 byteLength = 0;
    };
}
