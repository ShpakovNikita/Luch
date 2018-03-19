#pragma once

#include <Husky/UniquePtr.h>

namespace Husky
{
    class Stream;
}

namespace Husky::glTF
{
    struct glTF;

    class glTFParser
    {
    public:
        UniquePtr<glTF> ParseJSON(Stream* stream);
    };
}
