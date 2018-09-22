#pragma once

#include <Husky/Types.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/BaseObject.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/SceneV1/AlphaMode.h>

namespace Husky::SceneV1
{
    struct TextureInfo
    {
        RefPtr<Texture> texture;
        int32 texCoord = 0;
    };

    struct NormalTextureInfo : public TextureInfo
    {
        float32 scale = 1;
    };

    struct OcclusionTextureInfo : public TextureInfo
    {
        float32 strength = 1;
    };

    struct PbrMetallicRoughness
    {
        Vec4 baseColorFactor = { 1, 1, 1, 1 };
        TextureInfo baseColorTexture;
        float32 metallicFactor = 1;
        float32 roughnessFactor = 1;
        TextureInfo metallicRoughnessTexture;
    };

    class PbrMaterial : public BaseObject
    {
    public:
        PbrMaterial();
        ~PbrMaterial();

        String name;
        PbrMetallicRoughness metallicRoughness;
        NormalTextureInfo normalTexture;
        OcclusionTextureInfo occlusionTexture;
        Vec3 emissiveFactor = { 0, 0, 0 };
        TextureInfo emissiveTexture;
        AlphaMode alphaMode = AlphaMode::Opaque;
        float32 alphaCutoff = 0.5f;
        bool doubleSided = false;

        inline bool HasBaseColorTexture() const { return metallicRoughness.baseColorTexture.texture != nullptr; }
        inline bool HasMetallicRoughnessTexture() const { return metallicRoughness.metallicRoughnessTexture.texture != nullptr; }
        inline bool HasNormalTexture() const { return normalTexture.texture != nullptr; }
        inline bool HasOcclusionTexture() const { return occlusionTexture.texture != nullptr; }
        inline bool HasEmissiveTexture() const { return emissiveTexture.texture != nullptr; }
    };
}
