#pragma once

#include <Luch/Types.h>

namespace Luch::Render
{
    struct SceneRendererConfig
    {
        bool useDiffuseGlobalIllumination = false;
        bool useSpecularGlobalIllumination = false;
        bool useForward = false;
        bool useDepthPrepass = false;
        bool useComputeResolve = false;
        bool useTiledDeferredPass = true;
    };
}
