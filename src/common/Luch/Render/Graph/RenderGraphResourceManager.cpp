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

    RenderMutableResource RenderGraphResourceManager::ImportAttachment(RefPtr<Texture> texture)
    {
        auto handle = GetNextHandle();
        importedAttachments[handle] = texture;
        return handle;
    }

    RenderMutableResource RenderGraphResourceManager::ImportAttachmentDeferred()
    {
        auto handle = GetNextHandle();
        importedAttachments[handle] = nullptr;
        return handle;
    }

    RenderMutableResource RenderGraphResourceManager::CreateAttachment(Size2i size, Format format)
    {
        auto handle = GetNextHandle();
        LUCH_ASSERT(size.width > 0 && size.height > 0);
        pendingAttachments[handle] = AttachmentCreateInfo { size, format };
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
        for(const auto& [handle, info] : pendingAttachments)
        {
            TextureCreateInfo createInfo;

            bool hasDepthOrStencil = FormatHasDepth(info.format) || FormatHasStencil(info.format);
            createInfo.width = info.size.width;
            createInfo.height = info.size.height;
            createInfo.format = info.format;
            createInfo.storageMode = ResourceStorageMode::DeviceLocal;
            createInfo.usage = TextureUsageFlags::ShaderRead;
            createInfo.usage |= hasDepthOrStencil 
                ? TextureUsageFlags::DepthStencilAttachment
                : TextureUsageFlags::ColorAttachment;

            auto [acquireTextureResult, acquiredTexture] = pool->AcquireTexture(createInfo);
            if(acquireTextureResult != GraphicsResult::Success)
            {
                return false;
            }

            acquiredTextures[handle] = acquiredTexture;
        }

        pendingAttachments.clear();

        for(const auto& [handle, ci] : pendingBuffers)
        {
            auto [acquireBufferResult, acquiredBuffer] = pool->AcquireBuffer(ci);
            if(acquireBufferResult != GraphicsResult::Success)
            {
                return false;
            }

            acquiredBuffers[handle] = acquiredBuffer;
        }

        return true;
    }

    void RenderGraphResourceManager::ProvideDeferredAttachment(RenderMutableResource handle, RefPtr<Texture> texture)
    {
        LUCH_ASSERT(handle);
        LUCH_ASSERT(importedAttachments[handle] == nullptr);
        importedAttachments[handle] = std::move(texture);
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
            auto it = importedAttachments.find(handle);
            if(it != importedAttachments.end())
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
