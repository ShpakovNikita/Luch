#pragma once

#include <Husky/RefPtr.h>
#include <Husky/Vulkan/VulkanForwards.h>
#include <Husky/SceneV1/Forwards.h>
#include <Husky/Vulkan/CommandBuffer.h>

namespace Husky::Render
{
    struct TextureUploadIntermediateResult
    {
        RefPtr<Vulkan::Image> image;
        RefPtr<Vulkan::DeviceBuffer> stagingBuffer;
        Vulkan::ImageMemoryBarrier transferImageBarrier;
        Vulkan::ImageMemoryBarrier readImageBarrier;
        Vulkan::BufferToImageCopy copy;
    };

    struct TextureUploaderResult
    {
        RefPtrVector<Vulkan::DeviceBuffer> stagingBuffers;
        RefPtrVector<Vulkan::CommandBuffer> commandBuffers;
    };

    class TextureUploader
    {
    public:
        TextureUploader(
            Vulkan::GraphicsDevice* device,
            Vulkan::CommandPool* commandPool);

        ResultValue<bool, TextureUploaderResult> UploadTextures(const Vector<SceneV1::Texture*>& textures);
    private:
        ResultValue<bool, TextureUploadIntermediateResult> UploadTexture(SceneV1::Texture* texture);
        ResultValue<bool, RefPtr<Vulkan::Sampler>> CreateSampler(SceneV1::Texture* texture);

        Vulkan::GraphicsDevice* device;
        Vulkan::CommandPool* commandPool;
    };
}
