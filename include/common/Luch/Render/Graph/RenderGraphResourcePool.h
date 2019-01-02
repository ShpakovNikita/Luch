#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Graphics/Size2.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/GraphicsResultValue.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    class RenderGraphResourcePool
    {
        static constexpr int32 DefaultTTL = 3;

        struct TexturePoolEntry
        {
            RefPtr<Texture> texture;
            int32 ttl = DefaultTTL;
            int32 addedOnTick = 0;
        };

        using TextureEntries = UnorderedMap<TextureCreateInfo, Vector<TexturePoolEntry>>;
    public:
        RenderGraphResourcePool(GraphicsDevice* device);

        void Tick();

        GraphicsResultRefPtr<Texture> AcquireTexture(const TextureCreateInfo& ci);
        void ReturnTexture(RefPtr<Texture> texture);
    private:
        GraphicsDevice* device = nullptr;
        int32 ticksElapsed = 0;
        TextureEntries textureEntries;
    };
}
