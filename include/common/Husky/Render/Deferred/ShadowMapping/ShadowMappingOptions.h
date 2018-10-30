#pragma once

#include <Husky/Types.h>
#include <Husky/Graphics/Format.h>

namespace Husky::Render::Deferred::ShadowMapping
{
    using namespace Graphics;

    class ShadowMappingOptions
    {
    public:
        int32 shadowMapSize = 256;
        Format shadowMapFormat = Format::D32Sfloat;
    };
}
