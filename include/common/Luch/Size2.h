#pragma once

#include <Luch/Types.h>

namespace Luch
{
    template<typename T>
    struct Size2
    {
        T width = 0;
        T height = 0;
    };

    using Size2i = Size2<int32>;
    using Size2f = Size2<float32>;
}
