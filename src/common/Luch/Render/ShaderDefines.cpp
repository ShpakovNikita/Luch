#include <Luch/Render/ShaderDefines.h>

namespace Luch::Render
{
    namespace SemanticShaderDefines
    {
        const String Empty = "";
        const String HasNormal = "HAS_NORMAL";
        const String HasTangent = "HAS_TANGENT";
        const String HasTexCoord0 = "HAS_TEXCOORD_0";
    }

    namespace MaterialShaderDefines
    {
        const String HasBaseColorTexture = "HAS_BASE_COLOR_TEXTURE";
        const String HasMetallicRoughnessTexture = "HAS_METALLIC_ROUGHNESS_TEXTURE";
        const String HasNormalTexture = "HAS_NORMAL_TEXTURE";
        const String HasOcclusionTexture = "HAS_OCCLUSION_TEXTURE";
        const String HasEmissiveTexture = "HAS_EMISSIVE_TEXTURE";

        const String AlphaMask = "ALPHA_MASK";
    };
}
