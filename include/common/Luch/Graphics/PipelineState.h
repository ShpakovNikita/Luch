#pragma once

#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
{
    class PipelineState : public GraphicsObject
    {
    public:
        PipelineState(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~PipelineState() = 0;

        virtual const PipelineStateCreateInfo& GetCreateInfo() const = 0;
    };

    inline PipelineState::~PipelineState() {}
}
