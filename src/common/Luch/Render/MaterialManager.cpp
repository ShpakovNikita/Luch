#include <Luch/Render/MaterialManager.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/RenderUtils.h>

#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Texture.h>
#include <Luch/SceneV1/Sampler.h>

#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/BufferCreateInfo.h>

namespace Luch::Render
{
    using namespace Graphics;

    bool MaterialManager::Initialize(GraphicsDevice* aDevice)
    {
        device = aDevice;

        resources = MakeUnique<MaterialResources>();

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.maxDescriptorSets = DescriptorSetCount;
        descriptorPoolCreateInfo.descriptorCount =
        {
            { ResourceType::Texture, MaxTexturesPerMaterial * DescriptorCount },
            { ResourceType::Sampler, MaxTexturesPerMaterial * DescriptorCount },
            { ResourceType::UniformBuffer, DescriptorCount },
        };

        auto[createDescriptorPoolResult, createdDescriptorPool] = device->CreateDescriptorPool(
            descriptorPoolCreateInfo);

        if (createDescriptorPoolResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return false;
        }

        resources->descriptorPool = std::move(createdDescriptorPool);

        resources->materialUniformBufferBinding.OfType(ResourceType::UniformBuffer);

        resources->baseColorTextureBinding.OfType(ResourceType::Texture);
        resources->baseColorSamplerBinding.OfType(ResourceType::Sampler);

        resources->metallicRoughnessTextureBinding.OfType(ResourceType::Texture);
        resources->metallicRoughnessSamplerBinding.OfType(ResourceType::Sampler);

        resources->normalTextureBinding.OfType(ResourceType::Texture);
        resources->normalSamplerBinding.OfType(ResourceType::Sampler);

        resources->occlusionTextureBinding.OfType(ResourceType::Texture);
        resources->occlusionSamplerBinding.OfType(ResourceType::Sampler);

        resources->emissiveTextureBinding.OfType(ResourceType::Texture);
        resources->emissiveSamplerBinding.OfType(ResourceType::Sampler);

        DescriptorSetLayoutCreateInfo materialBufferDescriptorSetLayoutCreateInfo;
        materialBufferDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .AddBinding(&resources->materialUniformBufferBinding);

        DescriptorSetLayoutCreateInfo materialTextureDescriptorSetLayoutCreateInfo;
        materialTextureDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(5)
            .AddBinding(&resources->baseColorTextureBinding)
            .AddBinding(&resources->metallicRoughnessTextureBinding)
            .AddBinding(&resources->normalTextureBinding)
            .AddBinding(&resources->occlusionTextureBinding)
            .AddBinding(&resources->emissiveTextureBinding);

        DescriptorSetLayoutCreateInfo materialSamplerDescriptorSetLayoutCreateInfo;
        materialSamplerDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Sampler)
            .WithNBindings(5)
            .AddBinding(&resources->baseColorSamplerBinding)
            .AddBinding(&resources->metallicRoughnessSamplerBinding)
            .AddBinding(&resources->normalSamplerBinding)
            .AddBinding(&resources->occlusionSamplerBinding)
            .AddBinding(&resources->emissiveSamplerBinding);

        auto[createMaterialTextureDescriptorSetLayoutResult, createdMaterialTextureDescriptorSetLayout] = device->CreateDescriptorSetLayout(
            materialTextureDescriptorSetLayoutCreateInfo);

        if (createMaterialTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            return false;
        }

        resources->materialTextureDescriptorSetLayout = std::move(createdMaterialTextureDescriptorSetLayout);

        auto[createMaterialBufferDescriptorSetLayoutResult, createdMaterialBufferDescriptorSetLayout] = device->CreateDescriptorSetLayout(
            materialBufferDescriptorSetLayoutCreateInfo);

        if (createMaterialBufferDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            return false;
        }

        resources->materialBufferDescriptorSetLayout = std::move(createdMaterialBufferDescriptorSetLayout);

