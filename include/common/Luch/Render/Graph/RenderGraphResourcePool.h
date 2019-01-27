#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Graphics/Size2.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/GraphicsResultValue.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    class RenderGraphResourcePool
    {
        static constexpr int32 DefaultTTL = 3;

        struct PoolEntry
        {
            RefPtr<Texture> texture;
            int32 ttl = DefaultTTL;
            int32 addedOnTick = 0;
        };

        struct TexturePoolEntry : PoolEntry
        {
            RefPtr<Texture> texture;
        };

        struct BufferPoolEntry : PoolEntry
        {
            RefPtr<Buffer> buffer;
        };

        using TextureEntries = UnorderedMap<TextureCreateInfo, Vector<TexturePoolEntry>>;
        using BufferEntries = UnorderedMap<BufferCreateInfo, Vector<BufferPoolEntry>>;
    public:
        RenderGraphResourcePool(GraphicsDevice* device);

        void Tick();

        GraphicsResultRefPtr<Texture> AcquireTexture(const TextureCreateInfo& ci);
        void ReturnTexture(RefPtr<Texture> texture);

        GraphicsResultRefPtr<Buffer> AcquireBuffer(const BufferCreateInfo& ci);
        void ReturnBuffer(RefPtr<Buffer> buffer);
    private:
        GraphicsDevice* device = nullptr;
        int32 ticksElapsed = 0;
        TextureEntries textureEntries;
        BufferEntries bufferEntries;
    };
}
