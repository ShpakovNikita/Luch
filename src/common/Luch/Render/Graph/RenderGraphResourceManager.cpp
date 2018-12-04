#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Graphics/Texture.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    RenderGraphResourceManager::RenderGraphResourceManager(GraphicsDevice* aDevice)
        : device(aDevice)
    {
    }

    RenderMutableResource RenderGraphResourceManager::CreateRenderTarget(const RenderTargetInfo& info)
    {
        auto handle = GetNextHandle();
        pendingRenderTargets[handle] = info;
        return handle;
    }

    RenderMutableResource RenderGraphResourceManager::ImportRenderTarget(const RefPtr<Texture>& texture)
    {
        auto handle = GetNextHandle();
        importedRenderTargets[handle] = texture;
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

    void RenderGraphResourceManager::MarkUnused(const Vector<RenderResource>& unusedResources)
    {
        for(auto unusedResource : unusedResources)
        {
            pendingRenderTargets.erase(RenderMutableResource{ unusedResource });
            importedRenderTargets.erase(RenderMutableResource{ unusedResource });
            modifiedResources.erase(RenderMutableResource{ unusedResource });
        }
    }

    bool RenderGraphResourceManager::Build()
    {
        for(const auto& [handle, info] : pendingRenderTargets)
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

        pendingRenderTargets.clear();

        return true;
    }

    void RenderGraphResourceManager::Reset()
    {
        createdTextures.clear();
        pendingRenderTargets.clear();
        importedRenderTargets.clear();
        modifiedResources.clear();
    }

    RefPtr<Texture> RenderGraphResourceManager::GetTexture(RenderResource textureHandle)
    {
        {
            auto it = modifiedResources.find(textureHandle);
            if(it != modifiedResources.end())
            {
                textureHandle = it->second;
            }
        }

        {
            auto it = createdTextures.find(textureHandle);
            if(it != createdTextures.end())
            {
                return it->second;
            }
        }

        {
            auto it = importedRenderTargets.find(textureHandle);
            if(it != importedRenderTargets.end())
            {
                return it->second;
            }
        }

        return nullptr;
    }

    RenderMutableResource RenderGraphResourceManager::GetNextHandle()
    {
        auto handle = RenderMutableResource{ nextHandleValue };
        nextHandleValue++;
        return handle;
    }
}
