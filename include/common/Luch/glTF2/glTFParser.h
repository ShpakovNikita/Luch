#pragma once

#include <Luch/SharedPtr.h>

namespace Luch
{
    class Stream;
}

namespace Luch::glTF
{
    struct glTFRoot;

    class glTFParser
    {
    public:
        SharedPtr<glTFRoot> ParseJSON(Stream* stream);
    };
}
