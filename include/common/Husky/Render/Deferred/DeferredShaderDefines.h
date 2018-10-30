#pragma once

namespace Husky::Render::Deferred
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
