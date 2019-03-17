#pragma once

#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
{
    class ComputePipelineState : public GraphicsObject
    {
    public:
        ComputePipelineState(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~ComputePipelineState() = 0;

        virtual const ComputePipelineStateCreateInfo& GetCreateInfo() const = 0;
    };

    inline ComputePipelineState::~ComputePipelineState() {}
}
