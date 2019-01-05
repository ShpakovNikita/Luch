#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/SceneV1/SceneV1Forwards.h>

namespace Luch
{
    class MouseNodeController
    {
    public:
        const RefPtr<SceneV1::Node>& GetNode() { return node; }
        void SetNode(RefPtr<SceneV1::Node> aNode) { node = std::move(aNode); }

        void Add(float32 dx, float32 dy);

        void Tick();
    private:
        RefPtr<SceneV1::Node> node;

        Array<float32, 2> speed = { 0.01f, 0.01f }; // degrees / px
        Array<float32, 2> ds = {};
    };
}
