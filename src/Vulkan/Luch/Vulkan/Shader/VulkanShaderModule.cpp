#include <Luch/Vulkan/Shader/VulkanShaderModule.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Vulkan/Shader/VulkanGlslShaderCompiler.h>

namespace Luch::Vulkan
{
    VulkanShaderProgram::VulkanShaderProgram(VulkanGraphicsDevice* device, vk::ShaderModule aModule)
        : ShaderProgram (device)
        , module(aModule)
    {
    }

    VulkanShaderProgram::~VulkanShaderProgram()
    {
        Destroy();
    }

    void VulkanShaderProgram::Destroy()
    {
        if (device)
        {
            device->GetDevice().destroyShaderModule(module, device->GetAllocationCallbacks());
        }
    }

    VulkanShaderModule::VulkanShaderModule(VulkanGraphicsDevice* aDevice, const Vector<Byte> &aGlslSource,
                                           const UnorderedMap<String, Variant<int32, String> > &aDefines)
        : ShaderLibrary (aDevice)
        , device(aDevice)
        , glslSource(aGlslSource)
        , defines(aDefines)
    {
    }

    VulkanShaderModule::~VulkanShaderModule()
    {
    }

    GraphicsResultRefPtr<ShaderProgram> VulkanShaderModule::CreateShaderProgram(
                ShaderStage stage,
                const String& name)
    {
        Vector<uint32_t> spirvBytecode;
        bool success = VulkanGLSLShaderCompiler::TryCompileShader(stage, glslSource, spirvBytecode, defines);
        if (!success)
        {
            return GraphicsResult::CompilerError;
        }

        vk::ShaderModuleCreateInfo ci;
        ci.setPCode(reinterpret_cast<const uint32_t*>(spirvBytecode.data()));
        ci.setCodeSize(spirvBytecode.size() * sizeof(uint32_t));
        auto [createResult, vulkanShaderModule] = device->GetDevice().createShaderModule(ci, device->GetAllocationCallbacks());
        if (createResult != vk::Result::eSuccess)
        {
            device->GetDevice().destroyShaderModule(vulkanShaderModule, device->GetAllocationCallbacks());
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<VulkanShaderProgram>(device, vulkanShaderModule) };
        }
    }


}
