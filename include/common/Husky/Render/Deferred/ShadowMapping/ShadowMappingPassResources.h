#pragma once


#include <Husky/Render/Deferred/DeferredForwards.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/RefPtr.h>

namespace Husky::Render::Deferred::ShadowMapping
{
    using namespace Graphics;

    // I'm pretty sure I'm going to need base color texture
    // if I want to shadow map objects with alpha mask
    struct ShadowMappingPassResources
    {
        // outer vector stores maps of depth textures for each swapchain frame
        // unordered map stores vectors of depth textures for each light
        // inner vector stores either one (for directional and spot lights) or six (for point lights) depth textures
        Vector<UnorderedMap<SceneV1::Light*, Vector<RefPtr<Texture>>>> shadowMaps;

        DescriptorSetBinding cameraUniformBufferBinding;

        DescriptorSetBinding meshUniformBufferBinding;

        DepthStencilAttachment depthStencilAttachmentTemplate;

        RefPtr<DescriptorSetLayout> meshBufferSetLayout;
        RefPtr<DescriptorSetLayout> cameraBufferSetLayout;

        RefPtr<PipelineState> directionalLightPipelineState;
        RefPtr<PipelineState> pointLightPipelineState;
        RefPtr<PipelineState> spotlightPipelineState;
    };
}
