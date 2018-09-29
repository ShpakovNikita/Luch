#pragma once

#include <Husky/Graphics/GraphicsObject.h>
#include <Husky/Graphics/GraphicsResultValue.h>
#include <Husky/Graphics/ShaderStage.h>

namespace Husky::Graphics
{
    class ShaderLibrary : public GraphicsObject
    {
    public:
        ShaderLibrary(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~ShaderLibrary() = 0;

        virtual GraphicsResultRefPtr<ShaderProgram> CreateShaderProgram(
            ShaderStage stage,
            const String& name) = 0;
    };

    inline ShaderLibrary::~ShaderLibrary() {};
}
