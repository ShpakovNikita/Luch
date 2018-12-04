#include <Luch/Render/Graph/RenderGraphResources.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    RenderMutableResource::RenderMutableResource(uint32 aHandle) noexcept
        : handle(aHandle)
    {
    }

    RenderMutableResource::RenderMutableResource(RenderResource resource) noexcept
        : handle(resource.handle)
    {
    }

    RenderResource::RenderResource(RenderMutableResource mutableResource) noexcept
        : handle(mutableResource.handle)
    {
    }

    RenderResource::RenderResource(uint32 aHandle) noexcept : handle(aHandle) {}
}
