#pragma once

namespace Husky::Graphics
{
    enum class StencilOperation
    {
        Keep,
        Zero,
        Replace,
        IncrementAndClamp,
        DecrementAndClamp,
        IncrementAndWrap,
        DecrementAndWrap,
        Invert,
    };
}
