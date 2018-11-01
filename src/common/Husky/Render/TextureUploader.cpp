#include <Husky/Render/TextureUploader.h>
#include <Husky/SceneV1/Texture.h>
#include <Husky/SceneV1/Sampler.h>
#include <Husky/SceneV1/Image.h>
#include <Husky/Graphics/GraphicsDevice.h>
#include <Husky/Graphics/TextureCreateInfo.h>
#include <Husky/Graphics/BufferCreateInfo.h>
#include <Husky/Graphics/CommandPool.h>
#include <Husky/Graphics/CopyCommandList.h>
#include <Husky/Render/FormatUtils.h>

namespace Husky::Render
{
    TextureUploader::TextureUploader(
        Graphics::GraphicsDevice* aDevice,
        Graphics::CommandPool* aCommandPool)
        : device(aDevice)
        , commandPool(aCommandPool)
    {
    }

    ResultValue<bool, TextureUploaderResult> TextureUploader::UploadTextures(const Vector<SceneV1::Texture*>& textures)
    {
        TextureUploaderResult result;

        auto [allocateCommandListResult, commandList] = commandPool->AllocateCopyCommandList();
        if (allocateCommandListResult != Graphics::GraphicsResult::Success)
        {
            return { false };
        }

        Vector<TextureUploadIntermediateResult> intermediateResults;
        intermediateResults.reserve(textures.size());
        result.stagingBuffers.reserve(textures.size());

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

            result.stagingBuffers.push_back(intermediateResult.stagingBuffer);
            intermediateResults.push_back(intermediateResult);
        }

        commandList->Begin();

        for (const auto& intermediateResult : intermediateResults)
        {
            commandList->CopyBufferToTexture(
                intermediateResult.stagingBuffer,
                intermediateResult.texture,
                intermediateResult.copy);
        }

        commandList->End();

        result.commandLists = { commandList };

        return { true, result };
    }

    ResultValue<bool, TextureUploadIntermediateResult> TextureUploader::UploadTexture(SceneV1::Texture* texture)
    {
        TextureUploadIntermediateResult result;

        SceneV1::Image* hostImage = texture->GetHostImage();

        Graphics::TextureCreateInfo textureCreateInfo;
        textureCreateInfo.textureType = Graphics::TextureType::Texture2D;
        textureCreateInfo.format = texture->IsSRGB() ? FormatUtils::GetSRGBFormat(hostImage->GetFormat()) : hostImage->GetFormat();
        textureCreateInfo.width = hostImage->GetWidth();
        textureCreateInfo.height = hostImage->GetHeight();
        textureCreateInfo.usage = Graphics::TextureUsageFlags::ShaderRead | Graphics::TextureUsageFlags::TransferDestination;
        textureCreateInfo.storageMode = Graphics::ResourceStorageMode::DeviceLocal;

        auto[createTextureResult, createdTexture] = device->CreateTexture(textureCreateInfo);
        if (createTextureResult != Graphics::GraphicsResult::Success)
        {
            return { false };
        }

        auto &hostImageBuffer = hostImage->GetBuffer();

        Graphics::BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = hostImageBuffer.size();
        bufferCreateInfo.usage = Graphics::BufferUsageFlags::TransferSource;
        bufferCreateInfo.storageMode = Graphics::ResourceStorageMode::Shared;

        auto[createStagingBufferResult, stagingBuffer] = device->CreateBuffer(bufferCreateInfo, hostImageBuffer.data());

        if (createStagingBufferResult != Graphics::GraphicsResult::Success)
        {
            return { false };
        }

        auto [createSamplerResult, createdSampler] = CreateSampler(texture);
        if (!createSamplerResult)
        {
            return { false };
        }

        texture->SetDeviceTexture(createdTexture);
        texture->GetSampler()->SetDeviceSampler(createdSampler);

        result.texture = createdTexture;
        result.stagingBuffer = stagingBuffer;

        int32 width = hostImage->GetWidth();
        int32 height = hostImage->GetHeight();

        result.copy.bytesPerRow = hostImage->GetBytesPerPixel() * width;
        result.copy.bytesPerImage = result.copy.bytesPerRow * height;
        result.copy.sourceSize = { width, height };

        return { true, result };
    }

    ResultValue<bool, RefPtr<Graphics::Sampler>> TextureUploader::CreateSampler(SceneV1::Texture* texture)
    {
        const auto& samplerCI = texture->GetSampler()->GetSamplerDescription();
        auto [createSamplerResult, createdSampler] = device->CreateSampler(samplerCI);
        if (createSamplerResult != Graphics::GraphicsResult::Success)
        {
            return { false };
        }

        return { true, createdSampler };
    }
}
