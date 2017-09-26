#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/ShaderModule.h>

namespace Husky::Vulkan
{
    class ShaderCompiler
    {
    public:
        virtual ~ShaderCompiler() {}
    }
}