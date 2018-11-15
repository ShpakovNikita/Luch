#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Graphics/IndexType.h>
#include <Luch/SceneV1/Buffer.h>

namespace Luch::SceneV1
{
    struct IndexBuffer
    {
        RefPtr<Buffer> backingBuffer;

        Graphics::IndexType indexType = Graphics::IndexType::UInt16;
        int32 count = 0;

        int32 byteOffset = 0;
        int32 byteLength = 0;
    };
}
