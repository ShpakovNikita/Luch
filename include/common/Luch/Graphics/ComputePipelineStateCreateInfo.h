#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Graphics/GraphicsForwards.h>

namespace Luch::Graphics
{
    struct ComputePipelineStateCreateInfo
    {
        RefPtr<ShaderProgram> kernelProgram;

        PipelineLayout* pipelineLayout = nullptr;
        String name = "";
    };
}
