#pragma once

#include <Husky/Types.h>

namespace Husky::Graphics
{
    template<typename T>
    struct Color
    {
        T red = {};
        T green = {};
        T blue = {};
        T alpha = {};
    };

    using ColorUInt8 = Color<uint8>;
    using ColorUInt32 = Color<uint32>;
    using ColorSNorm32 = Color<float32>;
}
