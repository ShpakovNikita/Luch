#include <Husky/Vulkan/ShaderModule.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    ShaderModule::ShaderModule(GraphicsDevice* aDevice, vk::ShaderModule aModule)
        : device(aDevice)
        , module(aModule)
    {
    }

    ShaderModule::ShaderModule(ShaderModule&& other)
        : device(other.device)
        , module(other.module)
    {
        other.device = nullptr;
        other.module = nullptr;
    }

    ShaderModule& ShaderModule::operator=(ShaderModule&& other)
    {
        device = other.device;
        module = other.module;

        other.device = nullptr;
        other.module = nullptr;

        return *this;
    }

    ShaderModule::~ShaderModule()
    {
        Destroy();
    }

    void ShaderModule::Destroy()
    {
        if (device)
        {
            device->DestroyShaderModule(this);
        }
    }
}