#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphResourcePool.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Graphics/GraphicsDevice.h>

namespace Luch::Render::Graph
{
    RenderGraphResourceManager::RenderGraphResourceManager(
        GraphicsDevice* aDevice,
        RenderGraphResourcePool* aPool)
        : device(aDevice)
        , pool(aPool)
    {
    }

    RenderGraphResourceManager::~RenderGraphResourceManager()
    {
        for(auto& texture : acquiredTextures)
        {
            pool->ReturnTexture(texture.second);
        }
    }

    RenderMutableResource RenderGraphResourceManager::ImportTexture(RefPtr<Texture> texture)
    {
        auto handle = GetNextHandle();
        importedTextures[handle] = texture;
        return handle;
    }

    RenderMutableResource RenderGraphResourceManager::ImportTextureDeferred()
    {
        auto handle = GetNextHandle();
        importedTextures[handle] = nullptr;
        return handle;
    }

    RenderMutableResource RenderGraphResourceManager::CreateTexture(const TextureCreateInfo& createInfo)
    {
        auto handle = GetNextHandle();
        LUCH_ASSERT(createInfo.width > 0 && createInfo.height > 0);
        pendingTextures[handle] = createInfo;
        return handle;
    }

    RenderMutableResource RenderGraphResourceManager::ModifyResource(RenderMutableResource handle)
    {
        // kv - modified resource, original resource
        // Always map to original resource, so that we don't end up with modify chains
        // Then we can look up original resource in a single lookup

        auto nextHandle = GetNextHandle();
        auto it = modifiedResources.find(handle);
        if(it != modifiedResources.end())
        {
            modifiedResources[nextHandle] = it->second;
        }
        else
        {
            modifiedResources[nextHandle] = handle;
        }

        return nextHandle;
    }

    RenderMutableResource RenderGraphResourceManager::ImportBuffer(RefPtr<Buffer> buffer)
    {
        auto handle = GetNextHandle();
        importedBuffers[handle] = buffer;
        return handle;
    }

    RenderMutableResource RenderGraphResourceManager::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        auto handle = GetNextHandle();
        pendingBuffers[handle] = createInfo;
        return handle;
    }

    bool RenderGraphResourceManager::Build()
    {
        for(const auto& [handle, ci] : pendingTextures)
        {
            auto [acquireTextureResult, acquiredTexture] = pool->AcquireTexture(ci);
            if(acquireTextureResult != GraphicsResult::Success)
            {
                return false;
            }

            acquiredTextures[handle] = acquiredTexture;
        }

        pendingTextures.clear();

        for(const auto& [handle, ci] : pendingBuffers)
        {
            auto [acquireBufferResult, acquiredBuffer] = pool->AcquireBuffer(ci);
            if(acquireBufferResult != GraphicsResult::Success)
            {
                return false;
            }

            acquiredBuffers[handle] = acquiredBuffer;
        }

        pendingBuffers.clear();

        return true;
    }

    void RenderGraphResourceManager::ProvideDeferredTexture(RenderMutableResource handle, RefPtr<Texture> texture)
    {
        LUCH_ASSERT(handle);
        LUCH_ASSERT(importedTextures[handle] == nullptr);
        importedTextures[handle] = std::move(texture);
    }

    RefPtr<Texture> RenderGraphResourceManager::GetTexture(RenderResource handle)
    {
         {
            auto it = modifiedResources.find(handle);
            if(it != modifiedResources.end())
            {
                handle = it->second;
            }
         }

         {
            auto it = acquiredTextures.find(handle);
            if(it != acquiredTextures.end())
            {
                return it->second;
            }
         }

         {
            auto it = importedTextures.find(handle);
            if(it != importedTextures.end())
            {
                return it->second;
            }
         }

         return nullptr;
    }

    RefPtr<Buffer> RenderGraphResourceManager::GetBuffer(RenderResource handle)
    {
         {
            auto it = modifiedResources.find(handle);
            if(it != modifiedResources.end())
            {
                handle = it->second;
            }
         }

         {
            auto it = acquiredBuffers.find(handle);
            if(it != acquiredBuffers.end())
            {
                return it->second;
            }
         }

         {
            auto it = importedBuffers.find(handle);
            if(it != importedBuffers.end())
            {
                return it->second;
            }
         }

         return nullptr;
    }

    RefPtr<Texture> RenderGraphResourceManager::ReleaseTexture(RenderResource handle)
    {
        {
            auto it = modifiedResources.find(handle);
            if(it != modifiedResources.end())
            {
                handle = it->second;
            }
        }

        {
            auto it = acquiredTextures.find(handle);
            if(it != acquiredTextures.end())
            {
                auto texture = it->second;
                acquiredTextures.erase(it);
                return texture;
            }
        }

        {
            auto it = importedTextures.find(handle);
            if(it != importedTextures.end())
            {
                return it->second;
            }
        }

         return nullptr;
    }

    RefPtr<Buffer> RenderGraphResourceManager::ReleaseBuffer(RenderResource handle)
    {
        {
            auto it = modifiedResources.find(handle);
            if(it != modifiedResources.end())
            {
                handle = it->second;
            }
        }

        {
            auto it = acquiredBuffers.find(handle);
            if(it != acquiredBuffers.end())
            {
                auto buffer = it->second;
                acquiredBuffers.erase(it);
                return buffer;
            }
        }

        {
           auto it = importedBuffers.find(handle);
           if(it != importedBuffers.end())
           {
               return it->second;
           }
        }

         return nullptr;
    }

    RenderMutableResource RenderGraphResourceManager::GetNextHandle()
    {
        auto newHandle = RenderMutableResource { nextHandleValue };
        nextHandleValue++;
        return newHandle;
    }
}
