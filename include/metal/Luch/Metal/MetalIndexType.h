#pragma once

#include <Luch/Graphics/IndexType.h>
#include <Luch/Assert.h>
#include <mtlpp.hpp>

namespace Luch::Metal
{
    using namespace Graphics;

    mtlpp::IndexType ToMetalIndexType(IndexType type)
    {
        switch(type)
        {
        case IndexType::UInt16:
            return mtlpp::IndexType::UInt16;
        case IndexType::UInt32:
            return mtlpp::IndexType::UInt32;
        default:
            HUSKY_ASSERT_MSG(false, "Unknown unsupported index type");
            return mtlpp::IndexType::UInt16;
        }
    }
}
