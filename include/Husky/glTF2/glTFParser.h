#pragma once

#include <Husky/SharedPtr.h>

namespace Husky
{
    class Stream;
}

namespace Husky::glTF
{
    struct glTFRoot;

    class glTFParser
    {
    public:
        SharedPtr<glTFRoot> ParseJSON(Stream* stream);
    };
}
