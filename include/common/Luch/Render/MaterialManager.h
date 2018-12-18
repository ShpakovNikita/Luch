#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/ResultValue.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/RenderForwards.h>

namespace Luch::Render
{
    using namespace Graphics;

    struct MaterialResources
    {
        RefPtr<DescriptorPool> descriptorPool;

        // Material bindings
        DescriptorSetBinding materialUniformBufferBinding;
        DescriptorSetBinding baseColorTextureBinding;
        DescriptorSetBinding baseColorSamplerBinding;
        DescriptorSetBinding metallicRoughnessTextureBinding;
        DescriptorSetBinding metallicRoughnessSamplerBinding;
        DescriptorSetBinding normalTextureBinding;
        DescriptorSetBinding normalSamplerBinding;
        DescriptorSetBinding occlusionTextureBinding;
        DescriptorSetBinding occlusionSamplerBinding;
        DescriptorSetBinding emissiveTextureBinding;
        DescriptorSetBinding emissiveSamplerBinding;

        RefPtr<DescriptorSetLayout> materialTextureDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> materialBufferDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> materialSamplerDescriptorSetLayout;

        SharedPtr<SharedBuffer> sharedBuffer;
    };

    class MaterialManager
    {
        static constexpr int32 MaxTexturesPerMaterial = 5;
        static constexpr int32 SharedBufferSize = 1024 * 1024;
        static constexpr int32 DescriptorSetCount = 1024;
        static constexpr int32 DescriptorCount = 1024;
    public:
        bool Initialize(SharedPtr<RenderContext> context);
        bool Deinitialize();

        bool PrepareMaterial(SceneV1::PbrMaterial* material);
        void UpdateMaterial(SceneV1::PbrMaterial* material);

        MaterialResources* GetResources() { return resources.get(); }
    private:
        SharedPtr<RenderContext> context;
        UniquePtr<MaterialResources> resources;
    };
}
