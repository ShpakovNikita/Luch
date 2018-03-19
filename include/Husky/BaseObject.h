#pragma once

#include <atomic>
#include <Husky/Assert.h>
#include <Husky/Types.h>

namespace Husky
{
    class BaseObject
    {
    public:
        virtual ~BaseObject() {}
    protected:
        BaseObject()
            : refCount(1)
        {
        }

        BaseObject(const BaseObject& other) = default;
        BaseObject& operator=(const BaseObject& other) = default;

        BaseObject(BaseObject&& other)
        {

        }

        BaseObject& operator=(BaseObject&& other) = default;

        inline void AddReference() noexcept
        {
            ++refCount;
        }

        inline void RemoveReference() noexcept
        {
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
