#pragma once
#if LUCH_USE_VULKAN
#include <Luch/Graphics/GraphicsResult.h>
#include <Luch/Vulkan.h>
#endif

namespace Luch
{
    template<typename Result, typename Value>
    class ResultValue
    {
    public:
        ResultValue(Result aResult, const Value& aValue)
            : result(aResult)
            , value(aValue)
        {
        }

        ResultValue(Result aResult, Value&& aValue)
            : result(aResult)
            , value(std::forward<Value>(aValue))
        {
        }

        ResultValue(Result aResult)
            : result(aResult)
            , value()
        {
        }

        ResultValue(ResultValue&& other)
            : result(std::move(other.result))
            , value(std::move(other.value))
        {
        }

        #if LUCH_USE_VULKAN
        ResultValue(vk::Result result)
            : ResultValue(ToGraphicsResult(result)) {}
        ResultValue(vk::Result result, Value val)
            : ResultValue(ToGraphicsResult(result), std::forward<Value>(val)) {}
        #endif


        ~ResultValue() = default;

        Result result;
        Value value;

    private:
    #if LUCH_USE_VULKAN
        Graphics::GraphicsResult ToGraphicsResult(vk::Result vkResult)
        {
            using namespace Graphics;
            // todo: support other result codes
            switch (vkResult)
            {
            case vk::Result::eSuccess:
                return GraphicsResult::Success;
            default:
                return GraphicsResult::UnknownError;
            }
        }
    #endif
    };
}
