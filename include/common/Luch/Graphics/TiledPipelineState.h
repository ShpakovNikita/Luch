#pragma once

#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
{
    class TiledPipelineState : public GraphicsObject
    {
    public:
        TiledPipelineState(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~TiledPipelineState() = 0;

        virtual const TiledPipelineStateCreateInfo& GetCreateInfo() const = 0;
    };

    inline TiledPipelineState::~TiledPipelineState() {}
}
