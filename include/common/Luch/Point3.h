#pragma once

#include <Luch/Types.h>

namespace Luch
{
    template<typename T>
    struct Point3
    {
        T x = 0;
        T y = 0;
        T z = 0;
    };

    using Point3i = Point3<int32>;
    using Point3f = Point3<float32>;
}

