#pragma once

#include <Husky/Graphics/GraphicsObject.h>

namespace Husky::Graphics
{
    class PipelineLayout : public GraphicsObject
    {
    public:
        virtual ~PipelineLayout() = 0 {};

        virtual const PipelineLayoutCreateInfo& GetCreateInfo() const = 0;
    }
}
