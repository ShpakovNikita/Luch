#pragma once

#include <Husky/Types.h>

namespace Husky::Graphics
{
    template<typename T>
    struct Size2D
    {
        T width = 0;
        T height = 0;
    };

    using IntSize2D = Size2D<int32>;
    using FloatSize2D = Size2D<float32>;
}
