#pragma once

#include <Husky/BaseObject.h>
#include <Husky/VectorTypes.h>

namespace Husky::SceneV1
{
    enum class LightType : int32
    {
        Point = 0,
        Spot = 1,
        Directional = 2,
        Ambient = 3
    };

    class Light : public BaseObject
    {
    public:
        Light() = default;

        LightType GetType() const { return type; }
        void SetType(LightType aType) { type = aType; }

        const Optional<Vec3>& GetColor() const { return color; }
        void SetColor(const Vec3& aColor) { color = aColor; }

        const Optional<Vec3>& GetDirection() const { return direction; }
        void SetDirection(const Vec3& aDirection) { direction = aDirection; }

        const Optional<float32>& GetSpotlightAngle() const { return spotlightAngle; }
        void SetSpotlightAngle(const Optional<float32>& aSpotlightAngle) { spotlightAngle = aSpotlightAngle; }

        const Optional<float32>& GetRange() const { return range; }
        void SetRange(const Optional<float32>& aRange) { range = aRange; }

        const float32& GetIntensity() const { return intensity; }
        void SetIntensity(float32 aIntensity) { intensity = aIntensity; }

        bool IsEnabled() const { return enabled; }
        void SetEnabled(bool aEnabled) { enabled = aEnabled; }

        const String& GetName() const { return name; }
        void SetName(const String& aName) { name = aName; }

        int32 GetIndex() const { return index; }
        void SetIndex(int32 aIndex) { index = aIndex; }
    private:
        LightType type;
        Optional<Vec3> color = Vec3{ 1.0, 1.0, 1.0 };
        Optional<Vec3> direction;
        Optional<float32> spotlightAngle = 0;
        Optional<float32> range = 0;
        bool enabled = false;
        float32 intensity = 1.0;
        int32 index = 0;
        String name;
    };
}
