#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Texture.h>

namespace Luch::SceneV1
{
    // TODO
    PbrMaterial::PbrMaterial() = default;
    PbrMaterial::~PbrMaterial() = default;

    void PbrMaterial::SetBaseColorTexture(const RefPtr<Texture>& aTexture)
    {
        if(baseColorTexture != aTexture)
        {
            baseColorTexture = aTexture;
            SetDirtyFlagBits(PbrMaterialDirtyFlags::BaseColorTexture);
        }
    }

    void PbrMaterial::SetMetallicRoughnessTexture(const RefPtr<Texture>& aTexture)
    {
        if(metallicRoughnessTexture != aTexture)
        {
            metallicRoughnessTexture = aTexture;
            SetDirtyFlagBits(PbrMaterialDirtyFlags::MetallicRoughnessTexture);
        }
    }

    void PbrMaterial::SetNormalTexture(const RefPtr<Texture>& aTexture)
    {
        if(normalTexture != aTexture)
        {
            normalTexture = aTexture;
            SetDirtyFlagBits(PbrMaterialDirtyFlags::NormalTexture);
        }
    }

    void PbrMaterial::SetOcclusionTexture(const RefPtr<Texture>& aTexture)
    {
        if(occlusionTexture != aTexture)
        {
            occlusionTexture = aTexture;
            SetDirtyFlagBits(PbrMaterialDirtyFlags::OcclusionTexture);
        }
    }

    void PbrMaterial::SetEmissiveTexture(const RefPtr<Texture>& aTexture)
    {
        if(emissiveTexture != aTexture)
        {
            emissiveTexture = aTexture;
            SetDirtyFlagBits(PbrMaterialDirtyFlags::EmissiveTexture);
        }
    }

    void PbrMaterial::SetProperties(const PbrMaterialProperties& aProperties)
    {
        properties = aProperties;
        SetDirtyFlagBits(PbrMaterialDirtyFlags::Properties);
    }
}
