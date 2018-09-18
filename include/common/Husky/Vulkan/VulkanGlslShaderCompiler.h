#pragma once

#include <Husky/ShaderStage.h>
#include <Husky/Types.h>

namespace Husky::Vulkan
{
    class VulkanGLSLShaderCompiler
    {
    public:
        using Bytecode = Vector<uint32>;
        static void Initialize();
        static void Deinitialize();
        bool TryCompileShader(
            ShaderStage shaderStage,
            const Vector<Byte>& glslSource,
            Bytecode& spirvBytecode,
            const UnorderedMap<String, String>& flags);
    private:
        String GeneratePreamble(const UnorderedMap<String, String>& flags);
    };
}
