#include <Husky/Render/TextureUploader.h>
#include <Husky/SceneV1/Texture.h>
#include <Husky/SceneV1/Sampler.h>
#include <Husky/SceneV1/Image.h>
#include <Husky/Vulkan/CommandPool.h>

namespace Husky::Render
{
    TextureUploader::TextureUploader(
        Vulkan::GraphicsDevice* aDevice,
        Vulkan::CommandPool* aCommandPool)
        : device(aDevice)
        , commandPool(aCommandPool)
    {
    }

    ResultValue<bool, TextureUploaderResult> TextureUploader::UploadTextures(const Vector<SceneV1::Texture*>& textures)
    {
        TextureUploaderResult result;

        auto [allocateBuffersResult, buffer] = commandPool->AllocateCommandBuffer(Vulkan::CommandBufferLevel::Primary);
        if (allocateBuffersResult != vk::Result::eSuccess)
        {
            return { false };
        }

        Vector<TextureUploadIntermediateResult> intermediateResults;
        intermediateResults.reserve(textures.size());

        for (SceneV1::Texture* texture : textures)
        {
            if (texture == nullptr)
            {
                // It's easier to add textures like this without checking for null
                continue;
            }

            auto [succeeded, intermediateResult] = UploadTexture(texture);

            // TODO
            HUSKY_ASSERT(succeeded);

            intermediateResults.push_back(intermediateResult);
        }

        Vulkan::PipelineBarrier beforeBarrier;
        beforeBarrier
            .ToStage(vk::PipelineStageFlagBits::eTransfer);

        Vulkan::PipelineBarrier afterBarrier;
        afterBarrier
            .FromStage(vk::PipelineStageFlagBits::eTransfer)
            .ToStage(vk::PipelineStageFlagBits::eFragmentShader);

        for (const auto& intermediateResult : intermediateResults)
        {
            beforeBarrier.WithImageBarrier(&intermediateResult.transferImageBarrier);
            afterBarrier.WithImageBarrier(&intermediateResult.readImageBarrier);
        }

        buffer
            ->Begin()
            ->PipelineBarrier(beforeBarrier);

        for (const auto& intermediateResult : intermediateResults)
        {
            buffer->CopyBufferToImage(
                intermediateResult.stagingBuffer,
                intermediateResult.image,
                vk::ImageLayout::eTransferDstOptimal,
                intermediateResult.copy
            );
        }

        buffer
            ->PipelineBarrier(afterBarrier)
            ->End();

        result.commandBuffers = { buffer };

        return { true, result };
    }

    ResultValue<bool, TextureUploadIntermediateResult> TextureUploader::UploadTexture(SceneV1::Texture* texture)
    {
        TextureUploadIntermediateResult result;

        SceneV1::Image* hostImage = texture->GetHostImage();

        int32 hostImageComponentCount = hostImage->GetComponentCount();

        vk::Format format;

        switch (hostImageComponentCount)
        {
        case 1:
            format = vk::Format::eR8Unorm;
            break;
        case 2:
            format = vk::Format::eR8G8Unorm;
            break;
        case 3:
            format = vk::Format::eR8G8B8Unorm;
            break;
        case 4:
            format = vk::Format::eR8G8B8A8Unorm;
            break;
        default:
            HUSKY_ASSERT(false);
        }

        vk::ImageCreateInfo imageCI;
        imageCI.imageType = vk::ImageType::e2D;
        imageCI.format = format;
        imageCI.extent.width = hostImage->GetWidth();
        imageCI.extent.height = hostImage->GetHeight();
        imageCI.extent.depth = 1;
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = 1;
        imageCI.samples = vk::SampleCountFlagBits::e1;
        imageCI.tiling = vk::ImageTiling::eOptimal;
        imageCI.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
        imageCI.sharingMode = vk::SharingMode::eExclusive;

        auto[createImageResult, createdImage] = device->CreateImage(imageCI);
        if (createImageResult != vk::Result::eSuccess)
        {
            return { false };
        }

        texture->SetDeviceImage(createdImage);

        const auto &queueIndices = device->GetQueueIndices();
        int32 stagingBufferSize = createdImage->GetMemoryRequirements().size;

        auto &hostImageBuffer = hostImage->GetBuffer();

        // TODO think about separate queue for transfer
        auto[createStagingBufferResult, stagingBuffer] = device->CreateBuffer(
            hostImageBuffer.size(),
            queueIndices->graphicsQueueFamilyIndex,
            vk::BufferUsageFlagBits::eTransferSrc,
            true);

        if (createStagingBufferResult != vk::Result::eSuccess)
        {
            return { false };
        }

        auto[mapResult, mappedMemory] = stagingBuffer->MapMemory(hostImageBuffer.size(), 0);
        if (mapResult != vk::Result::eSuccess)
        {
            return { false };
        }

        memcpy(mappedMemory, hostImageBuffer.data(), hostImageBuffer.size());

        stagingBuffer->UnmapMemory();

        auto [createImageViewResult, createdImageView] = device->CreateImageView(createdImage);
        if (createImageViewResult != vk::Result::eSuccess)
        {
            return { false };
        }

        auto [createSamplerResult, createdSampler] = CreateSampler(texture);
        if (!createSamplerResult)
        {
            return { false };
        }

        texture->GetSampler()->SetDeviceSampler(createdSampler);

        result.image = createdImage;
        result.stagingBuffer = stagingBuffer;

        result.copy.imageExtent = vk::Extent3D{ (uint32)hostImage->GetWidth(), (uint32)hostImage->GetHeight(), 1 };

        result.transferImageBarrier
            .ForImage(createdImage)
            .FromLayout(vk::ImageLayout::eUndefined)
            .ToLayout(vk::ImageLayout::eTransferDstOptimal)
            .ToAccess(vk::AccessFlagBits::eTransferWrite);

        result.readImageBarrier
            .ForImage(createdImage)
            .FromLayout(vk::ImageLayout::eTransferDstOptimal)
            .ToLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .FromAccess(vk::AccessFlagBits::eTransferWrite)
            .ToAccess(vk::AccessFlagBits::eShaderRead);

        return { true, result };
    }

    ResultValue<bool, RefPtr<Vulkan::Sampler>> TextureUploader::CreateSampler(SceneV1::Texture* texture)
    {
        const auto& samplerCI = texture->GetSampler()->GetSamplerDescription();
        auto [createSamplerResult, createdSampler] = device->CreateSampler(samplerCI);
        if (createSamplerResult != vk::Result::eSuccess)
        {
            return { false };
        }

        return { true, createdSampler };
    }
}
