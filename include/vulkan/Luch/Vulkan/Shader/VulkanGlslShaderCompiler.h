#pragma once

#include <Luch/Graphics/ShaderStage.h>
#include <Luch/Types.h>

namespace Luch::Vulkan
{
    class VulkanGLSLShaderCompiler
    {
    public:
        static void Initialize();
        static void Deinitialize();
        static bool TryCompileShader(
            Graphics::ShaderStage shaderStage,
            const Vector<char8>& glslSource,
            Vector<uint32_t>& spirvBytecode,
            const UnorderedMap<String, Variant<int32, String>>& flags);
    private:
        static String GeneratePreamble(const UnorderedMap<String, Variant<int32, String>>& flags);
    };
}
