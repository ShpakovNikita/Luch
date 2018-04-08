#pragma once

#include <Husky/Types.h>
#include <Husky/RefPtr.h>
#include <Husky/VectorTypes.h>
#include <Husky/BaseObject.h>
#include <Husky/SceneV1/CameraType.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::SceneV1
{
    class Camera : public BaseObject
    {
    protected:
        Camera(CameraType type, const String& name = "");
    public:
        inline CameraType GetCameraType() const { return type; }

        virtual Mat4x4 GetCameraViewMatrix() const;
        virtual void SetCameraViewMatrix(const Mat4x4& view);
        virtual Mat4x4 GetCameraProjectionMatrix() const = 0;

        inline const RefPtr<Vulkan::Buffer>& GetUniformBuffer() const { return uniformBuffer; }
        inline void SetUniformBuffer(const RefPtr<Vulkan::Buffer>& aUniformBuffer) { uniformBuffer = aUniformBuffer; }

        inline const RefPtr<Vulkan::DescriptorSet>& GetDescriptorSet() const { return descriptorSet; }
        inline void SetDescriptorSet(const RefPtr<Vulkan::DescriptorSet>& aDescriptorSet) { descriptorSet = aDescriptorSet; }
    protected:
        Mat4x4 viewMatrix;
    private:
        CameraType type;
        String name;

        RefPtr<Vulkan::DescriptorSet> descriptorSet;
        RefPtr<Vulkan::Buffer> uniformBuffer;
    };

    class PerspectiveCamera : public Camera
    {
    public:
        PerspectiveCamera(
            float32 yfov,
            float32 znear,
            Optional<float32> zfar = {},
            Optional<float32> aspectRatio = {},
            const String& name = "");

        Mat4x4 GetCameraProjectionMatrix() const override;

        void SetViewAspectRatio(float32 aspectRatio) { viewAspectRatio = aspectRatio; }

        inline float32 GetYFOV() const { return yfov; }
        inline float32 GetZNear() const { return znear; }
        inline const Optional<float32>& GetZFar() const { return zfar; }
        inline const Optional<float32>& GetAspectRatio() const { return aspectRatio; }
    private:
        float32 yfov;
        float32 znear;
        Optional<float32> zfar;
        Optional<float32> aspectRatio;
        float32 viewAspectRatio = 1;
    };

    class OrthographicCamera : public Camera
    {
    public:
        OrthographicCamera(
            float32 xmag,
            float32 ymag,
            float32 zfar,
            float32 znear,
            const String& name = ""
        );

        Mat4x4 GetCameraProjectionMatrix() const override;

        inline float32 GetXMag()  const { return xmag; }
        inline float32 GetYMag()  const { return ymag; }
        inline float32 GetZFar()  const { return zfar; }
        inline float32 GetZNear() const { return znear; }
    private:
        float32 xmag;
        float32 ymag;
        float32 zfar;
        float32 znear;
    };
}
