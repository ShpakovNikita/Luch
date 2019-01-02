#include <Luch/Render/Graph/RenderGraphResourcePool.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/Texture.h>

#include <algorithm>

namespace Luch::Render::Graph
{
    RenderGraphResourcePool::RenderGraphResourcePool(GraphicsDevice* aDevice)
        : device(aDevice)
    {
    }

    void RenderGraphResourcePool::Tick()
    {
        ticksElapsed++;
        // TODO discard textures
    }

    GraphicsResultRefPtr<Texture> RenderGraphResourcePool::AcquireTexture(const TextureCreateInfo& ci)
    {
        auto& matchingEntries = textureEntries[ci];

        if(!matchingEntries.empty())
        {
            auto it = std::min_element(
                matchingEntries.begin(),
                matchingEntries.end(),
                [](auto left, auto right) { return left.addedOnTick < right.addedOnTick; });

            auto texture = it->texture;
            matchingEntries.erase(it);
            return { GraphicsResult::Success, texture };
        }
        else
        {
            return device->CreateTexture(ci); 
        }
    }

    void RenderGraphResourcePool::ReturnTexture(RefPtr<Texture> texture)
    {
        const auto& ci = texture->GetCreateInfo();

        auto& matchingEntries = textureEntries[ci];

        auto it = std::find_if(
            matchingEntries.begin(),
            matchingEntries.end(),
            [&texture](auto entry) { return entry.texture == texture; });
        LUCH_ASSERT(it == matchingEntries.end());

        TexturePoolEntry newEntry;
        newEntry.texture = texture;
        newEntry.addedOnTick = ticksElapsed;

        matchingEntries.push_back(newEntry);
    }
}
