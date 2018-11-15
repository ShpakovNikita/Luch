#include <Luch/Metal/MetalShaderProgram.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
{
    using namespace Graphics;

    MetalShaderProgram::MetalShaderProgram(
        MetalGraphicsDevice* device,
        mtlpp::Function aFunction)
        : ShaderProgram(device)
        , function(aFunction)
    {
    }
}
