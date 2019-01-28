#pragma once

#include <Luch/Types.h>

namespace Luch::Render
{
    struct ShaderDefines
    {
        UnorderedMap<String, Variant<int32, String>> defines;

        inline void AddFlag(String flag)
        {
            AddDefine(flag, "1");
        }

        inline void AddDefine(String define, const Variant<int32, String>& value)
        {
            if(!define.empty())
            {
                defines[define] = value;
            }
        }
    };

    namespace SemanticShaderDefines
    {
        extern const String HasNormal;
        extern const String HasTangent;
        extern const String HasTexCoord0;
    }

    namespace MaterialShaderDefines
    {
        extern const String HasBaseColorTexture;
        extern const String HasMetallicRoughnessTexture;
        extern const String HasNormalTexture;
        extern const String HasOcclusionTexture;
        extern const String HasEmissiveTexture;

        extern const String AlphaMask;
    };
}
