#pragma once

#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
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

