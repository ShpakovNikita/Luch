#pragma once

#include <Husky/Types.h>
#include <Husky/RefPtr.h>
#include <Husky/VectorTypes.h>
#include <Husky/BaseObject.h>
#include <Husky/SceneV1/CameraType.h>
#include <Husky/Graphics/GraphicsForwards.h>

namespace Husky::SceneV1
{
    class Camera : public BaseObject
    {
    protected:
        Camera(CameraType type);
    public:
        inline CameraType GetCameraType() const { return type; }

        virtual Mat4x4 GetCameraViewMatrix() const;
        virtual void SetCameraViewMatrix(const Mat4x4& view);

        virtual Vec3 GetCameraPosition();

        virtual Mat4x4 GetCameraProjectionMatrix() const = 0;

        inline const RefPtr<Graphics::Buffer>& GetUniformBuffer() const { return uniformBuffer; }
        inline void SetUniformBuffer(const RefPtr<Graphics::Buffer>& aUniformBuffer)
        {
            uniformBuffer = aUniformBuffer;
        }

        inline const RefPtr<Graphics::DescriptorSet>& GetVertexDescriptorSet() const { return vertexDescriptorSet; }
        inline void SetVertexDescriptorSet(const RefPtr<Graphics::DescriptorSet>& aVertexDescriptorSet)
        {
            vertexDescriptorSet = aVertexDescriptorSet;
        }

        inline const RefPtr<Graphics::DescriptorSet>& GetFragmentDescriptorSet() const { return fragmentDescriptorSet; }
        inline void SetFragmentDescriptorSet(const RefPtr<Graphics::DescriptorSet>& aFragmentDescriptorSet)
        {
            fragmentDescriptorSet = aFragmentDescriptorSet;
        }

        inline const String& GetName() const { return name; }
        inline void SetName(const String& aName) { name = aName; }
    protected:
        Mat4x4 viewMatrix;
    private:
        CameraType type = CameraType::Perspective;
        String name;

        RefPtr<Graphics::DescriptorSet> vertexDescriptorSet;
        RefPtr<Graphics::DescriptorSet> fragmentDescriptorSet;
        RefPtr<Graphics::Buffer> uniformBuffer;
    };

    class PerspectiveCamera : public Camera
    {
    public:
        PerspectiveCamera();

        PerspectiveCamera(
            float32 yfov,
            float32 znear,
            Optional<float32> zfar = {},
            Optional<float32> aspectRatio = {});

        Mat4x4 GetCameraProjectionMatrix() const override;

        void SetViewAspectRatio(float32 aspectRatio) { viewAspectRatio = aspectRatio; }

        inline float32 GetYFOV() const { return yfov; }
        inline void SetYFOV(float32 aYfov) { yfov = aYfov; }

        inline float32 GetZNear() const { return znear; }
        inline void SetZNear(float32 aZNear) { znear = aZNear; }

        inline const Optional<float32>& GetZFar() const { return zfar; }
        inline void SetZFar(const Optional<float32>& aZFar) { zfar = aZFar; }

        inline const Optional<float32>& GetAspectRatio() const { return aspectRatio; }
        inline void SetAspectRatio(const Optional<float32>& aAspectRatio) { aspectRatio = aAspectRatio; }
    private:
        float32 yfov = glm::pi<float32>();
        float32 znear = 0.000001;
        Optional<float32> zfar;
        Optional<float32> aspectRatio;
        float32 viewAspectRatio = 1;
    };

    class OrthographicCamera : public Camera
    {
    public:
        OrthographicCamera();

        OrthographicCamera(
            float32 xmag,
            float32 ymag,
            float32 zfar,
            float32 znear);

        Mat4x4 GetCameraProjectionMatrix() const override;

        inline float32 GetXMag()  const { return xmag; }
        inline void SetXMag(float32 aXMag) { xmag = aXMag; }

        inline float32 GetYMag()  const { return ymag; }
        inline void SetYMag(float32 aYMag) { ymag = aYMag; }

        inline float32 GetZFar()  const { return zfar; }
        inline void SetZFar(float32 aZFar) { zfar = aZFar; }

        inline float32 GetZNear() const { return znear; }
        inline void SetZNear(float32 aZNear) { znear = aZNear; }
    private:
        float32 xmag = 1.0;
        float32 ymag = 1.0;
        float32 zfar = 1.0;
        float32 znear = 0.000001;
    };
}
