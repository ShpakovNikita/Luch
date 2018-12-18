#pragma once

#include <Luch/Types.h>
#include <Luch/VectorTypes.h>
#include <Luch/RefPtr.h>
#include <Luch/BaseObject.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/SceneV1/AlphaMode.h>
#include <Luch/Flags.h>

namespace Luch::SceneV1
{
    struct TextureInfo
    {
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
        TextureInfo baseColorTextureInfo;
        Vec4 baseColorFactor = { 1, 1, 1, 1 };
        float32 metallicFactor = 1;
        float32 roughnessFactor = 1;
        TextureInfo metallicRoughnessTextureInfo;
    };

    struct PbrMaterialProperties
    {
        PbrMetallicRoughness metallicRoughness;
        NormalTextureInfo normalTextureInfo;
        OcclusionTextureInfo occlusionTextureInfo;
        Vec3 emissiveFactor = { 0, 0, 0 };
        TextureInfo emissiveTextureInfo;
        AlphaMode alphaMode = AlphaMode::Opaque;
        float32 alphaCutoff = 0.5f;
        bool doubleSided = false;
    };

    enum PbrMaterialDirtyFlags
    {
        None = 0,
        Properties = 1 << 0,
        BaseColorTexture = 1 << 1,
        NormalTexture = 1 << 2,
        MetallicRoughnessTexture = 1 << 3,
        OcclusionTexture = 1 << 4,
        EmissiveTexture = 1 << 5,
        Textures = BaseColorTexture | NormalTexture | MetallicRoughnessTexture | OcclusionTexture | EmissiveTexture,
        All = Properties | Textures,
    };

    DEFINE_OPERATORS_FOR_FLAGS_ENUM(PbrMaterialDirtyFlags);

    class PbrMaterial : public BaseObject
    {
        friend class Scene;
    public:
        PbrMaterial();
        ~PbrMaterial();

        inline PbrMaterialDirtyFlags GetDirtyFlags() const { return dirtyFlags; }
        inline void SetDirtyFlagBits(PbrMaterialDirtyFlags bits) { dirtyFlags |= bits; }
        inline void ClearDirtyFlagBits(PbrMaterialDirtyFlags bits) { dirtyFlags &= ~bits; }

        inline const PbrMaterialProperties& GetProperties() const { return properties; }
        void SetProperties(const PbrMaterialProperties& properties);

        inline const String& GetName() const { return name; }
        inline void SetName(String aName) { name = aName; }

        inline bool HasBaseColorTexture() const { return baseColorTexture != nullptr; }
        inline bool HasMetallicRoughnessTexture() const { return metallicRoughnessTexture != nullptr; }
        inline bool HasNormalTexture() const { return normalTexture != nullptr; }
        inline bool HasOcclusionTexture() const { return occlusionTexture != nullptr; }
        inline bool HasEmissiveTexture() const { return emissiveTexture != nullptr; }

        inline const RefPtr<Texture>& GetBaseColorTexture() { return baseColorTexture; }
        inline const RefPtr<Texture>& GetMetallicRoughnessTexture() { return metallicRoughnessTexture; }
        inline const RefPtr<Texture>& GetNormalTexture() { return normalTexture; }
        inline const RefPtr<Texture>& GetOcclusionTexture() { return occlusionTexture; }
        inline const RefPtr<Texture>& GetEmissiveTexture() { return emissiveTexture; }

        void SetBaseColorTexture(const RefPtr<Texture>& texture);
        void SetMetallicRoughnessTexture(const RefPtr<Texture>& texture);
        void SetNormalTexture(const RefPtr<Texture>& texture);
        void SetOcclusionTexture(const RefPtr<Texture>& texture);
        void SetEmissiveTexture(const RefPtr<Texture>& texture);

        inline const RefPtr<Graphics::DescriptorSet>& GetTextureDescriptorSet()
        {
            return textureDescriptorSet;
        }

        inline const RefPtr<Graphics::DescriptorSet>& GetBufferDescriptorSet()
        {
            return bufferDescriptorSet;
        }

        inline const RefPtr<Graphics::DescriptorSet>& GetSamplerDescriptorSet()
        {
            return samplerDescriptorSet;
        }

        inline void SetTextureDescriptorSet(
            const RefPtr<Graphics::DescriptorSet>& aTextureDescriptorSet)
        {
            textureDescriptorSet = aTextureDescriptorSet;
        }

        inline void SetBufferDescriptorSet(
            const RefPtr<Graphics::DescriptorSet>& aBufferDescriptorSet)
        {
            bufferDescriptorSet = aBufferDescriptorSet;
        }

        inline void SetSamplerDescriptorSet(
            const RefPtr<Graphics::DescriptorSet>& aSamplerDescriptorSet)
        {
            samplerDescriptorSet = aSamplerDescriptorSet;
        }
    private:
        PbrMaterialDirtyFlags dirtyFlags = PbrMaterialDirtyFlags::All;
        String name;

        PbrMaterialProperties properties;
        RefPtr<Texture> baseColorTexture;
        RefPtr<Texture> normalTexture;
        RefPtr<Texture> metallicRoughnessTexture;
        RefPtr<Texture> occlusionTexture;
        RefPtr<Texture> emissiveTexture;

        RefPtr<Graphics::DescriptorSet> textureDescriptorSet;
        RefPtr<Graphics::DescriptorSet> bufferDescriptorSet;
        RefPtr<Graphics::DescriptorSet> samplerDescriptorSet;
    };
}
