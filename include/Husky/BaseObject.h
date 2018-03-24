#pragma once

#include <atomic>
#include <Husky/Assert.h>
#include <Husky/Types.h>

namespace Husky
{
    class BaseObject
    {
        template<typename T>
        friend class RefPtr;
    public:
        virtual ~BaseObject() {}
    protected:
        BaseObject()
            : refCount(1)
        {
        }

        BaseObject(const BaseObject& other) = delete;
        BaseObject& operator=(const BaseObject& other) = delete;

        BaseObject(BaseObject&& other)
            : refCount(other.refCount.load())
        {
            other.refCount.store(0);
        }

        BaseObject& operator=(BaseObject&& other)
        {
            refCount = other.refCount.load();
            other.refCount.store(0);
        }

        inline void AddReference() noexcept
        {
            ++refCount;
        }

        inline void RemoveReference() noexcept
        {
            // TODO
            auto newRefCount = --refCount;
            HUSKY_ASSERT(newRefCount >= 0, "Negative refcount");
            if (newRefCount == 0)
            {
                delete this;
            }
        }
    private:
        std::atomic<int32> refCount;
    };
}
