#pragma once

#include <Luch/RefPtr.h>
#include <Luch/SceneV1/Buffer.h>

namespace Luch::SceneV1
{
    struct VertexBuffer
    {
        RefPtr<Buffer> backingBuffer;
        int32 stride = 4;
        int32 byteOffset = 0;
        int32 byteLength = 0;
    };
}
