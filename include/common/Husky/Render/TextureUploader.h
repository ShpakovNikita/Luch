#pragma once

#include <Husky/RefPtr.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/CopyCommandList.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/ResultValue.h>

namespace Husky::Render
{
    struct TextureUploadIntermediateResult
    {
        RefPtr<Graphics::Texture> texture;
        RefPtr<Graphics::Buffer> stagingBuffer;
        Graphics::BufferToTextureCopy copy;
    };

    struct TextureUploaderResult
    {
        RefPtrVector<Graphics::Buffer> stagingBuffers;
        RefPtrVector<Graphics::CopyCommandList> commandLists;
    };

    class TextureUploader
    {
    public:
        TextureUploader(
            Graphics::GraphicsDevice* device,
            Graphics::CommandPool* commandPool);

        ResultValue<bool, TextureUploaderResult> UploadTextures(const Vector<SceneV1::Texture*>& textures);
    private:
        ResultValue<bool, TextureUploadIntermediateResult> UploadTexture(SceneV1::Texture* texture);
        ResultValue<bool, RefPtr<Graphics::Sampler>> CreateSampler(SceneV1::Texture* texture);

        Graphics::GraphicsDevice* device;
        Graphics::CommandPool* commandPool;
    };
}
