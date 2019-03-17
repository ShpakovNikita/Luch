#include <Luch/Render/Graph/RenderGraphResourcePool.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/Texture.h>
#include <Luch/Graphics/Buffer.h>

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
        // TODO discard textures and buffers
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
            LUCH_ASSERT(texture != nullptr);
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

    GraphicsResultRefPtr<Buffer> RenderGraphResourcePool::AcquireBuffer(const BufferCreateInfo& ci)
    {
        auto& matchingEntries = bufferEntries[ci];

        if(!matchingEntries.empty())
        {
            auto it = std::min_element(
                matchingEntries.begin(),
                matchingEntries.end(),
                [](auto left, auto right) { return left.addedOnTick < right.addedOnTick; });

            auto buffer = it->buffer;
            matchingEntries.erase(it);
            LUCH_ASSERT(buffer != nullptr);
            return { GraphicsResult::Success, buffer };
        }
        else
        {
            return device->CreateBuffer(ci); 
        }
    }

    void RenderGraphResourcePool::ReturnBuffer(RefPtr<Buffer> buffer)
    {
        const auto& ci = buffer->GetCreateInfo();

        auto& matchingEntries = bufferEntries[ci];

        auto it = std::find_if(
            matchingEntries.begin(),
            matchingEntries.end(),
            [&buffer](auto entry) { return entry.buffer == buffer; });
        LUCH_ASSERT(it == matchingEntries.end());

        BufferPoolEntry newEntry;
        newEntry.buffer = buffer;
        newEntry.addedOnTick = ticksElapsed;

        matchingEntries.push_back(newEntry);
    }
}
