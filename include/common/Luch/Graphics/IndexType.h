#pragma once

#include <Luch/Assert.h>
#include <Luch/Types.h>

namespace Luch::Graphics
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
            HUSKY_ASSERT_MSG(false, "Unknown index type");
        }
    }
}