        auto[createMaterialSamplerDescriptorSetLayoutResult, createdMaterialSamplerDescriptorSetLayout] = device->CreateDescriptorSetLayout(
            materialSamplerDescriptorSetLayoutCreateInfo);

        if (createMaterialSamplerDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            return false;
        }

        resources->materialSamplerDescriptorSetLayout = std::move(createdMaterialSamplerDescriptorSetLayout);

        return true;
    }

    bool MaterialManager::Deinitialize()
    {
        resources.reset();
        device = nullptr;

        return true;
    }

    bool MaterialManager::PrepareMaterial(SceneV1::PbrMaterial* material)
    {
        auto[allocateTextureDescriptorSetResult, textureDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
            resources->materialTextureDescriptorSetLayout);
        if(allocateTextureDescriptorSetResult != GraphicsResult::Success)
        {
            return false;
        }

        auto[allocateBufferDescriptorSetResult, bufferDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
            resources->materialBufferDescriptorSetLayout);
        if(allocateBufferDescriptorSetResult != GraphicsResult::Success)
        {
            return false;
        }

        auto[allocateSamplerDescriptorSetResult, samplerDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
            resources->materialSamplerDescriptorSetLayout);
        if(allocateSamplerDescriptorSetResult != GraphicsResult::Success)
        {
            return false;
        }

        material->SetTextureDescriptorSet(textureDescriptorSet);
        material->SetBufferDescriptorSet(bufferDescriptorSet);
        material->SetSamplerDescriptorSet(samplerDescriptorSet);

        return true;
    }

    void MaterialManager::UpdateMaterial(SceneV1::PbrMaterial* material, SharedBuffer* sharedBuffer)
    {
        auto textureDescriptorSet = material->GetTextureDescriptorSet();
        auto samplerDescriptorSet = material->GetSamplerDescriptorSet();
        auto bufferDescriptorSet = material->GetBufferDescriptorSet();

        if (material->HasBaseColorTexture())
        {
            textureDescriptorSet->WriteTexture(
                resources->baseColorTextureBinding,
                material->GetBaseColorTexture()->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                resources->baseColorSamplerBinding,
                material->GetBaseColorTexture()->GetDeviceSampler());
        }

        if (material->HasMetallicRoughnessTexture())
        {
            textureDescriptorSet->WriteTexture(
                resources->metallicRoughnessTextureBinding,
                material->GetMetallicRoughnessTexture()->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                resources->metallicRoughnessSamplerBinding,
                material->GetMetallicRoughnessTexture()->GetDeviceSampler());
        }

        if (material->HasNormalTexture())
        {
            textureDescriptorSet->WriteTexture(
                resources->normalTextureBinding,
                material->GetNormalTexture()->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                resources->normalSamplerBinding,
                material->GetNormalTexture()->GetDeviceSampler());
        }

        if (material->HasOcclusionTexture())
        {
            textureDescriptorSet->WriteTexture(
                resources->occlusionTextureBinding,
                material->GetOcclusionTexture()->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                resources->occlusionSamplerBinding,
                material->GetOcclusionTexture()->GetDeviceSampler());
        }

        if (material->HasEmissiveTexture())
        {
            textureDescriptorSet->WriteTexture(
                resources->emissiveTextureBinding,
                material->GetEmissiveTexture()->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                resources->emissiveSamplerBinding,
                material->GetEmissiveTexture()->GetDeviceSampler());
        }

        MaterialUniform materialUniform = RenderUtils::GetMaterialUniform(material);

        // TODO
        auto suballocation = sharedBuffer->Suballocate(sizeof(MaterialUniform), 256);

        bufferDescriptorSet->WriteUniformBuffer(
            resources->materialUniformBufferBinding,
            suballocation.buffer,
            suballocation.offset);

        memcpy(suballocation.offsetMemory, &materialUniform, sizeof(MaterialUniform));

        textureDescriptorSet->Update();
        samplerDescriptorSet->Update();
        bufferDescriptorSet->Update();
    }
}
