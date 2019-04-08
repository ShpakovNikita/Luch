#pragma once

#include <Luch/Types.h>

namespace Luch::Render
{
    struct SceneRendererConfig
    {
        bool useGlobalIllumination = false;
        bool useDiffuseGlobalIllumination = false;
        bool useSpecularGlobalIllumination = false;
        bool useForward = true;
        bool useDepthPrepass = false;
        bool useComputeResolve = false;
        bool useTiledDeferredPass = false;
    };
}
