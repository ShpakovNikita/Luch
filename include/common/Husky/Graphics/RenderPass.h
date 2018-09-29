#pragma once

#include <Husky/Graphics/GraphicsObject.h>

namespace Husky::Graphics
{
    class RenderPass : public GraphicsObject
    {
    public:
        RenderPass(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~RenderPass() = 0;

        virtual const RenderPassCreateInfo& GetCreateInfo() const = 0;
    };

    inline RenderPass::~RenderPass() {}
}

