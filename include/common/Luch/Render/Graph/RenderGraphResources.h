#pragma once

namespace Luch::Render::Graph
{

}
#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Types.h>
#include <Luch/Graphics/BufferUsageFlags.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/TextureUsageFlags.h>
#include <Luch/Graphics/Attachment.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    class RenderResourceManager;
    class RenderResource;

    class RenderMutableResource
    {
        friend class RenderResource;
        friend class RenderGraphResourceManager;
        friend struct std::hash<Luch::Render::Graph::RenderMutableResource>;
    public:
        RenderMutableResource() = default;
        RenderMutableResource(const RenderMutableResource& other) = default;
        RenderMutableResource& operator=(const RenderMutableResource& other) = default;

        operator bool()
        {
            return handle != 0;
        }

        friend bool operator==(const RenderMutableResource& left, const RenderMutableResource& right)
        {
            return left.handle == right.handle;
        }

        friend bool operator!=(const RenderMutableResource& left, const RenderMutableResource& right)
        {
            return left.handle != right.handle;
        }

        friend bool operator<(const RenderMutableResource& left, const RenderMutableResource& right)
        {
            return left.handle < right.handle;
        }
    private:
        explicit RenderMutableResource(uint32 aHandle) noexcept;
        explicit RenderMutableResource(RenderResource resource) noexcept;

        uint32 handle = 0;
    };

    class RenderResource
    {
        friend class RenderMutableResource;
        friend class RenderGraphResourceManager;
        friend struct std::hash<Luch::Render::Graph::RenderResource>;
    public:
        RenderResource() = default;
        RenderResource(const RenderResource& other) = default;
        RenderResource& operator=(const RenderResource& other) = default;
        RenderResource(RenderMutableResource mutableResource) noexcept;

        operator bool()
        {
            return handle != 0;
        }

        friend bool operator==(const RenderResource& left, const RenderResource& right)
        {
            return left.handle == right.handle;
        }

        friend bool operator!=(const RenderResource& left, const RenderResource& right)
        {
            return left.handle != right.handle;
        }

        friend bool operator==(const RenderMutableResource& left, const RenderResource& right)
        {
            return left.handle == right.handle;
        }

        friend bool operator==(const RenderResource& left, const RenderMutableResource& right)
        {
            return left.handle == right.handle;
        }

        friend bool operator!=(const RenderMutableResource& left, const RenderResource& right)
        {
            return left.handle != right.handle;
        }

        friend bool operator!=(const RenderResource& left, const RenderMutableResource& right)
        {
            return left.handle != right.handle;
        }

        friend bool operator<(const RenderResource& left, const RenderResource& right)
        {
            return left.handle < right.handle;
        }
    private:
        explicit RenderResource(uint32 aHandle) noexcept;
        uint32 handle = 0;
    };

}

namespace std
{
    template<>
    struct hash<Luch::Render::Graph::RenderResource>
    {
        size_t operator()(const Luch::Render::Graph::RenderResource& resource) const
        {
            return resource.handle;
        }
    };

    template<>
    struct hash<Luch::Render::Graph::RenderMutableResource>
    {
        size_t operator()(const Luch::Render::Graph::RenderMutableResource& resource) const
        {
            return resource.handle;
        }
    };
}