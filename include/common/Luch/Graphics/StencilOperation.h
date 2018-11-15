#pragma once

namespace Luch::Graphics
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
