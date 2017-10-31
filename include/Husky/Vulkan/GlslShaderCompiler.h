#pragma once

#include <Husky/ShaderStage.h>
#include <Husky/Types.h>

namespace Husky::Vulkan
{
    class GLSLShaderCompiler
    {
    public:
        using Bytecode = Vector<uint32>;
        static void Initialize();
        static void Deinitialize();
        bool TryCompileShader(ShaderStage shaderStage, const char8* glslSource, Bytecode& spirvBytecode);
    };
}
