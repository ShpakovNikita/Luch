#pragma once

#include <Luch/RefPtr.h>
#include <Luch/BaseObject.h>
#include <Luch/SceneV1/Forwards.h>

namespace Luch::SceneV1
{
    class Accessor : public BaseObject
    {
    public:

    private:
        RefPtr<Buffer> buffer;
        int32 bufferOffset = 0;

    };
}
