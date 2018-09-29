#pragma once

#include <Husky/Graphics/ShaderProgram.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalShaderProgram : public ShaderProgram
    {
    public:
        MetalShaderProgram(
            MetalGraphicsDevice* device,
            mtlpp::Function function);

        inline mtlpp::Function GetMetalFunction() { return function; }
    private:
        mtlpp::Function function;
    };
}
