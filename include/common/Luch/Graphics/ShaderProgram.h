#pragma once

#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
{
    class ShaderProgram : public GraphicsObject
    {
    public:
        ShaderProgram(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~ShaderProgram() = 0;
    };

    inline ShaderProgram::~ShaderProgram() {};
}
