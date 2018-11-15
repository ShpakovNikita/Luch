#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Deferred/DeferredForwards.h>

namespace Luch::Render::Deferred
{
    using namespace Graphics;

    struct DeferredResources
    {
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<DescriptorSetLayout> cameraBufferDescriptorSetLayout;
        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        DescriptorSetBinding cameraUniformBufferBinding;

        UniquePtr<SharedBuffer> sharedBuffer;
    };
}
