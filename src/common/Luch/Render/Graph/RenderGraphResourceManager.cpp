#include <Luch/Render/Graph/RenderGraphResourceManager.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

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

    RenderMutableResource RenderGraphResourceManager::CreateBuffer(BufferUsageFlags usageFlags)
    {
        auto handle = GetNextHandle();
        pendingBuffers[handle] = usageFlags;
        return handle;
    }

    RenderMutableResource RenderGraphResourceManager::ImportBuffer(const RefPtr<Buffer>& buffer)
    {
        auto handle = GetNextHandle();
        importedBuffers[handle] = buffer;
        return handle;
    }

    RenderMutableResource RenderGraphResourceManager::GetNextHandle()
    {
        auto handle = RenderMutableResource{ nextHandleValue };
        nextHandleValue++;
        return handle;
    }
}
