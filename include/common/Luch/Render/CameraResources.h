#pragma once

#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Size2.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>

namespace Luch::Render
{
    using namespace Graphics;

    struct CameraPersistentResources
    {
        RefPtr<DescriptorPool> descriptorPool;
        DescriptorSetBinding cameraUniformBufferBinding;
        RefPtr<DescriptorSetLayout> cameraBufferDescriptorSetLayout;
    };
}