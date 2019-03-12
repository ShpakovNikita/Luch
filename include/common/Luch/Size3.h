#pragma once

#include <Luch/Types.h>

namespace Luch
{
    template<typename T>
    struct Size3
    {
        T width = 0;
        T height = 0;
        T depth = 0;
    };

    using Size3i = Size3<int32>;
    using Size3f = Size3<float32>;
}
