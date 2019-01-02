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
        R8G8Unorm,
        R8G8Unorm_sRGB,

        // Color (RGB)
        R8G8B8Unorm,
        R8G8B8Unorm_sRGB,

        // Color RGBA
        R8G8B8A8Unorm,
        R8G8B8A8Unorm_sRGB,
        B8G8R8A8Unorm,
        B8G8R8A8Unorm_sRGB,

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

        R8G8Sint,
        R8G8Uint,
        R16G16Sint,
        R16G16Uint,
        R32G32Uint,
        R32G32Sfloat,

        R8G8B8Sint,
        R8G8B8Uint,
        R16G16B16Sint,
        R16G16B16Uint,
        R32G32B32Uint,
        R32G32B32Sfloat,

        R8G8B8A8Sint,
        R8G8B8A8Uint,
        R16G16B16A16Sint,
        R16G16B16A16Uint,
        R32G32B32A32Uint,
        R32G32B32A32Sfloat
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
