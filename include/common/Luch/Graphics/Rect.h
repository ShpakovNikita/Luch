#pragma once

#include <Luch/Graphics/Point2D.h>
#include <Luch/Graphics/Size2D.h>

namespace Luch::Graphics
{
    template<typename T>
    struct Rect
    {
        Point2D<T> origin;
        Size2D<T> size;
    };

    using IntRect = Rect<int32>;
    using FloatRect = Rect<float32>;
}
