#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>
#include <Luch/Graphics/ShaderLibrary.h>
#include <Luch/Graphics/ShaderProgram.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    class VulkanShaderProgram : public ShaderProgram
    {
    public:
        VulkanShaderProgram(VulkanGraphicsDevice* device, vk::ShaderModule module);
        ~VulkanShaderProgram() override;

        inline vk::ShaderModule GetModule() { return module; }

    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::ShaderModule module;
    };

    class VulkanShaderModule : public ShaderLibrary
    {
    public:
        VulkanShaderModule(VulkanGraphicsDevice* aDevice, const Vector<Byte> &aGlslSource,
                           const UnorderedMap<String, Variant<int32, String>> &aDefines);
        ~VulkanShaderModule() override;

        GraphicsResultRefPtr<ShaderProgram> CreateShaderProgram(
                    ShaderStage stage,
                    const String& name) override;

    private:
        VulkanGraphicsDevice* device = nullptr;
        Vector<Byte> glslSource;
        UnorderedMap<String, Variant<int32, String>> defines;
    };
}
