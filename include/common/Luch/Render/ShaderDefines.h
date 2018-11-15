#pragma once

#include <Luch/Types.h>

namespace Luch::Render
{
    template<typename T>
    struct ShaderDefines
    {
        UnorderedMap<String, Variant<int32, String>> defines;

        void AddFlag(T flag)
        {
            AddDefine(flag, "1");
        }

        void AddDefine(T define, const Variant<int32, String>& value)
        {
            LUCH_ASSERT(mapping != nullptr);

            if (define != T{})
            {
                defines[mapping->at(define)] = value;
            }
        }

        const UnorderedMap<T, String>* mapping = nullptr;
    };
}
