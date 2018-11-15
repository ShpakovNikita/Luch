#pragma once

#include <Luch/Types.h>

namespace Luch::SceneV1
{
    struct BufferSource
    {
        String root;
        String filename;
        int32 byteLength = 0;
    };
}
