#pragma once

#include <Luch/Types.h>
#include <Luch/Graphics/Format.h>

namespace Luch::Render::Deferred::ShadowMapping
{
    using namespace Graphics;

    class ShadowMappingOptions
    {
    public:
        int32 shadowMapSize = 256;
        Format shadowMapFormat = Format::D32Sfloat;
    };
}
