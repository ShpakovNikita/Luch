#pragma once

#include <Husky/Assert.h>
#include <Husky/Types.h>

namespace Husky
{
    enum class IndexType
    {
        UInt16,
        UInt32,
    };

    inline int32 IndexSize(IndexType indexType)
    {
        switch (indexType)
        {
        case IndexType::UInt16:
            return sizeof(uint16);
        case IndexType::UInt32:
            return sizeof(uint32);
        default:
            HUSKY_ASSERT(false, "Unknown index type");
        }
    }
}
