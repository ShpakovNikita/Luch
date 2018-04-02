#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class ShaderModule : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        ShaderModule(GraphicsDevice* device, vk::ShaderModule module);

        ShaderModule(const ShaderModule& other) = delete;
        ShaderModule(ShaderModule&& other) = delete;
        ShaderModule& operator=(const ShaderModule& other) = delete;
        ShaderModule& operator=(ShaderModule&& other) = delete;

        ~ShaderModule() override;

        inline vk::ShaderModule GetModule() { return module; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::ShaderModule module;
    };
}
