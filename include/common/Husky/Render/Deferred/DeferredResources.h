#pragma once

#include <Husky/Types.h>
#include <Husky/RefPtr.h>
#include <Husky/UniquePtr.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/DescriptorSetBinding.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;

    struct DeferredResources
    {
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<DescriptorSetLayout> cameraBufferDescriptorSetLayout;
        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        DescriptorSetBinding cameraUniformBufferBinding;
    };
}
