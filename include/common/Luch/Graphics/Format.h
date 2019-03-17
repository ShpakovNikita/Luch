#pragma once

namespace Luch::Graphics
{
    enum class Format
    {
        Undefined,

        // Color (R)
        R8Unorm,
        R8Unorm_sRGB,

        // Color (RG)
        RG8Unorm,
        RG8Unorm_sRGB,

        // Color (RGB)
        RGB8Unorm,
        RGB8Unorm_sRGB,

        // Color RGBA
        RGBA8Unorm,
        RGBA8Unorm_sRGB,
        BGRA8Unorm,
        BGRA8Unorm_sRGB,

        // Depth
        D16Unorm,
        D32Sfloat,

        // Stencil
        S8Uint,

        // Depth/Stencil
        D16UnormS8Uint,
        D24UnormS8Uint,
        D32SfloatS8Uint,

        // Other
        R8Sint,
        R8Uint,
        R16Sint,
        R16Uint,
        R32Uint,
        R32Sfloat,

        RG8Sint,
        RG8Uint,
        RG16Sint,
        RG16Uint,
        RG32Uint,
        RG32Sfloat,

        RGB8Sint,
        RGB8Uint,
        RGB16Sint,
        RGB16Uint,
        RGB32Uint,
        RGB32Sfloat,

        RGBA8Sint,
        RGBA8Uint,
        RGBA16Sint,
        RGBA16Uint,
        RGBA32Uint,

        RGBA16Sfloat,
        RGBA32Sfloat,
    };

    inline bool FormatIsDepthOnlyFormat(Format format)
    {
        switch(format)
        {
        case Format::D16Unorm:
        case Format::D32Sfloat:
            return true;
        default:
            return false;
        }
    }

    inline bool FormatIsStencilOnlyFormat(Format format)
    {
        switch(format)
        {
        case Format::S8Uint:
            return true;
        default:
            return false;
        }
    }

    inline bool FormatIsDepthStencilFormat(Format format)
    {
        switch(format)
        {
        case Format::D16UnormS8Uint:
        case Format::D24UnormS8Uint:
        case Format::D32SfloatS8Uint:
            return true;
        default:
            return false;
        }
    }

    inline bool FormatHasDepth(Format format)
    {
        return FormatIsDepthOnlyFormat(format) || FormatIsDepthStencilFormat(format);
    }

    inline bool FormatHasStencil(Format format)
    {
        return FormatIsStencilOnlyFormat(format) || FormatIsDepthStencilFormat(format);
    }
}
