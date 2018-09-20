#pragma once

#include <Husky/Graphics/GraphicsForwards.h>

namespace Husky::Graphics
{
    class CommandPool : public BaseObject
    {
    public:
        virutal ~CommandPool() = 0 {};

        virtual RefPtr<GraphicsCommandList> CreateGraphicsCommandList() = 0;
        virtual RefPtr<ComputeCommandList> CreateComputeCommandList() = 0;
    };
}
