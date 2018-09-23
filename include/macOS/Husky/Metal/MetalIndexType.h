#pragma once

#include <Husky/Graphics/IndexType.h>
#include <Husky/Assert.h>
#include <mtlpp.hpp>

namespace Husky::Metal
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
