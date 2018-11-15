#pragma once


#include <Luch/Render/Deferred/DeferredForwards.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/Attachment.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/RefPtr.h>

namespace Luch::Render::Deferred::ShadowMapping
{
    using namespace Graphics;

    // I'm pretty sure I'm going to need base color texture
    // if I want to shadow map objects with alpha mask
    struct ShadowMappingPassResources
    {
        UniquePtr<SharedBuffer> sharedBuffer;
        DescriptorSetBinding cameraUniformBufferBinding;
        DescriptorSetBinding meshUniformBufferBinding;

        DepthStencilAttachment depthStencilAttachmentTemplate;

        RefPtr<DescriptorPool> descriptorPool;

        RefPtr<DescriptorSetLayout> meshBufferSetLayout;
        RefPtr<DescriptorSetLayout> cameraBufferSetLayout;

        RefPtr<CommandPool> commandPool;

        RefPtr<PipelineLayout> pipelineLayout;

        RefPtr<PipelineState> pipelineState;
    };
}
