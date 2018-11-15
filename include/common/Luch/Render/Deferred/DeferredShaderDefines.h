#pragma once

namespace Luch::Render::Deferred
{
    enum class DeferredShaderDefines
    {
        Empty, // special value for no define
        //HasPosition,
        HasNormal,
        HasTangent,
        HasTexCoord0,

        HasBaseColorTexture,
        HasMetallicRoughnessTexture,
        HasNormalTexture,
        HasOcclusionTexture,
        HasEmissiveTexture,

        AlphaMask,
    };
}
