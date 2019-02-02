#pragma once

#include <Luch/Types.h>
#include <Luch/Graphics/Format.h>

namespace Luch::Render::Passes::TiledDeferred::TiledDeferredConstants
{
    using namespace Graphics;

    constexpr int32 GBufferColorAttachmentBegin = 0;
    constexpr int32 GBufferColorAttachmentCount = 4;
    constexpr int32 GBufferColorAttachmentEnd = GBufferColorAttachmentBegin + GBufferColorAttachmentCount;
    constexpr int32 ColorAttachmentCount = 5;
    constexpr int32 LuminanceAttachmentIndex = 4;
    constexpr Array<Format, ColorAttachmentCount> ColorAttachmentFormats =
    {
        Format::RGBA16Sfloat, // gbuffer 0
        Format::RGBA16Sfloat, // gbuffer 1
        Format::RGBA16Sfloat, // gbuffer 2
        Format::R32Sfloat,    // gbuffer depth (clip space)
        Format::RGBA16Sfloat, // luminance
    };
}