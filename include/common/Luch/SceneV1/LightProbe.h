#pragma once

#include <Luch/BaseObject.h>
#include <Luch/VectorTypes.h>
#include <Luch/Size2.h>

namespace Luch::SceneV1
{
    class LightProbe : public BaseObject
    {
    public:
        LightProbe() = default;

        bool HasDiffuseIrradiance() const { return diffuseIrradiance; }
        void SetHasDiffuseIrradiance(bool value) { diffuseIrradiance = value; }

        bool HasSpecularReflection() const { return specularReflection; }
        void SetHasSpecularReflection(bool value) { specularReflection = value; }

        Size2i GetSize() const { return size; }
        void SetSize(Size2i aSize) { size = aSize; }

        float32 GetZNear() const { return zNear; }
        void SetZNear(float32 aZNear) { zNear = aZNear; }

        float32 GetZFar() const { return zFar; }
        void SetZFar(float32 aZFar) { zFar = aZFar; }

        bool IsEnabled() const { return enabled; }
        void SetEnabled(bool aEnabled) { enabled = aEnabled; }

        const String& GetName() const { return name; }
        void SetName(const String& aName) { name = aName; }
    private:
        bool enabled = true;
        bool diffuseIrradiance = false;
        bool specularReflection = false;
        float32 zNear = 0.0;
        float32 zFar = 0.0;
        Size2i size;
        String name;
    };
}
