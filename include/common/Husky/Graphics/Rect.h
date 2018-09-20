#pragma once

#include <Husky/Graphics/Point2D.h>
#include <Husky/Graphics/Size2D.h>

namespace Husky::Graphics
{
    template<typename T>
    struct Rect
    {
        Point2D origin;
        Size2D size;
    }

    using IntRect = Rect<int32>;
    using FloatRect = Rect<float32>;
}
