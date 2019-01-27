#pragma once

#include <Luch/Types.h>

namespace Luch::Render
{
    struct SceneRendererConfig
    {
        bool useDepthPrepass = false;
        bool useComputeResolve = false;
        bool useComputeTonemap = false;
    };
}
