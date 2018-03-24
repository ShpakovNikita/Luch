#pragma once

#include <Husky/RefPtr.h>
#include <Husky/BaseObject.h>
#include <Husky/SceneV1/Forwards.h>

namespace Husky::SceneV1
{
    class Accessor : public BaseObject
    {
    public:

    private:
        RefPtr<Buffer> buffer;
        int32 bufferOffset = 0;

    };
}
