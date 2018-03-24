#pragma once

#include <Husky/Types.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/BaseObject.h>
#include <Husky/SceneV1/Forwards.h>
#include <Husky/SceneV1/AlphaMode.h>

namespace Husky::SceneV1
{
    struct PbrMetallicRoughness
    {
        Vec4 baseColorFactor = { 1, 1, 1, 1 };
        RefPtr<Texture> baseColorTexture;
        float32 metallicFactor = 1;
        float32 roughnessFactor = 1;
        RefPtr<Texture> metallicRoughnessTexture;
    };

    class PbrMaterial : public BaseObject
    {
    public:
        PbrMaterial();
        ~PbrMaterial();

        String name;
        PbrMetallicRoughness metallicRoughness;
        RefPtr<Texture> normalTexture;
        float32 normalScale = 1;
        RefPtr<Texture> occlusionTexture;
        float32 occlusionStrength = 1;
        RefPtr<Texture> emissiveTexture;
        AlphaMode alphaMode = AlphaMode::Opaque;
        Vec3 emissiveFactor = { 0, 0, 0 };
        float32 alphaCutoff = 0.5f;
        bool doubleSided = false;
    };
}
