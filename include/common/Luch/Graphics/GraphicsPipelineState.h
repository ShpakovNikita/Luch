#pragma once

#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
{
    class GraphicsPipelineState : public GraphicsObject
    {
    public:
        GraphicsPipelineState(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~GraphicsPipelineState() = 0;

        virtual const GraphicsPipelineStateCreateInfo& GetCreateInfo() const = 0;
    };

    inline GraphicsPipelineState::~GraphicsPipelineState() {}
}
