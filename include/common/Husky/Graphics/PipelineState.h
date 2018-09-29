#pragma once

#include <Husky/Graphics/GraphicsObject.h>

namespace Husky::Graphics
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
