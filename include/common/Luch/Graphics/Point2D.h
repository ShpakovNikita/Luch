#pragma once

#include <Luch/Types.h>

namespace Luch::Graphics
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

