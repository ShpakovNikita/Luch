#pragma once

namespace Husky
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

        ~ResultValue() = default;

        Result result;
        Value value;
    };

}
