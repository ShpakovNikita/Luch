#pragma once

#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Size2.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>

namespace Luch::Render
{
    using namespace Graphics;

    struct LightPersistentResources
    {
        DescriptorSetBinding lightingParamsBinding;
        DescriptorSetBinding lightsBufferBinding;
        RefPtr<DescriptorSetLayout> lightsBufferDescriptorSetLayout;
    };
}