#pragma once

#include <Husky/Types.h>
#include <Husky/RefPtr.h>
#include <Husky/BaseObject.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::SceneV1
{
    struct BufferSource
    {
        String filename;
        int32 byteLength = 0;
    };
}
