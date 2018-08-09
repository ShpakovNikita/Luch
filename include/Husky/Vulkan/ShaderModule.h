#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class ShaderModule : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        ShaderModule(GraphicsDevice* device, vk::ShaderModule module);
        ~ShaderModule() override;

        inline vk::ShaderModule GetModule() { return module; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::ShaderModule module;
    };
}
