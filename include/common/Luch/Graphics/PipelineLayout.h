#pragma once

#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
{
    class PipelineLayout : public GraphicsObject
    {
    public:
        PipelineLayout(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~PipelineLayout() = 0;

        virtual const PipelineLayoutCreateInfo& GetCreateInfo() const = 0;
    };

    inline PipelineLayout::~PipelineLayout() {}
}
