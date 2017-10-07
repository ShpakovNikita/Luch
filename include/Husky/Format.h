#pragma once

namespace Husky
{
    enum class Format
    {
        Undefined,

        // Color
        R8G8B8A8Unorm,

        // Depth
        D16Unorm,
        D32Sfloat,

        // Stencil
        S8Uint,

        // Depth/Stencil
        D16UnormS8Uint,
        D24UnormS8Uint,
        D32SfloatS8Uint,
    };
}
