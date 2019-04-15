#pragma once

#include <Luch/Point3.h>
#include <Luch/Size3.h>

namespace Luch
{
    template<typename T>
    struct Rect3
    {
        Point3<T> origin;
        Size3<T> size;
    };

    using Rect3i = Rect3<int32>;
    using Rect3f = Rect3<float32>;
}
