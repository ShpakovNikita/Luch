#pragma once

#include <Husky/Graphics/GraphicsObject.h>

namespace Husky::Graphics
{
    class ShaderProgram : public GraphicsObject
    {
    public:
        ShaderProgram(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~ShaderProgram() = 0;
    };

    inline ShaderProgram::~ShaderProgram() {};
}
