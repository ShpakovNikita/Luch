#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class ShaderModule
    {
        friend class GraphicsDevice;

        ShaderModule() = default;

        ShaderModule(ShaderModule&& other);
        ShaderModule& operator=(ShaderModule&& other);

        ~ShaderModule();

        inline vk::ShaderModule GetModule() { return module; }
    private:
        ShaderModule(GraphicsDevice* device, vk::ShaderModule module);

        GraphicsDevice* device;
        vk::ShaderModule module;
    };
}