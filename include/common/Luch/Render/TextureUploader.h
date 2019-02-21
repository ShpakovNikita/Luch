#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/CopyCommandList.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/ResultValue.h>

namespace Luch::Render
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
        RefPtrVector<Graphics::CommandList> commandLists;
    };

    class TextureUploader
    {
    public:
        TextureUploader(
            Graphics::GraphicsDevice* device,
            Graphics::CommandPool* commandPool);

        ResultValue<bool, TextureUploaderResult> UploadTextures(
            const Vector<SceneV1::Texture*>& textures,
            bool generateMipmaps);
    private:
        ResultValue<bool, TextureUploadIntermediateResult> UploadTexture(
            SceneV1::Texture* texture,
            bool generateMipmaps);

        ResultValue<bool, RefPtr<Graphics::Sampler>> CreateSampler(SceneV1::Texture* texture);

        Graphics::GraphicsDevice* device;
        Graphics::CommandPool* commandPool;
    };
}
