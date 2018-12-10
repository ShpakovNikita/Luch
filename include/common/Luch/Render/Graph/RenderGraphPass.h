#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Types.h>
#include <Luch/Graphics/BufferUsageFlags.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/TextureUsageFlags.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    class RenderGraphPass
    {
    public:
        virtual inline ~RenderGraphPass() {}
        virtual void ExecuteGraphicsPass(
            RenderGraphResourceManager* resourceManager,
            GraphicsCommandList* commandList) = 0;
    };
}
