#pragma once

#include <Husky/RefPtr.h>
#include <Husky/Graphics/IndexType.h>
#include <Husky/SceneV1/Buffer.h>

namespace Husky::SceneV1
{
    struct IndexBuffer
    {
        RefPtr<Buffer> backingBuffer;

        IndexType indexType = IndexType::UInt16;
        int32 count = 0;

        int32 byteOffset = 0;
        int32 byteLength = 0;
    };
}
