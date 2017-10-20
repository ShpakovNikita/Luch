#pragma once

#include <Husky/ShaderStage.h>
#include <Husky/Types.h>

namespace Husky::Vulkan
{
    class GlslShaderCompiler
    {
    public:
        using Bytecode = Vector<uint32>;
        bool TryCompileShader(ShaderStage shaderStage, const char8* glslSource, Bytecode& spirvBytecode);
    };
}
