#pragma once

#include <Luch/Types.h>

namespace Luch::Graphics
{
    template<typename T>
    struct Point2
    {
        T x = 0;
        T y = 0;
    };

    using Point2i = Point2<int32>;
    using Point2f = Point2<float32>;
}

