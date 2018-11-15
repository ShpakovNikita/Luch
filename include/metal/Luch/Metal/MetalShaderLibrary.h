#pragma once

#include <Luch/Graphics/ShaderLibrary.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalShaderLibrary : public ShaderLibrary
    {
    public:
        MetalShaderLibrary(
            MetalGraphicsDevice* device,
            mtlpp::Library library);

        GraphicsResultRefPtr<ShaderProgram> CreateShaderProgram(
            ShaderStage stage,
            const String& name) override;
    private:
        mtlpp::Library library;
    };

}
