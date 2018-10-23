#pragma once


#include <Husky/Render/Deferred/DeferredForwards.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/DescriptorSetBinding.h>
#include <Husky/Graphics/Attachment.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/RefPtr.h>

namespace Husky::Render::Deferred::ShadowMapping
{
    using namespace Graphics;

    // I'm pretty sure I'm going to need base color texture
    // if I want to shadow map objects with alpha mask
    struct ShadowMappingPassResources
    {
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
