#pragma once

#include <Luch/Types.h>
#include <Luch/Graphics/Format.h>

namespace Luch::Render::Deferred::DeferredConstants
{
    using namespace Graphics;

    constexpr int32 GBufferColorAttachmentCount = 3;
    constexpr Array<Format, GBufferColorAttachmentCount> GBufferColorAttachmentFormats =
    {
        Format::R8G8B8A8Unorm,
        Format::R32G32B32A32Sfloat,
        Format::R32G32B32A32Sfloat,
    };
}