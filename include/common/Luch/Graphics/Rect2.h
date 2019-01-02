#pragma once

#include <Luch/Graphics/Point2.h>
#include <Luch/Graphics/Size2.h>

namespace Luch::Graphics
{
    template<typename T>
    struct Rect2
    {
        Point2<T> origin;
        Size2<T> size;
    };

    using Rect2i = Rect2<int32>;
    using Rect2f = Rect2<float32>;
}
