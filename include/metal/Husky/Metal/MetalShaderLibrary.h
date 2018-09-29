#pragma once

#include <Husky/Graphics/ShaderLibrary.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
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
