#pragma once

#include <Luch/Types.h>
#include <Luch/Graphics/Format.h>

namespace Luch::Render::Deferred::DeferredConstants
{
    using namespace Graphics;

    constexpr int32 GBufferColorAttachmentCount = 3;
    constexpr Array<Format, GBufferColorAttachmentCount> GBufferColorAttachmentFormats =
    {
        Format::RGBA8Unorm,
        Format::RGBA16Sfloat,
        Format::RGBA16Sfloat,
    };
}