#pragma once

#include <Husky/Types.h>

namespace Husky::Graphics
{
    template<typename T>
    struct Point2D
    {
        T x = 0;
        T y = 0;
    };

    using IntPoint2D = Point2D<int32>;
    using FloatPoint2D = Point2D<float32>;
}

