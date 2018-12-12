#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Graphics/GraphicsDevice.h>

namespace Luch::Render::Graph
{
    RenderGraphResourceManager::RenderGraphResourceManager(GraphicsDevice* aDevice)
        : device(aDevice)
    {
    }

    RenderMutableResource RenderGraphResourceManager::ImportAttachment(RefPtr<Texture> texture)
    {
        auto handle = GetNextHandle();
        importedAttachments[handle] = texture;
        return handle;
    }

    RenderMutableResource RenderGraphResourceManager::CreateAttachment(int32 width, int32 height, Format format)
    {
        auto handle = GetNextHandle();
        pendingAttachments[handle] = AttachmentCreateInfo { width, height, format };
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

    bool RenderGraphResourceManager::Build()
    {
        for(const auto& [handle, info] : pendingAttachments)
        {
            TextureCreateInfo createInfo;

            bool hasDepthOrStencil = FormatHasDepth(info.format) || FormatHasStencil(info.format);
            createInfo.width = info.width;
            createInfo.height = info.height;
            createInfo.format = info.format;
            createInfo.storageMode = ResourceStorageMode::DeviceLocal;
            createInfo.usage = TextureUsageFlags::ShaderRead;
            createInfo.usage |= hasDepthOrStencil 
                ? TextureUsageFlags::DepthStencilAttachment
                : TextureUsageFlags::ColorAttachment;

            auto [createTextureResult, createdTexture] = device->CreateTexture(createInfo);
            if(createTextureResult != GraphicsResult::Success)
            {
                return false;
            }

            createdTextures[handle] = createdTexture;
        }

        pendingAttachments.clear();

        return true;
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
            auto it = createdTextures.find(handle);
            if(it != createdTextures.end())
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
