#pragma once

#include <Husky/Graphics/GraphicsObject.h>

namespace Husky::Graphics
{
    class RenderPass : public GraphicsObject
    {
    public:
        RenderPass(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~RenderPass() = 0 {};

        virtual const RenderPass& GetCreateInfo() const = 0;
    }
}

