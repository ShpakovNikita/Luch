#pragma once

#include <Luch/BaseObject.h>
#include <Luch/VectorTypes.h>

namespace Luch::SceneV1
{
    enum class LightType : int32
    {
        Point = 0,
        Spot = 1,
        Directional = 2,
    };

    class Light : public BaseObject
    {
    public:
        Light() = default;

        LightType GetType() const { return type; }
        void SetType(LightType aType) { type = aType; }

        const Optional<Vec3>& GetColor() const { return color; }
        void SetColor(const Vec3& aColor) { color = aColor; }

        const Optional<float32>& GetInnerConeAngle() const { return innerConeAngle; }
        void SetInnerConeAngle(const Optional<float32>& aInnerConeAngle) { innerConeAngle = aInnerConeAngle; }

        const Optional<float32>& GetOuterConeAngle() const { return outerConeAngle; }
        void SetOuterConeAngle(const Optional<float32>& aOuterConeAngle) { outerConeAngle = aOuterConeAngle; }

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

        bool IsShadowEnabled() const { return shadowEnabled; }
        void SetShadowEnabled(bool aShadowEnabled) { shadowEnabled = aShadowEnabled; }
    private:
        LightType type;
        Optional<Vec3> color;
        Optional<float32> innerConeAngle;
        Optional<float32> outerConeAngle;
        Optional<float32> range;
        bool enabled = true;
        float32 intensity = 1.0;
        int32 index = 0;
        bool shadowEnabled = false;
        String name;
    };
}
