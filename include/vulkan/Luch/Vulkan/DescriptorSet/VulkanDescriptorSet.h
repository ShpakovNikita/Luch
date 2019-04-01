#pragma once

#include <vulkan/vulkan.hpp>
#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/DescriptorSetLayout.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    class VulkanDescriptorSet : public DescriptorSet
    {
        friend class VulkanGraphicsDevice;
        friend class VulkanDescriptorPool;
    public:
        VulkanDescriptorSet(VulkanGraphicsDevice* device, vk::DescriptorSet descriptorSet);

        // todo: implement
        DescriptorSetLayout* GetDescriptorSetLayout() { return layout; }

        void WriteTexture(const DescriptorSetBinding& binding, Texture* texture) {}
        void WriteUniformBuffer(const DescriptorSetBinding& binding, Buffer* buffer, int32 offset) {}
        void WriteThreadgroupMemory(const DescriptorSetBinding& binding, int32 length, int32 offset) {}
        void WriteSampler(const DescriptorSetBinding& binding, Sampler* sampler) {}
        void Update() {}
        // endof todo: implement

        vk::Result Free();

        static void Update(const VulkanDescriptorSetWrites& writes);

        inline vk::DescriptorSet GetDescriptorSet() { return descriptorSet; }
    private:
        VulkanGraphicsDevice* device = nullptr;
        DescriptorSetLayout* layout = nullptr; // todo: implement
        vk::DescriptorPool descriptorPool;
        vk::DescriptorSet descriptorSet;
    };
}
