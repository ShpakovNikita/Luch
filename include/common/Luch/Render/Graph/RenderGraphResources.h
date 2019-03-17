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
        RenderMutableResource(std::nullptr_t) {};

        operator bool() const
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

        friend bool operator==(const RenderMutableResource& left, std::nullptr_t)
        {
            return left.handle == 0;
        }

        friend bool operator==(std::nullptr_t, const RenderMutableResource& right)
        {
            return 0 == right.handle;
        }
    private:
        explicit RenderMutableResource(uint32 aHandle) noexcept
            : handle(aHandle)
        {
        }

        RenderMutableResource(RenderResource resource) noexcept;

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
        RenderResource(std::nullptr_t) {};
        RenderResource(RenderMutableResource mutableResource) noexcept
            : handle(mutableResource.handle)
        {
        }

        operator bool() const
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

        friend bool operator==(const RenderResource& left, std::nullptr_t)
        {
            return left.handle == 0;
        }

        friend bool operator==(std::nullptr_t, const RenderResource& right)
        {
            return 0 == right.handle;
        }
    private:
        explicit RenderResource(uint32 aHandle) noexcept;
        uint32 handle = 0;
    };

    inline RenderMutableResource::RenderMutableResource(RenderResource resource) noexcept
        : handle(resource.handle)
    {
    }
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
