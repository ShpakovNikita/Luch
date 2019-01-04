#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/SceneV1/SceneV1Forwards.h>

namespace Luch
{
    class WASDNodeController
    {
    public:
        static constexpr int32 XAxis = 0;
        static constexpr int32 YAxis = 1;
        static constexpr int32 ZAxis = 2;

        static constexpr int32 Negative = 0;
        static constexpr int32 Positive = 1;

        const RefPtr<SceneV1::Node>& GetNode() { return node; }
        void SetNode(RefPtr<SceneV1::Node> aNode) { node = std::move(aNode); }

        float32 GetSpeedd(int32 axis, int32 direction)
        {
            return speed[axis][direction];
        }

        void SetSpeed(int32 axis, int32 direction, float32 newSpeed)
        {
            speed[axis][direction] = newSpeed;
        }

        bool IsMoving(int32 axis, int32 direction)
        {
            return moving[axis][direction];
        }

        void SetMoving(int32 axis, int32 direction, bool value)
        {
            moving[axis][direction] = value;
        } 

        void Tick(float32 elapsedTime);
    private:
        RefPtr<SceneV1::Node> node;

        Array<Array<float32, 2>, 3> speed = {};
        Array<Array<bool, 2>, 3> moving = {};
    };
}
