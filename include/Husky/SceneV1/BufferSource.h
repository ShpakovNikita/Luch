#pragma once

#include <Husky/Types.h>

namespace Husky::SceneV1
{
    struct BufferSource
    {
        String root;
        String filename;
        int32 byteLength = 0;
    };
}
