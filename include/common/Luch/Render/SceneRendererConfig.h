#pragma once

#include <Luch/Types.h>

namespace Luch::Render
{
    struct SceneRendererConfig
    {
        bool useForward = false;
        bool useDepthPrepass = false;
        bool useComputeResolve = false;
        bool useTiledDeferredPass = true;
    };
}
