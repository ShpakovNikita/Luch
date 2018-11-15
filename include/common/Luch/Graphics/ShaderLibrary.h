#pragma once

#include <Luch/Graphics/GraphicsObject.h>
#include <Luch/Graphics/GraphicsResultValue.h>
#include <Luch/Graphics/ShaderStage.h>

namespace Luch::Graphics
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
