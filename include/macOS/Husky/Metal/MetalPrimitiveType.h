#pragma once

#include <Husky/Graphics/PrimitiveTopology.h>
#include <Husky/Assert.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    using namespace Graphics;

    inline mtlpp::PrimitiveType ToMetalPrimitiveType(PrimitiveTopology primitiveTopology)
    {
        switch(primitiveTopology)
        {
        case PrimitiveTopology::PointList:
            return mtlpp::PrimitiveType::Point;
        case PrimitiveTopology::LineList:
            return mtlpp::PrimitiveType::Line;
        case PrimitiveTopology::LineStrip:
            return mtlpp::PrimitiveType::LineStrip;
        case PrimitiveTopology::TriangleList:
            return mtlpp::PrimitiveType::Triangle;
        case PrimitiveTopology::TriangleStrip:
            return mtlpp::PrimitiveType::TriangleStrip;
        case PrimitiveTopology::TriangleFan:
            HUSKY_ASSERT_MSG(false, "Triangle fan primitive topology is unsupported in Metal");
            return mtlpp::PrimitiveType::Point;
        default:
            HUSKY_ASSERT_MSG(false, "Unknown unsupported primitive topology");
            return mtlpp::PrimitiveType::Point;
        }
    }
}
