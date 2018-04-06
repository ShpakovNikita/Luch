#pragma once

#include <Husky/Types.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/BaseObject.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/SceneV1/Forwards.h>
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
        TextureInfo emissiveTexture;
        AlphaMode alphaMode = AlphaMode::Opaque;
        Vec3 emissiveFactor = { 0, 0, 0 };
        float32 alphaCutoff = 0.5f;
        bool doubleSided = false;

        inline const RefPtr<Vulkan::DescriptorSet>& GetDescriptorSet() const { return descriptorSet; }
        inline void SetDescriptorSet(const RefPtr<Vulkan::DescriptorSet>& aDescriptorSet) { descriptorSet = aDescriptorSet; }
    private:
        RefPtr<Vulkan::DescriptorSet> descriptorSet;
    };
}
