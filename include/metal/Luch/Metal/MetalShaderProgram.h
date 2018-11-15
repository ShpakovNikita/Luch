#pragma once

#include <Luch/Graphics/ShaderProgram.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
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
