#include <Husky/Metal/MetalShaderProgram.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
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
